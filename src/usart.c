/**
 ******************************************************************************
 * File Name          : usart.c
 * Description        : Ce fichier sert pour configurer et ou utiliser le
 * 						peripherique USART
 * Created            : 13 juin 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* Defines -------------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t etat = 0;
static uint8_t data_reception_usart[30];
static uint8_t data_envoie_usart[30];
static buffer_t buffer_telecommande_reception;
static buffer_t buffer_telecommande_envoie;
uint8_t toggle_led_uart = 0;

/* Private function prototypes -----------------------------------------------*/

/* Public functions  ---------------------------------------------------------*/

void config_uart2(void){

	//Configuration des GPIO
	GPIO_MODE_CONFIG(GPIOA, 2, GPIO_ALT_FUNC); 		//On defini la pin 2 du port A comme une fonction alternative (USART)
	GPIO_MODE_CONFIG(GPIOA, 3, GPIO_ALT_FUNC); 		//On defini la pin 3 du port A comme une fonction alternative (USART)
	GPIO_OTYPE_CONFIG(GPIOA, 2, 0);					// Push-pull
	GPIO_OTYPE_CONFIG(GPIOA, 3, 1);					// Open-Drain
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR2_0; 			//Active la pull-up
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR3_0; 			//Active la pull-up
	GPIOA->OSPEEDR &= ~ GPIO_OSPEEDR_OSPEEDR2_0;	//On defini la vitesse du slew rate du TX a 2MHz
	GPIOA->OSPEEDR &= ~ GPIO_OSPEEDR_OSPEEDR3_0;	//On defini la vitesse du slew rate du RX a 2MHz
	GPIO_ALTFUN_CONFIG(GPIOA, 2, 1);				//On assigne la pin 2 du port A au peripherique USART
	GPIO_ALTFUN_CONFIG(GPIOA, 3, 1);				//On assigne la pin 3 du port A au peripherique USART

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	USART2->CR1 &= ~USART_CR1_OVER8; 	//On defini l'echantillonage a 16MHz
	USART2->CR1 &= ~USART_CR1_M;		//On defini la longueur d'un charactere a 8bit
	USART2->CR1 &= ~USART_CR1_PCE; 		//On ne veut pas de bit de parite

	USART2->CR2 &= ~USART_CR2_STOP; 	//On veut 1 seul STOP bit

	USART_CONFIG_BRR(USART2, 5000, 0);	//On defini la vitesse du Baud Rate a 9600

	USART2->CR1 |= USART_CR1_TE;	//On active le transmetteur
	USART2->CR1 |= USART_CR1_RE;	//On active le recepteur
	USART2->CR1 |= USART_CR1_UE;	//On active le USART


	USART2->CR1 |= USART_CR1_RXNEIE; 	//Les interruptions sont permise
	USART2->CR3 |= USART_CR3_EIE; 		//On permet a l'interruption d'erreur de s'activer

	NVIC->IP[7] |= NICE;		//On donne un noveau de priorite a l'interruption
	NVIC->ISER[0] |= 1<<28;		//On permet a la routine d'etre declanche

	// On cree 2 nouveaux buffer pour recevoir et envoyer des donnees en UART
	buffer_new(&buffer_telecommande_reception,data_reception_usart,30);
	buffer_new(&buffer_telecommande_envoie,data_envoie_usart,30);
}


void state_machine(control_struct_t *control){

	//Si nous avons recue des donnees de la telecommande
	if(buffer_count(&buffer_telecommande_reception)!=0){
		uint8_t reception;

		//On copie le contenu du buffer de reception dans un second buffer
		buffer_pull(&buffer_telecommande_reception, &reception);
		//On transcrit les donnees recus dans le buffer de transmission
		buffer_push(&buffer_telecommande_envoie,reception);

		/*On utilise une machine a etat pour valider l'integrite des donnees recue*/
		switch (etat){
		//Le premier paquet de donnees recu doit etre une commande
		case COMMANDE:
			//Si la commande est valide
			if(reception==0xF1 || reception==0xF0)
			{
				//On met a jour la commande du robot
				updateCommande(control,reception);
				//On passe au prochaine etat de la machine (Vitesse)
				etat = VITESSE;
			}
			//Si la commande recue n'est pas valide, on re-initialise la machine a etat
			else{
				etat = COMMANDE;
			}
			break;
			//Le deuxieme paquet de donnees recu doit etre une vitesse
		case VITESSE:
			//Si la vitesse recu est valide
			if(reception>=0 && reception<=200)
			{
				//On met a jour la vitesse du robot
				updateVitesseUart(control,reception);
				//On passe au prochaine etat de la machine (Angle)
				etat = ANGLE;
			}
			//Si la vitesse recue n'est pas valide, on re-initialise la machine a etat
			else{
				etat = COMMANDE;
			}
			break;
			//Le troisieme paquet de donnees recu doit etre un angle
		case ANGLE:
			//Si l'angle est valide
			if(reception>=0 && reception<=180)
			{
				//On met a jour l'angle du robot
				updateAngleUart(control,reception);
				updateVitesse_angle(control);
			}
			//Puisque l'angle est le dernier paquet recu, on re-initialise la machine a etat
			etat = COMMANDE;

			break;

		default:
			break;
		}
		//On renvoie les donnes a la telecommande
		USART2->CR1 |= USART_CR1_TXEIE;
		if(counterDelay5ms>=10){
			if(toggle_led_uart){
				GPIO_SET(GPIOC,1);
				toggle_led_uart=0;
			}else{
				GPIO_RESET(GPIOC,1);
				toggle_led_uart=1;
			}
			counterDelay5ms=0;
		}

	}
}

/**
 * @brief  La routine d'interuption du periherique USART
 * @param  None
 * @retval None
 */
void USART2_IRQHandler( void ){

	//Cas d'une reception de donnee (RX)
	if ( ( USART2->ISR & USART_ISR_RXNE ) == USART_ISR_RXNE ){
		buffer_push(&buffer_telecommande_reception,(uint8_t)(USART2->RDR));
	}
	// Cas d'un envoi de donnee (TX)
	else if ( ( USART2->ISR & USART_ISR_TXE ) == USART_ISR_TXE ){
		uint8_t envoie;
		if(buffer_count(&buffer_telecommande_envoie)!=0){
			buffer_pull(&buffer_telecommande_envoie, &envoie);
			USART2->TDR = (uint16_t)envoie;
		}
		//Si nous avons fini de transcrire le contenue du buffer
		//on transmet le tout
		if(buffer_count(&buffer_telecommande_envoie)==0)
			USART2->CR1 &= ~USART_CR1_TXEIE;
	}
	/* S'il y une erreur de transmission on recommence */
	else if ( ( USART2->ISR & USART_ISR_ORE ) == USART_ISR_ORE ){
		// on interdit le flag d�interrompre
		USART2->CR1 &= ~USART_ICR_ORECF ;
	}
}
