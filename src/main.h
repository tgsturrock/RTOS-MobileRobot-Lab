/**
 ******************************************************************************
 * File Name          : main.h
 * Description        : ce mocule contien les masques utilisé pour configurer les differents modules
 * Created            : Jun 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */
/*header file principal*/

#ifndef _SETTINGS_ROBOT_H_
#define _SETTINGS_ROBOT_H_

#include <stddef.h>
#include <stm32f0xx.h>
#include <stdlib.h>
#include "buffer.h"
#include "control.h"
#include "sonar.h"
/*
 * GPIO config macro
 */

#define GPIO_MODE_CONFIG(PORT,PIN,MODE) PORT->MODER=(PORT->MODER &~(0x3<<(PIN*2)))|(MODE<<(PIN*2))
#define GPIO_OTYPE_CONFIG(PORT,PIN,OTYPE) PORT->OTYPER = (PORT->OTYPER&~(1<<PIN))|(OTYPE<<PIN)
#define GPIO_ALTFUN_CONFIG(PORT,PIN,FUN) PORT->AFR[PIN>>3]=(PORT->AFR[PIN>>3]&~(0xF<<((PIN&0x1)*4))) | (FUN<<((PIN&0X7)*4))
#define SET_GPIO(PORT,PIN) PORT->ODR |= 1<<PIN
#define RESET_GPIO(PORT,PIN) PORT->ODR &= ~(1<<PIN)
#define GPIO_READ(PORT,PIN) PORT->IDR && (1<<PIN)


#define THE_ANWSER 42

#define GPIO_AVANT1(PORT,PIN) 		PORT->ODR |= ~(1010<<PIN)  		//0101
#define GPIO_AVANT2(PORT,PIN) 		PORT->ODR &= ~(1010<<PIN)  		//0101

#define GPIO_ARRIERE1(PORT,PIN) 	PORT->ODR |= (1010<<PIN) 		//1010
#define GPIO_ARRIERE2(PORT,PIN) 	PORT->ODR &= (1010<<PIN) 		//1010

#define GPIO_HORAIRE1(PORT,PIN) 	PORT->ODR |= (0110<<PIN) 		//1001
#define GPIO_HORAIRE2(PORT,PIN) 	PORT->ODR &= (0110<<PIN) 		//1001

#define GPIO_ANTI_HORAIRE1(PORT,PIN) 	PORT->ODR |= (0110<<PIN) 	//0110
#define GPIO_ANTI_HORAIRE2(PORT,PIN) 	PORT->ODR &= (0110<<PIN) 	//0110

#define GPIO_ARRET(PORT,PIN) 		PORT->ODR |= (1111<<PIN)		//11
#define GPIO_NEUTRE(PORT,PIN) 		PORT->ODR &= ~(1111<<PIN)		//00

/*
 * GPIO control macro
 */
#define BITS_DIRECTION 12
#define GPIO_SET(PORT,PIN) PORT->ODR |= 1<<PIN
#define GPIO_RESET(PORT,PIN) PORT->ODR &= ~(1<<PIN)


#define GPIO_INPUT 0b0
#define GPIO_OUTPUT 0b1
#define GPIO_ALT_FUNC 0b10
#define GPIO_ANALOG 0b11

#define NO_PULL 0b0
#define PULL_UP 0b1
#define PULL_DOWN 0b10



control_struct_t controlData;
uint16_t interup_5ms;
uint8_t adc_5ms;
uint32_t counterDelay5ms;




#endif
