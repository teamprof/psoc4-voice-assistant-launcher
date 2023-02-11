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
#include "project.h"
#include "./AppEvent.h"
#include "./AppLog.h"
#include "./Message.h"

/***************************************
 *              Constants
 ****************************************/
#define I2C_SLAVE_ADDR (0x08u)

/* Transfer statuses */
#define TRANSFER_CMPLT (0x00u)
#define TRANSFER_ERROR (0xFFu)

/* PWM duty cycles for zero and fifty percent */
#define PWM_LED_OFF 10001
#define PWM_LED_HALF_POWER 5000

/* Scales x and y value of the touchpad to PWM compare value */
#define PWM_SCALAR 100

/*  */
#define LED_OFF 1
#define LED_ON 0

/********************************************************************************
 * Function Name: ipcSendMessage()
 ******************************************************************************
 * send message to EZ-BLE™ PRoC™ Module (CYBLE-022001-00) via high level I2C api
 *
 * Parameters: msg
 * The message to be written to the slave device (EZ-BLE™ PRoC™ Module)
 *
 * Return:
 * Status of the transfer. There are two statuses
 *  - TRANSFER_CMPLT: transfer completed successfully.
 *  - TRANSFER_ERROR: an error occurred durring transfer.
 *
 ********************************************************************************/
static uint32 ipcSendMessage(const Message *msg)
{
    uint32 status = TRANSFER_ERROR;

    (void)I2C_I2CMasterClearStatus();

    /* Start I2C write and check status */
    if (I2C_I2C_MSTR_NO_ERROR == I2C_I2CMasterWriteBuf(I2C_SLAVE_ADDR,
                                                       (void *)msg, sizeof(*msg),
                                                       I2C_I2C_MODE_COMPLETE_XFER))
    {
        /* If I2C write started without errors,
         * wait until I2C Master completes write transfer */
        while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT))
        {
        }

        /* Report transfer status */
        if (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_ERR_XFER))
        {
            /* Check if all bytes were written */
            if (I2C_I2CMasterGetWriteBufSize() == sizeof(*msg))
            {
                status = TRANSFER_CMPLT;
            }
        }
    }

    return (status);
}

/********************************************************************************
 * Function Name: handlerGesture()
 ******************************************************************************
 * send message to EZ-BLE™ PRoC™ Module (CYBLE-022001-00) via high level I2C api
 *
 * Parameters:
 *  gesture: value returned from CapSense_DecodeWidgetGestures()
 *  xy: value returned from CapSense_GetXYCoordinates()
 *
 * Return:
 *  None
 *
 ********************************************************************************/
static void handlerGesture(uint32 gesture, uint32 xy)
{
    uint32 ipcRst;
    Message msg;

    switch (gesture)
    {
    case CapSense_ONE_FINGER_SINGLE_CLICK:
        DBGLOG(Debug, "CapSense_ONE_FINGER_SINGLE_CLICK");
        break;

    case CapSense_ONE_FINGER_EDGE_SWIPE_LEFT:
        DBGLOG(Debug, "CapSense_ONE_FINGER_EDGE_SWIPE_LEFT");
        break;

    case CapSense_ONE_FINGER_EDGE_SWIPE_RIGTH:
        DBGLOG(Debug, "CapSense_ONE_FINGER_EDGE_SWIPE_RIGTH");
        break;

    case CapSense_ONE_FINGER_ROTATE_CW:
        DBGLOG(Debug, "CapSense_ONE_FINGER_ROTATE_CW");
        PWM_Green_WriteCompare(PWM_LED_HALF_POWER);

        msg.event = EventLaunchApp;
        msg.iParam = AppVoiceAssistant;
        // msg.uParam = 0;
        // msg.lParam = 0L;

        ipcRst = ipcSendMessage(&msg);
        if (TRANSFER_CMPLT != ipcRst)
        {
            DBGLOG(Debug, "ipcSendMessage() returns %lu", ipcRst);
        }
        break;

    case CapSense_ONE_FINGER_ROTATE_CCW:
        DBGLOG(Debug, "CapSense_ONE_FINGER_ROTATE_CCW");
        PWM_Green_WriteCompare(PWM_LED_OFF);
        break;

    default:
        /* Check if the touchpad was touched */
        if (CapSense_GetXYCoordinates(CapSense_TOUCHPAD0_WDGT_ID) != CapSense_TOUCHPAD_NO_TOUCH)
        {
            uint16 Ycord = (uint16)(xy >> 16);
            uint16 Xcord = (uint16)xy;

            /* Change the PWM compare value based on finger position as long as the light was not off */
            PWM_Blue_WriteCompare(PWM_Blue_ReadCompare() == PWM_LED_OFF ? PWM_LED_OFF : (Xcord * PWM_SCALAR));
            PWM_Green_WriteCompare(PWM_Green_ReadCompare() == PWM_LED_OFF ? PWM_LED_OFF : (Ycord * PWM_SCALAR));
        }
        break;
    }
}

/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 *  The main function performs the following actions:
 *   1. Starts all hardware Components
 *   2. Starts the timestamp
 *   3. Initial scan of all CapSense widgets
 *   4. Checks if scan is complete
 *   4. Process all data and update time stamp
 *   5. Checks if there was a gesture or if the touchpad was touched
 *   6. Sends all data to the CapSense Tuner
 *   7. Scans all CapSense widgets and returns to step four
 *
 * Parameters:
 *  None
 *
 * Return:
 *  None
 *
 *******************************************************************************/
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Starts all Componenets */
    PWM_Blue_Start();
    PWM_Green_Start();
    I2C_Start();
    // EZI2C_Start();
    UART_Start();

    CapSense_Start();

    // /* Set up communication data buffer to CapSense data structure to be exposed to I2C master */
    // EZI2C_EzI2CSetBuffer1(sizeof(CapSense_dsRam), sizeof(CapSense_dsRam), (uint8 *)&CapSense_dsRam);

    /* Sets up a callback function using sysTick timer isr, the callback function is part of the CySysTickSetCallback API */
    CapSense_dsRam.timestampInterval = 2u;
    CySysTickStart();
    CySysTickSetCallback(0u, CapSense_IncrementGestureTimestamp);

    PRINTLN("\r\n***********************************************************************************");
    PRINTLN("Voice Assistant Launcher firmware v1.0");
    PRINTLN("***********************************************************************************");

    CapSense_ScanAllWidgets();

    for (;;)
    {
        /* Checks if the scan was completed before trying to process data */
        if (CapSense_NOT_BUSY == CapSense_IsBusy())
        {
            CapSense_ProcessAllWidgets();

            /* Stores the current detected gesture */
            uint32 gesture = CapSense_DecodeWidgetGestures(CapSense_TOUCHPAD0_WDGT_ID);

            /* Stores current finger position on the touchpad */
            uint32 XYcordinates = CapSense_GetXYCoordinates(CapSense_TOUCHPAD0_WDGT_ID);

            handlerGesture(gesture, XYcordinates);

            /* Required to maintain sychronization with tuner interface */
            CapSense_RunTuner();

            /* Initiates the next scan of all widgets */
            CapSense_ScanAllWidgets();
        }

        uint32 ch = UART_UartGetChar();
        if (0u != ch)
        {
            UART_UartPutChar(ch);
        }

        // CyDelay(5000u);
    }

    PWM_Blue_WriteCompare(PWM_LED_HALF_POWER);
    PWM_Green_WriteCompare(PWM_LED_HALF_POWER);

    /* If Led was off turn it to half power, if on turn it off */
    PWM_Blue_WriteCompare(PWM_Blue_ReadCompare() == PWM_LED_OFF ? PWM_LED_HALF_POWER : PWM_LED_OFF);
    PWM_Green_WriteCompare(PWM_Green_ReadCompare() == PWM_LED_OFF ? PWM_LED_HALF_POWER : PWM_LED_OFF);

    /* Turns on the red LED */
    Red_LED_Write(LED_ON);

    /* Turns off the red LED */
    Red_LED_Write(LED_OFF);
}

// /* Refresh interval in milliseconds for fast scan mode */
// #define LOOP_TIME_FASTSCANMODE (10u)

// /* Refresh interval in milliseconds for slow scan mode */
// #define LOOP_TIME_SLOWSCANMODE (200u)

// #define MILLI_SEC_TO_MICRO_SEC (1000u)

// #if (!CY_IP_SRSSV2)
// /* ILO frequency for PSoC 4 S-Series device */
// #define ILO_CLOCK_FACTOR (40u)
// #else
// /* ILO frequency for PSoC 4 device */
// #define ILO_CLOCK_FACTOR (32u)
// #endif

// /* Refresh rate control parameters */
// #define WDT_TIMEOUT_FAST_SCAN (ILO_CLOCK_FACTOR * LOOP_TIME_FASTSCANMODE)
// #define WDT_TIMEOUT_SLOW_SCAN (ILO_CLOCK_FACTOR * LOOP_TIME_SLOWSCANMODE)

// /* This timeout is for changing the refresh interval from fast to slow rate
//  *  The timeout value is WDT_TIMEOUT_FAST_SCAN * SCANMODE_TIMEOUT_VALUE */
// #define SCANMODE_TIMEOUT_VALUE (150u)

// /* Boolean constants */
// #define TRUE (1u)
// #define FALSE (0u)

// #define MIN_PROXIMITY_SIGNAL (CapSense_PROXIMITY0_FINGER_TH - CapSense_PROXIMITY0_HYSTERESIS)

// #define MAX_COMPARE_VALUE (65535u)

// /* Finite state machine states for device operating states */
// typedef enum
// {
//     SENSOR_SCAN = 0x01u,            /* Sensor is scanned in this state */
//     WAIT_FOR_SCAN_COMPLETE = 0x02u, /* CPU is put to sleep in this state */
//     PROCESS_DATA = 0x03u,           /* Sensor data is processed */
//     SLEEP = 0x04u                   /* Device is put to deep sleep */
// } DEVICE_STATE;

// /* Firmware implements two refresh rates for reducing average power consumption */
// typedef enum
// {
//     SLOW_SCAN_MODE = 0u,
//     FAST_SCAN_MODE = 1u
// } SCAN_MODE;

// /*****************************************************************************
//  * Global Variables
//  *****************************************************************************/
// /* Variable to check the WDT interrupt state */
// volatile uint8 wdtInterruptOccured = FALSE;

// volatile uint32 watchdogMatchValue = WDT_TIMEOUT_FAST_SCAN;

/*****************************************************************************
 * Funcation Declaration
 *****************************************************************************/
// /* API to prepare the device for deep sleep */
// void EnterDeepSleepLowPowerMode(uint8 interruptState);

// /* API to configure the WDT timer for controlling scan intervals */
// void WDT_Start(uint32 *wdtMatchValFastMode, uint32 *wdtMatchValSlowMode);

// /* API to get WDT matchvalue to generate precise scan intervals */
// uint32 CalibrateWdtMatchValue(void);

// int main(void)
// {
//     /* Compensated Watchdog match value in fast scan mode */
//     uint32 wdtMatchValFastMode = 0u;

//     /* Compensated Watchdog match value in slow scan mode */
//     uint32 wdtMatchValSlowMode = 0u;

//     /* Variable to store interrupt state */
//     uint8 interruptState = 0u;

//     /* Variable to hold the current device state
//      *  State machine starts with sensor scan state after power-up
//      */
//     DEVICE_STATE currentState = SENSOR_SCAN;

//     /* This variable is used to indicate the current power mode */
//     SCAN_MODE deviceScanMode = FAST_SCAN_MODE;

//     /* This variable is used to implement a software counter. If the value
//      *  of this counter is greater than SCANMODE_TIMEOUT_VALUE, it indicates that the
//      *  proximity sensor was inactive for more than 3s.*/
//     uint16 softCounter = 0;

//     CyGlobalIntEnable; /* Enable global interrupts. */

//     /* Starts all Componenets */
//     PWM_Blue_Start();
//     PWM_Green_Start();
//     I2C_Start();
//     // EZI2C_Start();

//     /* Allow power fluctuation to stop from hardware setup before CapSense starts */
//     CyDelay(300);

//     CapSense_Start();

//     // /* Set up communication data buffer to CapSense data structure to be exposed to I2C master */
//     // EZI2C_EzI2CSetBuffer1(sizeof(CapSense_dsRam), sizeof(CapSense_dsRam), (uint8 *)&CapSense_dsRam);

//     // /* Sets up a callback function using sysTick timer isr, the callback function is part of the CySysTickSetCallback API */
//     // CapSense_dsRam.timestampInterval = 2u;
//     // CySysTickStart();
//     // CySysTickSetCallback(0u, CapSense_IncrementGestureTimestamp);

//     UART_Start();

//     /* Configure proximity sensor parameters and connect it to AMUXBUS */
//     CapSense_CSDSetupWidgetExt(CapSense_PROXIMITY0_WDGT_ID, CapSense_PROXIMITY0_SNS0_ID);

//     /* Watchdog is used to control the loop time in this project and watchdog
//      *  is set to generate interrupt at every LOOP_TIME_FASTSCANMODE in fast scan mode
//      *  and at LOOP_TIME_SLOWSCANMODE in slow scan mode */
//     // WDT_Start(&wdtMatchValFastMode, &wdtMatchValSlowMode);

//     PRINTLN("***********************************************************************************");
//     PRINTLN("Voice Assistant Launcher firmware v0.1");
//     PRINTLN("***********************************************************************************");

//     for (;;)
//     {
//         /* Switch between sensor-scan -> wait-for-scan -> process -> sleep states */
//         switch (currentState)
//         {
//         case SENSOR_SCAN:
//             /* Initiate new scan only if the CapSense hardware is idle */
//             if (CapSense_NOT_BUSY == CapSense_IsBusy())
//             {
//                 if (CapSense_STATUS_RESTART_DONE == CapSense_RunTuner())
//                 {
//                     CapSense_CSDSetupWidgetExt(CapSense_PROXIMITY0_WDGT_ID, CapSense_PROXIMITY0_SNS0_ID);
//                 }
//                 /* Scan widget configured by CSDSetupWidgetExt API */
//                 CapSense_CSDScanExt();

//                 /* Put CPU to sleep while sensor scanning is in progress */
//                 currentState = WAIT_FOR_SCAN_COMPLETE;
//             }
//             break;

//         case WAIT_FOR_SCAN_COMPLETE:
//             /* Device is in CPU Sleep until CapSense scanning is complete or
//              *  device is woken-up by either CapSense interrupt or I2C interrupt */

//             /* Disable interrupts, so that ISR is not serviced while
//              *  checking for CapSense scan status. */
//             interruptState = CyEnterCriticalSection();

//             /* Check if CapSense scanning is complete */
//             if (CapSense_NOT_BUSY != CapSense_IsBusy())
//             {
//                 /* If CapSense scanning is in progress, put CPU to sleep */
//                 CySysPmSleep();
//             }
//             /* If CapSense scanning is complete, process the CapSense data */
//             else
//             {
//                 currentState = PROCESS_DATA;
//             }
//             /* Enable interrupts for servicing ISR */
//             CyExitCriticalSection(interruptState);
//             break;

//         case PROCESS_DATA:

//             /* Set next state to SLEEP */
//             currentState = SLEEP;

//             /* Process proximity widget */
//             CapSense_ProcessWidget(CapSense_PROXIMITY0_WDGT_ID);

//             if (deviceScanMode == FAST_SCAN_MODE)
//             {
//                 /* If proximity sensor is active, reset software counter */
//                 if (CapSense_IsWidgetActive(CapSense_PROXIMITY0_WDGT_ID))
//                 {
//                     uint32 compareVal = 0u;
//                     /* Scales the proximity sensor value to the PWM compare value, the max diff value is about 15% of the raw count */
//                     compareVal = (500 + (25 * (uint32)(CapSense_PROXIMITY0_SNS0_DIFF_VALUE - MIN_PROXIMITY_SIGNAL)));

//                     /* Checks if the proximity value is greater than the PWM limit */
//                     compareVal = ((compareVal >= MAX_COMPARE_VALUE) ? MAX_COMPARE_VALUE : compareVal);
//                     PWM_Blue_WriteCompare(compareVal);
//                     // PWM_WriteCompare(compareVal);
//                     softCounter = 0;
//                 }
//                 else
//                 {
//                     /* If there are no active widgets, turn off the LED */
//                     PWM_Blue_WriteCompare(0);
//                     // PWM_WriteCompare(0);

//                     /* Increment the software counter every LOOP_TIME_FASTSCANMODE if proximity
//                      *  is not detected. */
//                     softCounter++;

//                     /* If finger is not on sensor for SCANMODE_TIMEOUT_VALUE, switch off the
//                      *  LEDs and switch mode to slow scan mode to reduce power consumption */
//                     if (softCounter >= SCANMODE_TIMEOUT_VALUE)
//                     {
//                         /* Watchdog is configured to generate interrupt at LOOP_TIME_SLOWSCANMODE */
//                         watchdogMatchValue = wdtMatchValSlowMode;

// #if (CY_IP_SRSSV2)
//                         /* Configure Match value */
//                         CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, watchdogMatchValue);
// #endif

//                         /* Set mode to slow scan mode to scan sensors at LOOP_TIME_SLOWSCANMODE */
//                         deviceScanMode = SLOW_SCAN_MODE;
//                     }
//                 }
//             }

//             /* If deviceScanMode is SLOW_SCAN_MODE, perform the following tasks */
//             else
//             {
//                 /* If proximity sensor is active, switch to active mode */
//                 if (CapSense_IsWidgetActive(CapSense_PROXIMITY0_WDGT_ID))
//                 {
//                     /* If sensor is active in slow-scan mode, skip sleep
//                      *  and perform sensor scan */
//                     currentState = SENSOR_SCAN;

//                     /* Set watchdog match value to fast scan mode */
//                     watchdogMatchValue = wdtMatchValFastMode;

//                     /* Updates the baseline of the proximity sensor when it switches back
//                      * to fast scan mode*/
//                     CapSense_InitializeWidgetBaseline(CapSense_PROXIMITY0_WDGT_ID);

// #if (CY_IP_SRSSV2)
//                     /* Configure Match value */
//                     CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, watchdogMatchValue);
// #endif

//                     /* Change the device mode to fast scan mode to provide fast touch response */
//                     deviceScanMode = FAST_SCAN_MODE;
//                 }
//             }
//             break;

//         case SLEEP:
//             if (deviceScanMode == SLOW_SCAN_MODE)
//             {
//                 /* Put the device to deep sleep after each CapSense scan */
//                 // EnterDeepSleepLowPowerMode(interruptState);
//             }
//             else
//             {
//                 CySysPmSleep();
//             }

//             /* Start scanning the sensors only if interrupt occurred due to WDT.
//                Interrupt can also occur due to I2C interrupt while tuner is running.
//                In such cases, sensor is not scanned until WDT interrupt has occurred */
//             if (wdtInterruptOccured)
//             {
//                 /* Set state to scan sensor after device wakes up from sleep */
//                 currentState = SENSOR_SCAN;

//                 wdtInterruptOccured = FALSE;
//             }
//             break;

//         default:
//             /*******************************************************************
//              * Unknown power mode state. Unexpected situation.
//              ******************************************************************/
//             CYASSERT(0);
//             break;
//         }
//     }

//     CapSense_ScanAllWidgets();

//     for (;;)
//     {
//         /* Checks if the scan was completed before trying to process data */
//         if (CapSense_NOT_BUSY == CapSense_IsBusy())
//         {
//             CapSense_ProcessAllWidgets();

//             /* Stores the current detected gesture */
//             uint32 gesture = CapSense_DecodeWidgetGestures(CapSense_TOUCHPAD0_WDGT_ID);

//             /* Stores current finger position on the touchpad */
//             uint32 XYcordinates = CapSense_GetXYCoordinates(CapSense_TOUCHPAD0_WDGT_ID);

//             handlerGesture(gesture, XYcordinates);

//             /* Required to maintain sychronization with tuner interface */
//             CapSense_RunTuner();

//             /* Initiates the next scan of all widgets */
//             CapSense_ScanAllWidgets();
//         }

//         uint32 ch = UART_UartGetChar();
//         if (0u != ch)
//         {
//             UART_UartPutChar(ch);
//         }

//         // CyDelay(5000u);
//     }

//     PWM_Blue_WriteCompare(PWM_LED_HALF_POWER);
//     PWM_Green_WriteCompare(PWM_LED_HALF_POWER);

//     /* If Led was off turn it to half power, if on turn it off */
//     PWM_Blue_WriteCompare(PWM_Blue_ReadCompare() == PWM_LED_OFF ? PWM_LED_HALF_POWER : PWM_LED_OFF);
//     PWM_Green_WriteCompare(PWM_Green_ReadCompare() == PWM_LED_OFF ? PWM_LED_HALF_POWER : PWM_LED_OFF);

//     /* Turns on the red LED */
//     Red_LED_Write(LED_ON);

//     /* Turns off the red LED */
//     Red_LED_Write(LED_OFF);
// }