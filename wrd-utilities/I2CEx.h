/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __WRD_I2CEX_H__
#define __WRD_I2CEX_H__

#include "mbed-drivers/mbed.h"
#include <queue>

using namespace mbed::util;

#define MAX_WRITE_LENGTH 4

class I2CEx : public I2C
{
public:
    I2CEx(PinName _sda, PinName _scl);
    ~I2CEx(void);

    void read(int address, int reg, char* buffer, int length, FunctionPointer0<void> callback);

    template <typename T>
    void read(int address, int reg, char* buffer, int length, T* object, void (T::*member)(void))
    {
        FunctionPointer0<void> fp(object, member);
        read(address, reg, buffer, length, fp);
    }

    void write(int address, int reg, char* buffer, int length, FunctionPointer0<void> callback);

    template <typename T>
    void write(int address, int reg, char* buffer, int length, T* object, void (T::*member)(void))
    {
        FunctionPointer0<void> fp(object, member);
        write(address, reg, buffer, length, fp);
    }

private:

    typedef enum {
        READ,
        WRITE
    } action_t;

    typedef struct {
        action_t action;
        int address;
        int reg;
        char* buffer;
        int length;
        FunctionPointer0<void> callback;
    } transaction_t;

    void addQueue(action_t action,
                  int address,
                  int reg,
                  char* buffer,
                  int length,
                  FunctionPointer0<void> callback);

    void processQueue(void);
    void i2cDone(Buffer txBuffer, Buffer rxBuffer, int code);

    bool transactionInProgress;
    minar::callback_handle_t processQueueHandle;
    std::queue<transaction_t> sendQueue;

    char writeBuffer[MAX_WRITE_LENGTH];
};


#endif // __WRD_I2CEX_H__
