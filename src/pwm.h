/**
 ******************************************************************************
 * File Name          : pwm.h
 * Description        : ce module contien tout ce qui conserne le controle est le fonctionnement du pwm
 * Created            : Jun 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */
/* Prevent recursive inclusion -----------------------------------------------*/
#ifndef PWM_H_
#define PWM_H_
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Defines -------------------------------------------------------------------*/

/* Type definitions ----------------------------------------------------------*/

/* Function prototypes ------------------------------------------------------ */
/**
 * @brief  Fonction qui configure le peripherique de pwm
 * @param  None
 * @retval None
 */
void init_pwm();

/**
 * @brief  Fonction qui configure le peripherique de pwm
 * @param  None
 * @retval None
 */
void update_moteur(float vitesse_gauche, float vitesse_droite,uint8_t arret_urgence);


#endif /* PWM_H_ */
