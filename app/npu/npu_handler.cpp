/* The Clear BSD License
 *
 * Copyright (c) 2025 EdgeImpulse Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
