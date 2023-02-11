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
#pragma once
#include "main.h"

#define I2C_READ_BUFFER_SIZE 61  /* Max supported by BCP */
#define I2C_WRITE_BUFFER_SIZE 61 /* Max supported by BCP */

// #define RESET_I2C_READ_DATA
// #define ENABLE_I2C_ONLY_WHEN_CONNECTED

extern uint8 wrBuf[I2C_WRITE_BUFFER_SIZE]; /* I2C write buffer */
extern uint8 rdBuf[I2C_READ_BUFFER_SIZE];  /* I2C read buffer */
// extern uint32 byteCnt;

extern uint8 sendNotifications;

extern void sendI2CNotification(void);
extern void handleI2CTraffic(void);
