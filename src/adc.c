/**
 * @file        adc.c
 * @brief       Module for ADC (Analog-to-Digital Converter) readings and analysis.
 *
 * @details     This module handles the configuration of the ADC peripheral and
 * manages the acquisition and processing of analog data. It is
 * specifically designed to measure motor speeds and perform a
 * calibration routine to map ADC values to corresponding motor
 * speeds (positive and negative). An interrupt-driven approach
 * is used to read from ADC channels 4 and 5 for the left and
 * right motors, respectively.
 *
 * @author      Thomas Giguere Sturrock
 * @date        Jun 2022
*/

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include "main.h"
#include "pwm.h"
/* Defines -------------------------------------------------------------------*/
#define GAUCHE 0
#define DROITE 1
#define CONSTANTE_MOTEUR 8
#define AVANT 		1
#define ARRIERE 	-1
#define ARRET 		0

/* Private variables ---------------------------------------------------------*/

static uint8_t channel = GAUCHE;		//La variable utilise pour garder en memoire le canal a echantillone
static int32_t echantillon_droite=0;	//La somme des echantillons du moteur droite
static int32_t echantillon_gauche=0;	//La somme des echantillons du moteur gauche
static uint16_t compteur_gauche=0;		//La nombre d'echantillons du moteur droite
static uint16_t compteur_droite=0;		//La nombre d'echantillons du moteur gauche

int32_t echantillon_droite_temp;
uint16_t compteur_droite_temp;
int32_t echantillon_gauche_temp;
uint16_t compteur_gauche_temp;


int32_t vg_max_p;	 	//La voltage max positif du moteur gauche
int32_t vg_max_n;		//La voltage max negatif du moteur gauche
int32_t vg_min_p;		//La voltage min positif du moteur gauche
int32_t vg_min_n;		//La voltage min negatif du moteur gauche

int32_t vd_max_p;		//La voltage max positif du moteur droite
int32_t vd_max_n;		//La voltage max negatif du moteur droite
int32_t vd_min_p;		//La voltage min positif du moteur droite
int32_t vd_min_n;		//La voltage min negatif du moteur droite

int32_t pente_p_moteur_gauche;
int32_t pente_p_moteur_droite;
int32_t pente_n_moteur_gauche;
int32_t pente_n_moteur_droite;

int32_t abcisse_p_moteur_gauche;
int32_t abcisse_p_moteur_droite;
int32_t abcisse_n_moteur_gauche;
int32_t abcisse_n_moteur_droite;

/* Public functions  ---------------------------------------------------------*/
/**
 * @brief  Fonction qui configure le peripherique d'ADC
 * @param  None
 * @retval None
 */
void config_adc(void){
	//Les canaux 4 et 5 sont utilise par l'adc, mais les canaux 6 et 7 sont utiliser pour obtenir le sens de rotation des roues
	GPIOA->MODER |= GPIO_MODER_MODER4;	//Configure le canal 4 comme une entree analogue
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4); 		//Entree 4 est analogue, aucune pull-up/down

	GPIOA->MODER |= GPIO_MODER_MODER5;	//Configure le canal 5 comme une entree analogue
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR5); 		//Entree 5 est analogue, aucune pull-up/down

	GPIOA->MODER &= ~(GPIO_MODER_MODER6); 	//Configure le canal 6 comme une entree
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR6_1);		//Entree 6, aucune pull-up/down

	GPIOA->MODER &= ~(GPIO_MODER_MODER7);	//Configure le canal 7 comme une entree
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR7_1);		//Entree 7, aucune pull-up/down

	GPIOA->MODER |= (GPIO_MODER_MODER8_0); 	//Configure le canal 8 comme une sortie
	GPIO_OTYPE_CONFIG(GPIOA,8,1);				//Open drain
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR8_0);		//PULL-UP

	//Setup du signal d'horloge
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->APB2RSTR |= RCC_APB2RSTR_ADCRST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_ADCRST;
	// Change le hpre et le ppre au besoin si ca ne fonctionne pas
	ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;
	ADC1->CFGR2 |= ADC_CFGR2_CKMODE_1; 		//On divise la clock de 48MHz par 4 -> 12 MHz

	ADC1->CFGR1 |= ADC_CFGR1_WAIT; 			//Mode attente active
	ADC1->CFGR1 |= ADC_CFGR1_CONT; 			//Mode lecture continue active
	ADC1->CFGR1 &= ~ADC_CFGR1_ALIGN; 		//Alignement a droite
	ADC1->CFGR1 &= ~ADC_CFGR1_RES;  			//Resolution de 12 bits
	ADC1->CFGR1 &= ~ADC_CFGR1_SCANDIR;		//On scan les canaux de maniere ascendent (0 -> 18)

	ADC1->ISR |= ADC_ISR_EOC;
	ADC1->IER |= ADC_IER_EOCIE;
	//ADC1->IER |= ADC_IER_EOSEQIE;

	NVIC->ISER[0] = (((uint32_t) 1) << (ADC1_COMP_IRQn & 0x1F));
	NVIC->IP[(uint32_t)(ADC1_COMP_IRQn>>2)] = ADC_PRIORITY-(1<<(((ADC1_COMP_IRQn & 0x03) << 3)*8));

	ADC1->SMPR |= (ADC_SMPR1_SMPR_1|ADC_SMPR1_SMPR_0); //239.5cycles

	ADC1->CHSELR |= (ADC_CHSELR_CHSEL4|ADC_CHSELR_CHSEL5); //Permet a l'ADC d'echantillonner le canal 4 et 5

	ADC1->CR |= ADC_CR_ADCAL;//Part la calibration

	while(ADC1->CR & ADC_CR_ADCAL); //Attend que la calibration fini

	ADC1->CR |= ADC_CR_ADEN; //Active l'adc

	while(!(ADC1->ISR & ADC_ISR_ADRDY)); //Attend que l'ADC soit pret

	ADC1->CR |= ADC_CR_ADSTART;
}

void ADC1_COMP_IRQHandler(void){
	//Conversion du moteur gauche
	if(channel==GAUCHE){
		channel=DROITE;
		//Valeur negative
		if((GPIOA->IDR & ((uint16_t)GPIO_IDR_6))== ((uint16_t)GPIO_IDR_6)){
			echantillon_gauche -= (int32_t)ADC1->DR;
		}
		//Valeur positive
		else{
			echantillon_gauche += (int32_t)ADC1->DR;
		}
		compteur_gauche+=1;
	}
	//Conversion du moteur droit
	else if(channel==DROITE){
		channel=GAUCHE;
		//Valeur negative
		if((GPIOA->IDR & ((uint16_t)GPIO_IDR_7)) == ((uint16_t)GPIO_IDR_7)){
			echantillon_droite -= (int32_t)ADC1->DR;
		}
		//Valeur positive
		else{
			echantillon_droite += (int32_t)ADC1->DR;
		}
		compteur_droite+=1;
	}
}


/**
 * @brief  Fonction qui defini la valeur moyenne ressu par l'adc depuis
 *         la derniere utilisation de la fonction
 * @param  None
 * @retval None
 */
void vitesse_moyenne_mesure(void){
	//Copie les donnees d'echantillons
	__set_PRIMASK(1);

	echantillon_droite_temp=echantillon_droite;
	compteur_droite_temp=compteur_droite;
	echantillon_gauche_temp=echantillon_gauche;
	compteur_gauche_temp=compteur_gauche;

	//Purge la memoire pour collecter d'autres donnes
	echantillon_droite=0;
	compteur_droite=0;
	echantillon_gauche=0;
	compteur_gauche=0;
	__set_PRIMASK(0);

	if(adc_5ms){
		__set_PRIMASK(1);
		adc_5ms=0;

		if(compteur_droite_temp>0){
			controlData.v_moyenne_droite=echantillon_droite_temp/(int32_t)compteur_droite_temp;
			echantillon_droite_temp=0;
			compteur_droite_temp=0;
		}
		if(compteur_gauche>0){
			controlData.v_moyenne_gauche=echantillon_gauche_temp/(int32_t)compteur_gauche_temp;
			echantillon_gauche_temp=0;
			compteur_gauche_temp=0;
		}
		//On reactive les interruption et la conversion
		__set_PRIMASK(0);
	}
}

void moyenne(int32_t* v_droite, int32_t* v_gauche){
	//Purge la memoire et on demarre un autre echantillonage
	while(adc_5ms==0){}
	__set_PRIMASK(1);
	echantillon_droite=0;
	compteur_droite=0;
	echantillon_gauche=0;
	compteur_gauche=0;
	adc_5ms=0;
	__set_PRIMASK(0);
	while(adc_5ms==0){}

	//Copie les donnees d'echantillons
	__set_PRIMASK(1);
	echantillon_droite_temp=echantillon_droite;
	compteur_droite_temp=compteur_droite;
	echantillon_gauche_temp=echantillon_gauche;
	compteur_gauche_temp=compteur_gauche;
	__set_PRIMASK(0);


	if(adc_5ms){

		adc_5ms=0;

		if(compteur_droite_temp>0){
			*v_droite=echantillon_droite_temp/(int32_t)compteur_droite_temp;
			echantillon_droite_temp=0;
			compteur_droite_temp=0;

		}
		if(compteur_gauche_temp>0){
			*v_gauche=echantillon_gauche_temp/(int32_t)compteur_gauche_temp;
			echantillon_gauche_temp=0;
			compteur_gauche_temp=0;
		}

	}
}

/**
 * @brief  Fonction qui cree un delais en seconde
 * @param uint16_t time_in_sec : longueur du delais voulu en seconde
 * @retval None
 */
void delay_in_sec(uint16_t time_in_sec){
	uint16_t counter;
	counter=time_in_sec*1000/5;
	for(uint16_t delay=0;delay<counter;delay+=1){
		while(adc_5ms==0){}
		adc_5ms=0;
	}
}

/**
 * @brief  Fonction qui calibre les minimum et maximum de l'ADC
 * @param  None
 * @retval None
 */
void moteur_calibration(void){
	GPIO_SET(GPIOA, 8);
	delay_in_sec(1);

	//Calcul de la valeur max positive
	update_moteur(AVANT,AVANT,0);
	delay_in_sec(CONSTANTE_MOTEUR);
	moyenne(&vd_max_p,&vg_max_p);


	//Calcul de la valeur min positive
	update_moteur(ARRET,ARRET,0);
	delay_in_sec(CONSTANTE_MOTEUR);
	moyenne(&vd_min_p,&vg_min_p);


	//Calcul de la valeur max negative
	update_moteur(ARRIERE,ARRIERE,0);
	delay_in_sec(CONSTANTE_MOTEUR);
	moyenne(&vd_max_n,&vg_max_n);

	//Calcul de la valeur min negative
	update_moteur(ARRET,ARRET,0);
	delay_in_sec(CONSTANTE_MOTEUR);
	moyenne(&vd_min_n,&vg_min_n);

	GPIO_RESET(GPIOA, 8);

	vitesse_mapping_init();
}

/**
 * @brief  Fonction qui map les minimum et maximum des d'adc
 * @param  None
 * @retval None
 */
void vitesse_mapping_init(void){
	pente_p_moteur_gauche = (vg_max_p-vg_min_p);
	pente_p_moteur_droite = (vd_max_p-vd_min_p);
	pente_n_moteur_gauche = (vg_min_n-vg_max_n);
	pente_n_moteur_droite = (vd_min_n-vd_max_n);

}


/**
 * @brief  Fonction qui defini la vitesse de chaque moteur
 * @param  float* v_moyenne_gauche : vitesse du moteur gauche
 *         float* v_moyenne_droite : vitesse du moteur droit
 * @retval None
 */
void vitesse_mapping(float* v_moyenne_gauche,float* v_moyenne_droite){

	if(controlData.v_moyenne_gauche >= vg_min_p){
		*v_moyenne_gauche = ((float)(controlData.v_moyenne_gauche-vg_min_p)/pente_p_moteur_gauche);
	}
	else if ((controlData.v_moyenne_gauche < vg_min_p) && (controlData.v_moyenne_gauche > vg_min_n)){
		*v_moyenne_gauche = 0;
	}
	else{
		*v_moyenne_gauche =((float)(controlData.v_moyenne_gauche-vg_min_n)/pente_n_moteur_gauche);
	}

	//cote droite
	if(controlData.v_moyenne_droite >= vd_min_p){
		*v_moyenne_droite =((float)(controlData.v_moyenne_droite-vd_min_p)/pente_p_moteur_droite);
	}
	else if ((controlData.v_moyenne_droite < vd_min_p) && (controlData.v_moyenne_droite > vd_min_n)){
		*v_moyenne_droite = 0;
	}
	else{
		*v_moyenne_droite = ((float)(controlData.v_moyenne_droite-vd_min_n)/pente_n_moteur_droite);
	}
}






