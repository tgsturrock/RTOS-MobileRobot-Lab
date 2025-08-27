/****************************Proto 1********************************************
 Author : Bruno De Kelper et Alexandre Ceriani
 Date   : 10-06-2016
 Description :
 GPIO : -User button (PB0 et PB1)
        -Reset button (NRST)
        -LED bleu (PC8) et verte (PC9)
        -Led externe au board (PC0-PC7)
        -I2C1 SDA (PB7) et SCL (PB6)
        -ADC (PA4 et PA5)
        -Bit Direction_FB (PA6 et PA7)
        -Bit Direction Gauche (LSB : PB12 MSB : PB13)
        -Bit Direction Droit (LSB : PB14 MSB : PB15)
        -Bit calibration (PA8)
        -PWM sur timer 3 channel 1 (PB4) et 2 (PB5)
        -USART2 RX (PA2) et TX (PA3)
 */

#include <stdio.h> /*sprintf*/
#include "main.h"
#include "usart.h"
#include "pwm.h"
#include "adc.h"
#include "moteur.h"
#include "i2c.h"
#include "sonar.h"

// Frequence des Ticks du SysTick (en Hz)
#define MillisecondsIT ((uint32_t) 1000)

/*Fonctions main*/
void Configure_Clock(void);
void Configure_LED(void);
void Configure_button(void);
void SysTick_Handler(void) {

}

int main(void) {

	interup_5ms=0;
	counterDelay5ms=0;
	adc_5ms=0;

	// Configure les composantes du robot
	__set_PRIMASK(1);
	/*Initialisation des peripheriques*/
	Configure_Clock();
	Configure_LED();
	config_uart2();
	config_adc();
	initControl(&controlData);
	init_pwm();
	Init_I2C();



	__set_PRIMASK(0);

	moteur_calibration();

	float duty_g =0;
	float duty_d=0;
	uint8_t etat_sonar_droit = 0;
	uint8_t etat_sonar_gauche = 0;
	uint8_t arret_urgence = 0;

	while (1) {
		state_machine(&controlData);//parsing du uart

		if(interup_5ms){

			/*
			 * note il faut restart avec le bouton meme si l'arret est avec la telecommande
			 */
			if(((GPIOB->IDR & ((uint16_t)GPIO_IDR_1))== ((uint16_t)GPIO_IDR_1))||(pullCommande(&controlData)==0xF0)){
				arret_urgence = 1;//arret d'urgence
			}
			else if((GPIOB->IDR & ((uint16_t)GPIO_IDR_0))== ((uint16_t)GPIO_IDR_0)){
				arret_urgence = 0;//mise en marche
			}


			/*
			 * activation des led d'etat et des fonction selon l'etat
			 */
			if(arret_urgence){
				GPIO_SET(GPIOC,6);
				GPIO_RESET(GPIOC,7);
				update_moteur(duty_g, duty_d,arret_urgence);// met l'arret d'urgence
			}else{
				GPIO_SET(GPIOC,7);
				GPIO_RESET(GPIOC,6);

				task_sonar(&controlData,&etat_sonar_droit,&etat_sonar_gauche);
				control_tsk(etat_sonar_droit,etat_sonar_gauche,&controlData,&duty_g,&duty_d);
				update_moteur(duty_g, duty_d,arret_urgence);
			}


			interup_5ms = 0;//met flag d'interup a 0 pour attendre le prochain 5ms

		}
	}
	return (0);
}

/**
 * @brief  Fonction qui configure les DELs du robot
 * @param  None
 * @retval None
 */
void Configure_LED(void){
	GPIO_MODE_CONFIG(GPIOC,0,1);//set PC0 a output
	GPIO_MODE_CONFIG(GPIOC,1,1);//set PC1 a output
	GPIO_MODE_CONFIG(GPIOC,2,1);//set PC2 a output
	GPIO_MODE_CONFIG(GPIOC,3,1);//set PC3 a output
	GPIO_MODE_CONFIG(GPIOC,4,1);//set PC4 a output
	GPIO_MODE_CONFIG(GPIOC,5,1);//set PC5 a output
	GPIO_MODE_CONFIG(GPIOC,6,1);//set PC6 a output
	GPIO_MODE_CONFIG(GPIOC,7,1);//set PC7 a output
}

/**
 * @brief  Fonction qui configure les bouton du robot
 * @param  None
 * @retval None
 */
void Configure_button(void){
	GPIO_MODE_CONFIG(GPIOB,0,0);//set Pb0 a input
	GPIO_MODE_CONFIG(GPIOB,1,0);//set Pb1 a input

}

/**
 * @brief  Fonction qui configure l'horloge du robot
 * @param  None
 * @retval None
 */
__INLINE void Configure_Clock(void){

	RCC->CR |= RCC_CR_HSEON;					// On active l'horloge externe
	RCC->CR &= ~RCC_CR_HSEBYP;					// On selectione l'horloge externe comme source
	RCC->CR &= ~RCC_CR_HSION;					// On desactive le crystal interne
	RCC->CFGR2 &= RCC_CFGR_PPRE_DIV1;			// On garde la frequence du signal d'horloge tel quel.

	// PLLCLK = HSE  /PREDIV * PLLMUL
	// 48 MHz = 8MHz /1      * 6
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;	// On divise la clock par 1
	RCC->CFGR |= RCC_CFGR_PLLMUL6;				// On multiplie l'horloge par 6
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;			// On maintient l'horloge a 48MHz pour les peripherique

	RCC->CR |= RCC_CR_PLLON;		// On active le multiplicateur de frequence
	RCC->CFGR |= RCC_CFGR_SW_PLL;	// On selectione le multiplicateur comme source d'horloge du systeme

	//On attend
	while(!RCC_CR_PLLRDY){
	}

	// On active l'horloge sur chaque port
	RCC->AHBENR    |= RCC_AHBENR_GPIOAEN;
	RCC->AHBENR    |= RCC_AHBENR_GPIOBEN;
	RCC->AHBENR    |= RCC_AHBENR_GPIOCEN;
	RCC->AHBENR    |= RCC_AHBENR_GPIODEN;

	SystemCoreClockUpdate();	// Met a jour SystemCoreClock avec la config du RCC
	SysTick_Config(SystemCoreClock/MillisecondsIT);	// Configure le SysTick a 1 ms
}

/*End of file*/
