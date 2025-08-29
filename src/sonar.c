/**
 * @file        sonar.c
 * @brief       Module for controlling and managing sonar sensors.
 *
 * @details     This module provides the core logic for the robot's sonar-based
 * obstacle detection. It manages communication with the left and right sonar
 * sensors via the I2C bus, initiating pings and reading the distance to the
 * nearest object. The detection range is dynamically adjusted based on the
 * robot's current speed. It also provides status flags to indicate if an
 * obstacle has been detected on either side.
 *
 * @author      Thomas Giguere Sturrock
 * @date        Jun 2022
 */

/* Includes ------------------------------------------------------------------*/
#include "sonar.h"
#include "i2c.h"
#include <math.h>

/* Private variables ---------------------------------------------------------*/
uint8_t swap_gauche_droite = 0;
uint8_t compteur_nb_5ms = 0;

uint8_t sonar_gauche = 100;
uint8_t sonar_droit = 100;

uint8_t init_sonar = 1;

/* Public functions  ---------------------------------------------------------*/

/**
 * @brief  Fonction qui renvoie le sonar qui a un objet le plus proche
 * @param  control_struct_t *control : structure de controle pour avoir la vitesse du robot
 * 		   uint8_t *etatDroit : etat du sonar droit (1=obstacle a droite, 0= pas d'obstacle a droite)
 * 		   uint8_t *etatGauche : etat du sonar gauche (1=obstacle a gauche, 0= pas d'obstacle a gauche)
 * @retval None
 */
void task_sonar(control_struct_t *control,uint8_t *etatDroit,uint8_t *etatGauche){

	/*
	 * initialise le sonar
	 */
	if(init_sonar){
		I2C_Write(SONAR_ADR_G, SRF10_MAX_GAIN, 10); //j'ai un bug que le premier write de i2c du programe fonctionne pas

		I2C_Write(SONAR_ADR_G, SRF10_MAX_GAIN, 10); // set le gain de gauche a 10
		I2C_Write(SONAR_ADR_D, SRF10_MAX_GAIN, 10); // set le gain de droit a 10
		init_sonar=0;
	}

	if(compteur_nb_5ms>=10){//permet de s'enclancher seulement tout les 50ms

		uint8_t lsb =0;
		float vitesse = (pullVitesse(control));

		uint8_t range_sonar = (uint8_t)(MINRANGE+(float)(DELTA_RANGE*fabs(vitesse)));//donne une valeur entre MINRANGE et MAXRANGE

		uint8_t range_activation =(uint8_t)(100 + abs(vitesse*100));//varie la detection entre 1 et 2m selon la vitesse



		if(swap_gauche_droite){
			I2C_Write(SONAR_ADR_G, SRF10_RANGE_REG, range_sonar);//set la porte a 200cm pour le sonar gauche
			I2C_Write(SONAR_ADR_G, SRF10_CMD_REG, 0x51); // effectue un ping en cm sur le sonar gauche

			GPIO_SET(GPIOC,5);
			GPIO_RESET(GPIOC,4);

			I2C_Read(SONAR_ADR_D,SRF10_RANGE_LSB,&lsb);

			while(lsb==0);

			sonar_droit = lsb;

			swap_gauche_droite=0;
		}else{
			I2C_Write(SONAR_ADR_D, SRF10_RANGE_REG, range_sonar);//set la porte a 200cm pour le sonar gauche
			I2C_Write(SONAR_ADR_D, SRF10_CMD_REG, 0x51); // effectue un ping en cm sur le sonar droit


			GPIO_SET(GPIOC,4);
			GPIO_RESET(GPIOC,5);

			I2C_Read(SONAR_ADR_G,SRF10_RANGE_LSB,&lsb);

			while(lsb==0);

			sonar_gauche = lsb;

			swap_gauche_droite=1;
		}

		if((sonar_gauche<=range_activation)&&(sonar_gauche<=sonar_droit)){
			*etatGauche = 1;
			*etatDroit = 0;
			GPIO_SET(GPIOC,3);
			GPIO_RESET(GPIOC,2);
		}else if((sonar_droit<=range_activation)&&(sonar_droit<=sonar_gauche)){
			*etatDroit = 1;
			*etatGauche = 0;
			GPIO_SET(GPIOC,2);
			GPIO_RESET(GPIOC,3);
		}else{
			*etatDroit = 0;
			*etatGauche = 0;
			GPIO_RESET(GPIOC,2);
			GPIO_RESET(GPIOC,3);
		}
		compteur_nb_5ms=0;
	}
	compteur_nb_5ms++;

}

