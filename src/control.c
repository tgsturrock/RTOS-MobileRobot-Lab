/**
 ******************************************************************************
 * File Name          : control.c
 * Description        : ce mocule contien la structure de donne de control
 * Created            : Jun 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "control.h"
#include "adc.h"
#include "moteur.h"
/* Defines -------------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public functions  ---------------------------------------------------------*/

/**
 * @brief  init les variable de la structure a 0
 * @param  None
 * @retval None
 */
void initControl(control_struct_t *control){

	control->new_commande = 0;
	control->new_vitesse_uart = 0;
	control->new_angle_uart = 0;
	control->old_commande = 0;
	control->old_vitesse_uart = 0;
	control->old_angle_uart = 0;
	control->vitesse = 0;
	control->angle = 0;
	control->v_moyenne_gauche = 0;
	control->v_moyenne_droite = 0;


	//and more to come
}

/**
 * @brief  met la vielle valeur de commande dans old_commande et update la nouvelle valeur de commande
 * @param  control_struct_t *control : structure de controle a updater
 *         uint8_t commande : nouvelle valeur de commende
 * @retval None
 */
void updateCommande(control_struct_t *control,uint8_t commande){
	control->old_commande=control->new_commande;
	control->new_commande = commande;

}

/**
 * @brief  met la vielle valeur de vitesse uart dans old_vitesse_uart et update la nouvelle valeur de vitesse uart
 * @param  control_struct_t *control : structure de controle a updater
 *         uint8_t vitesse : nouvelle valeur de vitesse uart
 * @retval None
 */
void updateVitesseUart(control_struct_t *control,float vitesse){
	control->old_vitesse_uart=control->new_vitesse_uart;
	control->new_vitesse_uart = vitesse;
}

/**
 * @brief  met la vielle valeur d'angle uart dans old_angle_uart et update la nouvelle valeur d'angle uart
 * @param  control_struct_t *control : structure de controle a updater
 *         uint8_t vitesse : nouvelle valeur d'angle uart
 * @retval None
 */
void updateAngleUart(control_struct_t *control,float angle){
	control->old_angle_uart=control->new_angle_uart;
	control->new_angle_uart=angle;
}


/**
 * @brief  transforme les vitesse et angle ressu par le uart en valeur utilisable par la fonction d'asservicement
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
void updateVitesse_angle(control_struct_t *control){
	controlData.vitesse=(controlData.new_vitesse_uart-100.0)/100.0;
	controlData.angle=controlData.new_angle_uart*0.034906585039887;
}

/**
 * @brief  accesseur de new_commande
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
uint8_t pullCommande(control_struct_t *control){
	return control->new_commande;
}

/**
 * @brief  accesseur de vitesse
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
float pullVitesse(control_struct_t *control){
	return control->vitesse;
}

/**
 * @brief  accesseur de angle
 * @param  control_struct_t *control : structure de controle a updater
 * @retval None
 */
float pullAngle(control_struct_t *control){
	return control->angle;
}

/**
 * @brief  fonction qui cree les fait l'asservicement en prenant en compte les obstacle
 * @param  uint8_t sonar_droit : etat a 1 si il y a un obstacle a droite sinon 0
 * 		   uint8_t sonar_gauche : etat a 1 si il y a un obstacle a gauche sinon 0
 * 		   control_struct_t *control : structure de controle a updater
 * 		   float *Duty_G : vitesse voulue pour le moteur gauche
 * 		   float *Duty_D : vitesse voulue pour le moteur droit
 * @retval None
 */
void control_tsk(uint8_t sonar_droit,uint8_t sonar_gauche,control_struct_t *control, float *duty_g, float *duty_d){


	float v_moyenne_gauche=0;
	float v_moyenne_droite=0;

	float angle_corriger = 0;

	/*
	 * cree les vitesse de chaque moteur selon l'adc
	 */
	vitesse_moyenne_mesure();
	vitesse_mapping(&v_moyenne_gauche,&v_moyenne_droite);

	/*
	 * si un des sonar est activer tourne a +/-45 degree selon quelle sonar est activer sinon garde la direction normal
	 */
	if(sonar_droit||sonar_gauche){
		if(sonar_droit)
			angle_corriger=controlData.angle+PI_SUR_4;//angle +45 degree
		else
			angle_corriger=controlData.angle-PI_SUR_4;//angle -45 degree
	}else
		angle_corriger =controlData.angle;


	CalculPWM(controlData.vitesse, angle_corriger, v_moyenne_gauche, v_moyenne_droite, duty_g, duty_d);
}



