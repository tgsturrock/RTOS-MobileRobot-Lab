/**
 * @file        i2c.c
 * @brief       Hardware driver for the I2C bus.
 *
 * @details     This module provides the low-level functions required to
 * configure and manage the I2C1 peripheral in master mode. It uses an
 * interrupt-driven circular buffer to handle asynchronous I2C read and
 * write transactions, which are primarily used for communication with
 * peripheral devices, such as sonar sensors.
 *
 * @author      Thomas Giguere Sturrock
 * @date        Jun 2022
 */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include "moteur.h"

// Tampon de trame du I2C
volatile uint32_t I2CBuf[I2CBUFSIZE];
volatile uint16_t I2CBufIn = 0, I2CBufOut = 0;

uint8_t	 	SonarRange			= MINRANGE;
float	 	SonarMaxDistance	= MINDISTANCE;
uint16_t	SonarObstacle[2] 	= {0, 0};
uint8_t     *addresse_rx;

/* Private function prototypes -----------------------------------------------*/

/* Public functions  ---------------------------------------------------------*/

/**
 * @brief  Fonction qui calibre les minimum et maximum de l'I2C
 * @param  None
 * @retval None
 */
__INLINE void Init_I2C(void) {
	/* Configure et active l'horloge de I2C1 */
	RCC->CFGR3 |= RCC_CFGR3_I2C1SW;

	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	/*Disable I2C*/
	I2C1->CR1 = (uint32_t) 0x00000000; //l'adresse indiqu�e dans la trame qui se trouve dans le tampon
	I2C1->CR2 = (uint32_t) 0x00000000;

	/* Configure GPIOB => PB6 (SCL) et PB7 (SDA) */
	GPIO_OTYPE_CONFIG(GPIOB,6,1);
	GPIO_OTYPE_CONFIG(GPIOB,7,1);

	GPIO_MODE_CONFIG(GPIOB,6,GPIO_ALT_FUNC);
	GPIO_MODE_CONFIG(GPIOB,7,GPIO_ALT_FUNC);

	GPIO_ALTFUN_CONFIG(GPIOB, 6, 1);
	GPIO_ALTFUN_CONFIG(GPIOB, 7, 1);I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;

	GPIOB->OSPEEDR &= ~ GPIO_OSPEEDR_OSPEEDR6_0;	//On defini la vitesse du slew rate du SCL a 2MHz
	GPIOB->OSPEEDR &= ~ GPIO_OSPEEDR_OSPEEDR7_0;	//On defini la vitesse du slew rate du SDA a 2MHz

	GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR6_0 | GPIO_PUPDR_PUPDR7_0);


	/* Configure I2C1, master */
	/* Mode Standard @ 100kHz avec I2CCLK = 48MHz, rise time = 100ns, fall time = 10ns, Analog Filter ON */
	I2C1->TIMINGR = (uint32_t) 0x10805E89;

	/* Permet les interruptions */
	I2C1->CR1 |= (I2C_CR1_TXIE | I2C_CR1_TCIE | I2C_CR1_RXIE);

	/* Permet l'interruption du I2C1 dans le NVIC */
	NVIC->ISER[0] = (((uint32_t) 1) << (I2C1_IRQn & 0x1F));
	NVIC->IP[_IP_IDX(I2C1_IRQn)] = (NVIC->IP[_IP_IDX(I2C1_IRQn)] & ~(0xFF << _BIT_SHIFT(I2C1_IRQn))) |
			(((I2C_PRIORITY << (8 - __NVIC_PRIO_BITS)) & 0xFF) << _BIT_SHIFT(I2C1_IRQn));
	/* Active le I2C1 */
	I2C1->CR1 |= I2C_CR1_PE;
}


void I2C1_IRQHandler(void) {
	uint32_t status;

	status = I2C1->ISR;	// R�cup�re le status du I2C
	switch (status & ((uint32_t) (I2C_ISR_TXIS | I2C_ISR_TC | I2C_ISR_RXNE))) {
	case (I2C_ISR_TC | I2C_ISR_RXNE) :
		// Ceci corrige un probl�me occasionnel.
		// Normalement, on re�oit le drapeau RXNE en premier et le drapeau TC un peu plus tard.
		// Mais, il arrive que pour des retards de Timing, on les re�oit tous les deux en m�me temps.
		// Si c'est le cas, on doit les traiter tous les deux.
		/*
		 * TC
		*/
		if(I2CBufOut==I2CBufIn){
			I2C1->CR2 |= I2C_CR2_STOP;	// Effectue un STOP
		}else{
			I2C1->CR2 = I2CBuf[I2CBufOut];	// Initialise l'I2C avec le d�but de la trame (adresse du sonar et commande I2C)
			I2CBufOut = (I2CBufOut + 1) % I2CBUFSIZE;
			I2C1->CR2 |= I2C_CR2_START;	// Effectue un STOP
		}
		/*
		 * RXNE
		 */
		I2CBuf[I2CBufOut] = (I2C1->RXDR & I2C_RXDR_RXDATA);
		I2CBufOut = (I2CBufOut + 1) % I2CBUFSIZE;
		break;

	case I2C_ISR_TC :	// Une s�quence de transmission est compl�t�e


		if(I2CBufOut==I2CBufIn){
			I2C1->CR2 |= I2C_CR2_STOP;	// Effectue un STOP
		}else{
			I2C1->CR2 = I2CBuf[I2CBufOut];	// Initialise l'I2C avec le d�but de la trame (adresse du sonar et commande I2C)
			I2CBufOut = (I2CBufOut + 1) % I2CBUFSIZE;
			I2C1->CR2 |= I2C_CR2_START;	// Effectue un STOP
		}

		break;
	case I2C_ISR_TXIS :	 // La donn�e a �t� transmise et on est pr�t pour la prochaine

		I2C1->TXDR = I2CBuf[I2CBufOut] & I2C_TXDR_TXDATA;
		I2CBufOut = (I2CBufOut + 1) % I2CBUFSIZE;
		break; //l'adresse indiqu�e dans la trame qui se trouve dans le tampon

	case I2C_ISR_RXNE :	 // Une donn�e a �t� re�ue en provenance de l'esclave
		// Place la donn�e � l'adresse indiqu�e dans la trame qui se trouve dans le tampon

		addresse_rx = (uint8_t*)I2CBuf[I2CBufOut];
		*addresse_rx = (uint8_t)(I2C1->RXDR & I2C_RXDR_RXDATA);
		I2CBufOut = (I2CBufOut + 1) % I2CBUFSIZE;
		break;

	default :			 break;
	}
}

/**
 * @brief  Fonction d'ecriture de l'I2C
 * @param  uint8_t Addr : adresse d'ecriture
 * 		   uint8_t Reg : registre voulue pour l'ecriture
 * 		   uint8_t Val : valeur ecrite
 * @retval None
 */
__INLINE void I2C_Write(uint8_t Addr, uint8_t Reg, uint8_t Val) {
	// Ins�re une trame de Write dans le tampon et d�marre la s�quence du I2C, si n�cessaire
	I2CBuf[I2CBufIn] = (((uint32_t) 2) << I2C_CR2_NBYTES_POS) | ((uint32_t) Addr);	// Place l'adresse du sonar dans le tampon
	I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;
	I2CBuf[I2CBufIn] = (uint32_t) Reg;	// Place le num�ro de registre du sonar dans le tampon
	I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;
	I2CBuf[I2CBufIn] = (uint32_t) Val;	// Place la donn�e � transmettre au registre du sonar dans le tampon
	I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;

	if ((I2C1->ISR & I2C_ISR_BUSY) == 0) {	// Si l'I2C est inactif
		I2C1->CR2 = I2CBuf[I2CBufOut];	// Initialise l'I2C avec le d�but de la trame (adresse du sonar et commande I2C)
		I2CBufOut = (I2CBufOut + 1) % I2CBUFSIZE;
		I2C1->CR2 |= I2C_CR2_START;	// Effectue un START
	}
}

/**
 * @brief  Fonction d'ecriture de l'I2C
 * @param  uint8_t Addr : adresse de lecture
 * 		   uint8_t Reg : registre voulue pour la lecture
 * 		   uint8_t *Val : pointeur de la valeur receptionner par la lecture
 * @retval None
 */
__INLINE void I2C_Read(uint8_t Addr, uint8_t Reg, uint8_t *Val) {
	// Ins�re une trame de Read dans le tampon et d�marre la s�quence du I2C, si n�cessaire
	// Une trame de Read contient un morceau de trame de Write suivit d'un morceau de trame de Read
	I2CBuf[I2CBufIn] = (((uint32_t) 1) << I2C_CR2_NBYTES_POS) | ((uint32_t) Addr);	// Place l'adresse Write du sonar dans le tampon
	I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;
	I2CBuf[I2CBufIn] = (uint32_t) Reg;	// Place le num�ro de registre du sonar qu'on veut lire, dans le tampon
	I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;
	I2CBuf[I2CBufIn] = (((uint32_t) 1) << I2C_CR2_NBYTES_POS) | I2C_CR2_RD_WRN | ((uint32_t) Addr);	// Place l'adresse Read du sonar dans le tampon
	I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;
	I2CBuf[I2CBufIn] = (uint32_t) Val;	// Place l'adresse o� la donn�e lue devra �tre plac�e, dans le tampon
	I2CBufIn = (I2CBufIn + 1) % I2CBUFSIZE;

	if ((I2C1->ISR & I2C_ISR_BUSY) == 0) {	// Si l'I2C est inactif
		I2C1->CR2 = I2CBuf[I2CBufOut];	// Initialise l'I2C avec le d�but de la trame (adresse du sonar et commande I2C)
		I2CBufOut = (I2CBufOut + 1) % I2CBUFSIZE;
		I2C1->CR2 |= I2C_CR2_START;	// Effectue un START
	}
}


/*EOF*/
