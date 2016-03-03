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

#include "mbed-drivers/mbed.h"
#include "wrd-utilities/I2CRegister.h"

I2CRegister i2c(YOTTA_CFG_HARDWARE_WEARABLE_REFERENCE_DESIGN_EXTERNAL_GPIO_GPIO0_I2C_SDA,
                YOTTA_CFG_HARDWARE_WEARABLE_REFERENCE_DESIGN_EXTERNAL_GPIO_GPIO0_I2C_SCL);

uint8_t writeBuffer[10];
uint8_t readBuffer[10];

/*****************************************************************************/
/* App start                                                                 */
/*****************************************************************************/

void i2cDone(void)
{
    printf("done\r\n");
}

void i2cWriteDone(void)
{
    i2c.read(0x42, 0x02, readBuffer, 2, i2cDone);
}

void app_start(int, char *[])
{
    i2c.frequency(400000);

    writeBuffer[0] = 0xAB;
    writeBuffer[1] = 0xCD;

    i2c.write(0x42, 0x02, writeBuffer, 2, i2cWriteDone);
}

