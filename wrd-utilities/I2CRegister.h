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

#ifndef __WRD_I2C_REGISTER_H__
#define __WRD_I2C_REGISTER_H__

#include "mbed-drivers/v2/I2C.hpp"
#include "mbed-drivers/mbed.h"

using namespace mbed::util;

class I2CRegister : public mbed::drivers::v2::I2C
{
public:
    I2CRegister(PinName _sda, PinName _scl);

    void read(uint16_t address, uint8_t reg, void* buffer, size_t length, FunctionPointer0<void> callback);

    template <typename T>
    void read(uint16_t address, uint8_t reg, void* buffer, size_t length, T* object, void (T::*member)(void))
    {
        FunctionPointer0<void> fp(object, member);
        read(address, reg, buffer, length, fp);
    }

    void write(uint16_t address, uint8_t reg, void* buffer, size_t length, FunctionPointer0<void> callback);

    template <typename T>
    void write(uint16_t address, uint8_t reg, void* buffer, size_t length, T* object, void (T::*member)(void))
    {
        FunctionPointer0<void> fp(object, member);
        write(address, reg, buffer, length, fp);
    }

private:

    bool notBusy;
    uint8_t* writeBuffer;
    void i2cDone(mbed::drivers::v2::I2CTransaction * t, uint32_t event);
    FunctionPointer0<void> callbackHandle;
};

#endif // __WRD_I2C_REGISTER_H__
