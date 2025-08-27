/**
 ******************************************************************************
 * File Name          : usart.h
 * Description        : Ce fichier sert pour configurer et ou utiliser le
 * 						peripherique USART
 * Created            : 13 juin 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */

#ifndef USART_H_
#define USART_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "control.h"

/* Defines -------------------------------------------------------------------*/
#define USART_CONFIG_BRR(USART,USARTDIV,OVER8) USART->BRR=(USARTDIV & 0xFFF0)|((USARTDIV & 0x000F)>>OVER8)
#define NICE 69
#define COMMANDE 0
#define VITESSE 1
#define ANGLE 2

/* Type definitions ----------------------------------------------------------*/

/* Function prototypes ------------------------------------------------------ */

/**
 * @brief  Fonction qui configure le peripherique USART
 * @param  None
 * @retval None
 */
void config_uart2(void);

/**
 * @brief  	Fonction qui sert de machine a etat pour s'assurer de l'integrite du contenue recu le peripherique USART
 * 			Cette fonction envoie aussi un echo des donnees recue
 * @param  control_struct_t *control : Une variable de sauveguarde des etats de la machine a etat
 * @retval None
 */
void state_machine(control_struct_t *control);



#endif /* USART_H_ */
