/**
 * @file        pwm.c
 * @brief       Module for PWM control and operation.
 *
 * @details     This module contains the functions to configure and manage the
 * TIM3 peripheral for PWM generation. It controls the speed and direction of
 * the robot's motors by adjusting the PWM duty cycle and setting GPIO pins
 * for forward/reverse control. An interrupt handler is also included to
 * signal a 5ms interval for other tasks within the system.
 *
 * @author      Thomas Giguere Sturrock
 * @date        Jun 2022
 */

/* Includes ------------------------------------------------------------------*/
#include "pwm.h"
#include <math.h>
/* Defines -------------------------------------------------------------------*/
#define TIM3_PRESCALER (4 - 1)
#define TIM3_ARR_VALUE (60000-1)

/* Public functions  ---------------------------------------------------------*/

/**
 * @brief  Fonction qui configure le peripherique de pwm
 * @param  None
 * @retval None
 */
void init_pwm(){

	GPIO_MODE_CONFIG(GPIOB, 4, GPIO_ALT_FUNC);
	GPIO_MODE_CONFIG(GPIOB, 5, GPIO_ALT_FUNC);

	GPIO_ALTFUN_CONFIG(GPIOB, 4, 1);
	GPIO_ALTFUN_CONFIG(GPIOB, 5, 1);

	RCC->APB1ENR |= (uint16_t)RCC_APB1ENR_TIM3EN;

	TIM3->SMCR &= (uint16_t)~(TIM_SMCR_SMS);
	TIM3->CCMR1 |= (uint16_t)(TIM_CCMR1_OC2M|TIM_CCMR1_OC2PE|TIM_CCMR1_OC1M|TIM_CCMR1_OC1PE);
	TIM3->CCMR1 &= (uint16_t)~(TIM_CCMR1_CC2S|TIM_CCMR1_CC1S);
	TIM3->CCER |= (uint16_t)(TIM_CCER_CC1P|TIM_CCER_CC2P);
	TIM3->CCER |= (uint16_t)(TIM_CCER_CC1E|TIM_CCER_CC2E);
	TIM3->CR1 &= (uint16_t)~(TIM_CR1_CMS|TIM_CR1_DIR);
	TIM3->CR1 |= (uint16_t)TIM_CR1_CEN;
	TIM3->EGR |= (uint16_t)TIM_EGR_UG;

	TIM3->PSC = (uint16_t)TIM3_PRESCALER; /* Diviseur d'horloge */
	TIM3->ARR = (uint16_t)TIM3_ARR_VALUE; /* Compte maximal pour compléter un cycle */
	TIM3->CCR1 = (uint16_t)0;
	TIM3->CCR2 = (uint16_t)0;

	TIM3->DIER |= (uint16_t)TIM_DIER_UIE;

	NVIC->ISER[0] |= 1<<TIM3_IRQn;
	NVIC->IP[(uint32_t)(TIM3_IRQn>>2)] = THE_ANWSER<<(((TIM3_IRQn & 0x03) << 3)*8) ;

	/*
	 * init direction
	 */
	GPIO_MODE_CONFIG(GPIOB,12,1);
	GPIO_MODE_CONFIG(GPIOB,13,1);
	GPIO_MODE_CONFIG(GPIOB,14,1);
	GPIO_MODE_CONFIG(GPIOB,15,1);
}

/**
 * @brief  Fonction qui configure le peripherique de pwm
 * @param  None
 * @retval None
 */
void update_moteur(float vitesse_gauche, float vitesse_droite,uint8_t arret_urgence){


	/*
	 * verifie si il y a une arret d'urgence
	 */
	if(arret_urgence){
		SET_GPIO(GPIOB,12);
		SET_GPIO(GPIOB,13);

		SET_GPIO(GPIOB,14);
		SET_GPIO(GPIOB,15);

		TIM3->CCR1 = (uint32_t)0;
		TIM3->CCR2 = (uint32_t)0;
	}
	else{
		/*
		 * regarde le sens voulu du moteur droit
		 */
		if(vitesse_droite<0){
			RESET_GPIO(GPIOB,14);
			SET_GPIO(GPIOB,15);
		}else if(vitesse_droite>0){
			SET_GPIO(GPIOB,14);
			RESET_GPIO(GPIOB,15);
		}else{
			RESET_GPIO(GPIOB,14);
			RESET_GPIO(GPIOB,15);
		}

		/*
		 * regarde le sens voulu du moteur gauche
		 */
		if(vitesse_gauche<0){

			RESET_GPIO(GPIOB,12);
			SET_GPIO(GPIOB,13);
		}else if(vitesse_gauche>0){
			SET_GPIO(GPIOB,12);
			RESET_GPIO(GPIOB,13);
		}else{
			RESET_GPIO(GPIOB,12);
			RESET_GPIO(GPIOB,13);
		}

		float speed_abs1 = fabs(vitesse_gauche);
		float speed_abs2 = fabs(vitesse_droite);
		TIM3->CCR1 = (uint32_t)((float)(TIM3_ARR_VALUE)*speed_abs1);
		TIM3->CCR2 = (uint32_t)((float)(TIM3_ARR_VALUE)*speed_abs2);
	}

}


/**
 * @brief  interuption de PWM
 * @param  None
 * @retval None
 */
void TIM3_IRQHandler(void) {
	if ((TIM3->SR & TIM_SR_UIF) != 0) { // Vérifie qu’il s’agit d’une fin de cycle du PWM
		TIM3->SR = ~((uint16_t) TIM_SR_UIF); // Remet le drapeau d’interruption à zéro
	}
	interup_5ms = 1;//met flag d'interup a 1 pour indiquer que 5ms c'est passer
	adc_5ms = 1;
	counterDelay5ms++;

}


