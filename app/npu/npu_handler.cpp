/*
 * Copyright (c) 2024 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "npu_handler.h"
#include "RTE_Components.h"
#include "RTE_Device.h"
#include CMSIS_device_header
#include "ethosu_driver.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

static struct ethosu_driver npuDriver;

static void npu_irq_handler(void)
{
    ethosu_irq_handler(&npuDriver);
}


int npu_init(void)
{
    int err = 0;

    /* Base address is 0x4000E1000; interrupt number is 55. */
    void* const npuBaseAddr = reinterpret_cast<void*>(LOCAL_NPU_BASE);

    /*  Initialize Ethos-U NPU driver. */
    if (ethosu_init(&npuDriver, /* Arm Ethos-U device driver pointer  */
                    npuBaseAddr, /* Base address for the Arm Ethos-U device */
                    0, /* Cache memory pointer (not applicable for U55) */
                    0, /* Cache memory size */
                    1, /* Secure */
                    1) /* Privileged */ ) {
        ei_printf("Failed to initialize Arm Ethos-U driver");
        return 1;
    }

    NVIC_SetVector(LOCAL_NPU_IRQ_IRQn, (uint32_t) &npu_irq_handler);
    NVIC_EnableIRQ(LOCAL_NPU_IRQ_IRQn);
    
    return 0;
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
void cpu_cache_enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}

void cpu_cache_disable(void)
{
    /* Enable I-Cache */
    SCB_DisableICache();

    /* Enable D-Cache */
    SCB_DisableDCache();
}
