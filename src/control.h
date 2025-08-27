/**
 ******************************************************************************
 * File Name          : control.h
 * Description        : ce mocule contien la structure de donne de control
 * Created            : Jun 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */
/* Prevent recursive inclusion -----------------------------------------------*/
#ifndef CONTROL_H_
#define CONTROL_H_


/* Includes ------------------------------------------------------------------*/

typedef struct {
	volatile uint8_t new_commande;
	volatile uint8_t old_commande;
	volatile float new_vitesse_uart;
	volatile float old_vitesse_uart;
	volatile float new_angle_uart;
	volatile float old_angle_uart;
	volatile float vitesse;
	volatile float angle;
	volatile int32_t v_moyenne_gauche;
	volatile int32_t v_moyenne_droite;

} control_struct_t;

/* Defines -------------------------------------------------------------------*/
#define PI_SUR_4 (float)0.785398
/* Type definitions ----------------------------------------------------------*/

/* Function prototypes ------------------------------------------------------ */


/**
 * @brief  init les variable de la structure a 0
 * @param  None
 * @retval None
 */
void initControl(control_struct_t *control);


/**
 * @brief  met la vielle valeur de commande dans old_commande et update la nouvelle valeur de commande
 * @param  control_struct_t *control : structure de controle a updater
 *         uint8_t commande : nouvelle valeur de commende
 * @retval None
 */
void updateCommande(control_struct_t *control,uint8_t commande);

/**
 * @brief  met la vielle valeur de vitesse uart dans old_vitesse_uart et update la nouvelle valeur de vitesse uart
 * @param  control_struct_t *control : structure de controle a updater
 *         uint8_t vitesse : nouvelle valeur de vitesse uart
 * @retval None
 */
void updateVitesseUart(control_struct_t *control,float vitesse);

/**
 * @brief  met la vielle valeur d'angle uart dans old_angle_uart et update la nouvelle valeur d'angle uart
 * @param  control_struct_t *control : structure de controle a updater
 *         uint8_t vitesse : nouvelle valeur d'angle uart
 * @retval None
 */
void updateAngleUart(control_struct_t *control,float angle);


/**
 * @brief  transforme les vitesse et angle ressu par le uart en valeur utilisable par la fonction d'asservicement
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
void updateVitesse_angle(control_struct_t *control);


/**
 * @brief  accesseur de new_commande
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
uint8_t pullCommande(control_struct_t *control);

/**
 * @brief  accesseur de vitesse
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
float pullVitesse(control_struct_t *control);

/**
 * @brief  accesseur de angle
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
float pullAngle(control_struct_t *control);


/**
 * @brief  fonction qui cree les fait l'asservicement en prenant en compte les obstacle
 * @param  uint8_t sonar_droit : etat a 1 si il y a un obstacle a droite sinon 0
 * 		   uint8_t sonar_gauche : etat a 1 si il y a un obstacle a gauche sinon 0
 * 		   control_struct_t *control : structure de controle a updater
 * 		   float *Duty_G : vitesse voulue pour le moteur gauche
 * 		   float *Duty_D : vitesse voulue pour le moteur droit
 * @retval None
 */
void control_tsk(uint8_t sonar_droit,uint8_t sonar_Gauche,control_struct_t *control, float *Duty_G, float *Duty_D);

#endif /* CONTROL_H_ */
