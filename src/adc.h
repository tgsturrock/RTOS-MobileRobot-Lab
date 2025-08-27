/**
 ******************************************************************************
 * File Name          : adc.h
 * Description        : ce module contien tout ce qui correspond a la lecture et analyse de l'adc
 * Created            : Jun 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */

#ifndef ADC_H_
#define ADC_H_
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Defines -------------------------------------------------------------------*/
#define ADC_PRIORITY 20

/* Function prototypes ------------------------------------------------------ */
/**
 * @brief  Fonction qui configure le peripherique d'ADC
 * @param  None
 * @retval None
 */
void config_adc(void);

/**
 * @brief  Fonction qui calibre les minimum et maximum de l'ADC
 * @param  None
 * @retval None
 */
void moteur_calibration(void);

/**
 * @brief  Fonction qui defini la valeur moyenne ressu par l'adc depuis
 *         la derniere utilisation de la fonction
 * @param  None
 * @retval None
 */
void vitesse_moyenne_mesure(void);

/**
 * @brief  Fonction qui cree un delais en seconde
 * @param uint16_t time_in_sec : longueur du delais voulu en seconde
 * @retval None
 */
void delay_in_sec(uint16_t time_in_sec);

/**
 * @brief  Fonction qui map les minimum et maximum des d'adc
 * @param  None
 * @retval None
 */
void vitesse_mapping_init(void);

/**
 * @brief  Fonction qui defini la vitesse de chaque moteur
 * @param  float* v_moyenne_gauche : vitesse du moteur gauche
 *         float* v_moyenne_droite : vitesse du moteur droit
 * @retval None
 */
void vitesse_mapping(float* v_moyenne_gauche,float* v_moyenne_droite);


#endif /* ADC_H_ */
