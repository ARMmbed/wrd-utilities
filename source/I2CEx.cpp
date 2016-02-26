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


#include "wrd-utilities/I2CEx.h"


I2CEx::I2CEx(PinName sda, PinName scl)
    :   I2C(sda, scl),
        transactionInProgress(false),
        processQueueHandle(NULL)
{

}

I2CEx::~I2CEx()
{
    if (processQueueHandle)
    {
        minar::Scheduler::cancelCallback(processQueueHandle);
    }
}

void I2CEx::read(int address,
                 int reg,
                 char* buffer,
                 int length,
                 FunctionPointer0<void> callback)
{
    addQueue(I2CEx::READ, address, reg, buffer, length, callback);
}

void I2CEx::write(int address,
                  int reg,
                  char* buffer,
                  int length,
                  FunctionPointer0<void> callback)
{
    addQueue(I2CEx::WRITE, address, reg, buffer, length, callback);
}

void I2CEx::addQueue(action_t action,
                     int address,
                     int reg,
                     char* buffer,
                     int length,
                     FunctionPointer0<void> callback)
{
    /* add command to queue */
    I2CEx::transaction_t transaction;

    transaction.action   = action;
    transaction.address  = address;
    transaction.reg      = reg;
    transaction.buffer   = buffer;
    transaction.length   = length;
    transaction.callback = callback;

    sendQueue.push(transaction);

    /* schedule queue to be processed */
    if (transactionInProgress == false)
    {
        transactionInProgress = true;
        processQueueHandle = minar::Scheduler::postCallback(this, &I2CEx::processQueue)
                                .tolerance(1)
                                .getHandle();
    }
}

void I2CEx::processQueue(void)
{
    /* clear task handle */
    processQueueHandle = NULL;

    /* only process if queue is not empty */
    if (sendQueue.size() > 0)
    {
        /* get action */
        I2CEx::transaction_t transaction = sendQueue.front();

        /* */
        if (transaction.action == I2CEx::READ)
        {
            writeBuffer[0] = transaction.reg;

#if DEVICE_I2C_ASYNCH
            I2C::event_callback_t callback(this, &I2CEx::i2cDone);
            I2C::transfer(transaction.address,
                          writeBuffer, 1,
                          transaction.buffer, transaction.length,
                          callback);
#else
            I2C::read(transaction.address, writeBuffer, transaction.length);

            FunctionPointer3<void, Buffer, Buffer, int> callback(this, &I2CEx::i2cDone);
            minar::Scheduler::postCallback(callback);
#endif


        }
        else
        {
            writeBuffer[0] = transaction.reg;
            memcpy(&writeBuffer[1], transaction.buffer, transaction.length);

#if DEVICE_I2C_ASYNCH
            I2C::event_callback_t callback(this, &I2CEx::i2cDone);
            I2C::transfer(transaction.address,
                          writeBuffer, transaction.length + 1,
                          transaction.buffer, 0,
                          callback);
#else
            I2C::write(transaction.address, writeBuffer, transaction.length + 1);

            FunctionPointer3<void, Buffer, Buffer, int> callback(this, &I2CEx::i2cDone);
            minar::Scheduler::postCallback(callback);
#endif
        }
    }
    else
    {
        transactionInProgress = false;
    }
}

void I2CEx::i2cDone(Buffer txBuffer, Buffer rxBuffer, int code)
{
    (void) txBuffer;
    (void) rxBuffer;
    (void) code;

    /* get action */
    I2CEx::transaction_t transaction = sendQueue.front();

    /* remove action from queue */
    sendQueue.pop();

    /* signal callback */
    minar::Scheduler::postCallback(transaction.callback);

    /* process next action */
    processQueueHandle = minar::Scheduler::postCallback(this, &I2CEx::processQueue)
                            .tolerance(1)
                            .getHandle();
}
