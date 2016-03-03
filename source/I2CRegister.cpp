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


#include "wrd-utilities/I2CRegister.h"

using namespace mbed::drivers::v2;

I2CRegister::I2CRegister(PinName sda, PinName scl)
    :   I2C(sda, scl),
        notBusy(true),
        writeBuffer(NULL)
{

}

bool I2CRegister::read(uint16_t address,
                       uint8_t reg,
                       void* buffer,
                       size_t length,
                       FunctionPointer0<void> callback)
{
    bool result = false;

    if (notBusy)
    {
        notBusy = false;
        callbackHandle = callback;

        FunctionPointer2<void, I2CTransaction*, uint32_t>
        fp(this, &I2CRegister::i2cDone);

        I2CError retval = I2C::transfer_to(address)
                              .tx_ephemeral(&reg, 1)
                              .rx(buffer, length)
                              .on(I2C_EVENT_ALL, fp)
                              .apply();

        result = (retval == I2CError::None);
    }

    return result;
}

bool I2CRegister::write(uint16_t address,
                        uint8_t reg,
                        void* buffer,
                        size_t length,
                        FunctionPointer0<void> callback)
{
    bool result = false;

    if (notBusy)
    {
        notBusy = false;
        callbackHandle = callback;

        I2CError retval;
        FunctionPointer2<void, I2CTransaction*, uint32_t>
        fp(this, &I2CRegister::i2cDone);

        if (length < 7)
        {
            // stack allocated buffer
            uint8_t tempBuffer[length + 1];

            // set register and copy payload into buffer
            tempBuffer[0] = reg;
            memcpy(&tempBuffer[1], buffer, length);

            // setup i2c transfer, use ephemeral call to copy temporary buffer
            retval = I2C::transfer_to(address)
                         .tx_ephemeral(tempBuffer, length + 1)
                         .on(I2C_EVENT_ALL, fp)
                         .apply();
        }
        else
        {
            // heap allocated buffer
            uint8_t* tempBuffer = new uint8_t[length + 1];

            // copy to global pointer for cleanup
            writeBuffer = tempBuffer;

            // set register and copy payload into buffer
            tempBuffer[0] = reg;
            memcpy(&tempBuffer[1], buffer, length);

            // setup i2c transfer
            retval = I2C::transfer_to(address)
                         .tx(tempBuffer, length + 1)
                         .on(I2C_EVENT_ALL, fp)
                         .apply();
        }

        result = (retval == I2CError::None);
    }

    return result;
}

void I2CRegister::i2cDone(I2CTransaction* transaction, uint32_t event)
{
    (void) transaction;
    (void) event;

    notBusy = true;

    if (writeBuffer)
    {
        delete[] writeBuffer;
        writeBuffer = NULL;
    }

    if (callbackHandle)
    {
        minar::Scheduler::postCallback(callbackHandle)
            .tolerance(1);
    }
}
