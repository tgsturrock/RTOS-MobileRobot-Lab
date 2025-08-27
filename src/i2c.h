/**
 ******************************************************************************
 * File Name          : i2c.h
 * Description        : ce mocule sert de driver i2c
 * Created            : Jun 2022
 * Author             : Thomas Giguere Sturrock
 ******************************************************************************
 */
/* Prevent recursive inclusion -----------------------------------------------*/
#ifndef I2C_H_
#define I2C_H_
/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stm32f0xx.h>
#include "main.h"
/* Defines -------------------------------------------------------------------*/
#define I2C_PRIORITY 30

#define I2CBUFSIZE			64
#define I2C_CR2_NBYTES_POS	16

#define RANGE_TO_ms ((float) 0.256)
#define RANGE_TO_cm ((float) 4.43)

#define MAXRANGE  45
#define MINRANGE  20
#define DELTA_RANGE (float)(MAXRANGE-MINRANGE)
#define MINDISTANCE ((float) MINRANGE*RANGE_TO_cm)
#define MAXDISTANCE ((float) MAXRANGE*RANGE_TO_cm)

#define SONAR_ADR_G ((uint32_t) 0xE0) //Bit1-7 dans le registre I2C
#define SONAR_ADR_D ((uint32_t) 0xE2) //Bit1-7 dans le registre I2C
#define SONAR_PING 	((uint8_t) 0x51)
/* Type definitions ----------------------------------------------------------*/
enum SONAR_REG_W { REG_COMMAND = 0x00, REG_GAIN	= 0x01, REG_RANGE = 0x02, REG_NA = 0x03};
enum SONAR_REG_R { REG_REVISION = 0x00, REG_UNUSED	= 0x01, REG_RANGE_MSB = 0x02, REG_RANGE_LSB = 0x03};
enum SONAR_G_D   { SONAR_GAUCHE = 0, SONAR_DROIT = 1};

enum ENDIANNESS   { LSB_LITTLE_ENDIAN = 0, MSB_LITTLE_ENDIAN = 1};

/* Function prototypes ------------------------------------------------------ */

/**
 * @brief  Fonction qui configure le peripherique d'I2C
 * @param  None
 * @retval None
 */
void Init_I2C(void);

/**
 * @brief  Fonction d'ecriture de l'I2C
 * @param  uint8_t Addr : adresse d'ecriture
 * 		   uint8_t Reg : registre voulue pour l'ecriture
 * 		   uint8_t Val : valeur ecrite
 * @retval None
 */
void I2C_Write(uint8_t Addr, uint8_t Reg, uint8_t Val);

/**
 * @brief  Fonction d'ecriture de l'I2C
 * @param  uint8_t Addr : adresse de lecture
 * 		   uint8_t Reg : registre voulue pour la lecture
 * 		   uint8_t *Val : pointeur de la valeur receptionner par la lecture
 * @retval None
 */
void I2C_Read(uint8_t Addr, uint8_t Reg, uint8_t *Val);

#endif /* I2C_H_ */
