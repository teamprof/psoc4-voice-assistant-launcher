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
#include "app_Ble.h"

/*******************************************************************************
 * Function Name: AppCallBack
 ********************************************************************************
 * Summary:
 *        Call back event function to handle varios events from BLE stack
 *
 * Parameters:
 *  event:		event returned
 *  eventParam:	link to value of the events returned
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void AppCallBack(uint32 event, void *eventParam)
{
	uint8 i;
	CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;

	switch (event)
	{
	case CYBLE_EVT_STACK_ON:
		/* start advertising */
		apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);

		if (apiResult == CYBLE_ERROR_OK)
		{

#ifdef LED_INDICATION
			ADV_LED_ON();
#endif /* LED_INDICATION */
		}
		break;

	case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:

		sendNotifications = 0;

#ifdef ENABLE_I2C_ONLY_WHEN_CONNECTED
		/* Stop I2C Slave operation */
		I2C_Stop();

#endif

#ifdef LED_INDICATION
		/* Indicate disconnect event to user */
		DISCON_LED_ON();

		CyDelay(3000);
#endif /* LED_INDICATION */

		/* start advertising */
		apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);

		if (apiResult == CYBLE_ERROR_OK)
		{
#ifdef LED_INDICATION
			ADV_LED_ON();
#endif /* LED_INDICATION */
		}
		break;

	case CYBLE_EVT_GATT_CONNECT_IND:

#ifdef LED_INDICATION
		CONNECT_LED_ON();
#endif /* LED_INDICATION */

#ifdef ENABLE_I2C_ONLY_WHEN_CONNECTED
		/* Start I2C Slave operation */
		I2C_Start();

		/* Initialize I2C write buffer */
		I2C_I2CSlaveInitWriteBuf((uint8 *)wrBuf, I2C_WRITE_BUFFER_SIZE);

		/* Initialize I2C read buffer */
		I2C_I2CSlaveInitReadBuf((uint8 *)rdBuf, I2C_READ_BUFFER_SIZE);
#endif
		break;

	/* Client may do Write Value or Write Value without Response. Handle both */
	case CYBLE_EVT_GATTS_WRITE_REQ:
	case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
		wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam;

		/* Handling Notification Enable */
		if (wrReqParam->handleValPair.attrHandle == CYBLE_VOICE_ASSISTANT_LAUNCHER_TXCHARACTERISTIC_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
		{
			CYBLE_GATT_HANDLE_VALUE_PAIR_T I2CNotificationCCDHandle;
			uint8 I2CCCDValue[2];

			/* Extract CCCD Notification enable flag */
			sendNotifications = wrReqParam->handleValPair.value.val[0];

			/* Write the present I2C notification status to the local variable */
			I2CCCDValue[0] = sendNotifications;

			I2CCCDValue[1] = 0x00;

			/* Update CCCD handle with notification status data*/
			I2CNotificationCCDHandle.attrHandle = CYBLE_VOICE_ASSISTANT_LAUNCHER_TXCHARACTERISTIC_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;

			I2CNotificationCCDHandle.value.val = I2CCCDValue;

			I2CNotificationCCDHandle.value.len = 2;

			/* Report data to BLE component for sending data when read by Central device */
			CyBle_GattsWriteAttributeValue(&I2CNotificationCCDHandle, 0, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);
		}

		/* Handling Write data from Client */
		else if (wrReqParam->handleValPair.attrHandle == CYBLE_VOICE_ASSISTANT_LAUNCHER_RXCHARACTERISTIC_CHAR_HANDLE)
		// else if (wrReqParam->handleValPair.attrHandle == CYBLE_I2C_WRITE_I2C_WRITE_DATA_CHAR_HANDLE)
		{
			/* Turn off I2C interrupt before updating read registers */
			I2C_DisableInt();

			/*The data received from I2C client is extracted */
			for (i = 0; i < (wrReqParam->handleValPair.value.len); i++)
				rdBuf[i] = wrReqParam->handleValPair.value.val[i];

			/* Turn on I2C interrupt after updating read registers */
			I2C_EnableInt();
		}

		if (event == CYBLE_EVT_GATTS_WRITE_REQ)
		{
			CyBle_GattsWriteRsp(cyBle_connHandle);
		}
		break;

	default:
		break;
	}
}
