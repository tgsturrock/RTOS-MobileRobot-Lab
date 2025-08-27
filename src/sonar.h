/**
 ******************************************************************************
 * File Name          : sonar.h
 * Description        : ce module s'occupe du control du sonar
 * Created            : Jun 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */

/* Prevent recursive inclusion -----------------------------------------------*/
#ifndef SONAR_H_
#define SONAR_H_
/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stm32f0xx.h>
#include "main.h"
#include "control.h"
/* Defines -------------------------------------------------------------------*/
#define DISTANCE_MIN 100 //distance de 50 cm pour activer les sonar
/* Adresse des sonars */
#define SRF10_LEFT_ADDR ((uint32_t)(0xE0))
#define SRF10_RIGHT_ADDR ((uint32_t)(0xE2))
/* Registres internes des sonars. */
/* En lecture */
#define SRF10_SW_VERSION ((uint32_t)(0x00))
/* Inutilisé : 0x01*/
#define SRF10_RANGE_MSB ((uint32_t)(0x02))
#define SRF10_RANGE_LSB ((uint32_t)(0x03))
/* En écriture */
#define SRF10_CMD_REG ((uint32_t)(0x00))
#define SRF10_MAX_GAIN ((uint32_t)(0x01))
#define SRF10_RANGE_REG ((uint32_t)(0x02))
/* Inutilisé : 0x03*/
#define DISTANCE_ARRET_URGENCE 25;

/* Type definitions ----------------------------------------------------------*/

/* Function prototypes ------------------------------------------------------ */

/**
 * @brief  Fonction qui renvoie le sonar qui a un objet le plus proche
 * @param  control_struct_t *control : structure de controle pour avoir la vitesse du robot
 * 		   uint8_t *etatDroit : etat du sonar droit (1=obstacle a droite, 0= pas d'obstacle a droite)
 * 		   uint8_t *etatGauche : etat du sonar gauche (1=obstacle a gauche, 0= pas d'obstacle a gauche)
 * @retval None
 */
void task_sonar(control_struct_t *control,uint8_t *etatDroit,uint8_t *etatGauche);


#endif /* SONAR_H_ */
