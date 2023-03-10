/*
 * Copyright (C) 2022 teamprof.net@gmail.com or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "main.h"

uint8 wrBuf[I2C_WRITE_BUFFER_SIZE]; /* I2C write buffer */
uint8 rdBuf[I2C_READ_BUFFER_SIZE];	/* I2C read buffer */
// uint32 byteCnt;						/* variable to store the number of bytes written by I2C mater */

uint8 sendNotifications;	  /* Flag to check notification enabled/disabled */
CYBLE_API_RESULT_T apiResult; /*  variable to store BLE component API return */

/*******************************************************************************
 * Function Name: I2C_Brige_init
 ********************************************************************************
 * Summary:
 *    This function initializes the BLE and I2C component
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void I2CBrigeInit(void)
{
	/* Start BLE operation */
	apiResult = CyBle_Start(AppCallBack);

	if (apiResult != CYBLE_ERROR_OK)
	{

#ifdef LED_INDICATION
		DISCON_LED_ON();
#endif /* LED_INDICATION */

		while (1)
			;

		/* Failed to initialize stack */
	}

#ifndef ENABLE_I2C_ONLY_WHEN_CONNECTED
	/* Start I2C Slave operation */
	I2C_Start();

	/* Initialize I2C write buffer */
	I2C_I2CSlaveInitWriteBuf((uint8 *)wrBuf, I2C_WRITE_BUFFER_SIZE);

	/* Initialize I2C read buffer */
	I2C_I2CSlaveInitReadBuf((uint8 *)rdBuf, I2C_READ_BUFFER_SIZE);
#endif
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        System entrance point. This calls the initializing function and
* continuously process BLE and I2C events.
*
* Parameters:
*  void
*
* Return:
*  int
*

*******************************************************************************/
int main()
{
	/* Enable the Global Interrupt */
	CyGlobalIntEnable;

#ifdef LED_INDICATION
	/* Turn off all LEDS */
	ALL_LED_OFF();
#else
	/* Set the LED pins drive mode to Strong drive */
	DISCON_LED_SetDriveMode(DISCON_LED_DM_ALG_HIZ);
	CONNECT_LED_SetDriveMode(CONNECT_LED_DM_ALG_HIZ);
	ADV_LED_SetDriveMode(ADV_LED_DM_ALG_HIZ);
#endif /* LED_INDICATION */

	I2CBrigeInit();

	for (;;) /* Loop forever */
	{

#ifdef LOW_POWER_MODE
		/* put the BLESS and MCU core to lowest possible power state to save power*/
		handleLowPowerMode();
#endif /* LOW_POWER_MODE */

		/* Process queued BLE events */
		CyBle_ProcessEvents();

#ifdef ENABLE_I2C_ONLY_WHEN_CONNECTED
		if (cyBle_state == CYBLE_STATE_CONNECTED)
		{
			/* Handle I2C read and write */
			handleI2CTraffic();
		}
#else
		handleI2CTraffic();
#endif
	}
}
