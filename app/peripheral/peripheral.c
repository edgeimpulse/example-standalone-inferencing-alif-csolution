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
#include "RTE_Components.h"
#include "RTE_Device.h"
#include CMSIS_device_header
#include "peripheral.h"
#include "board.h"
#include "se_services_port.h"
#include <stdio.h>
#include "power.h"
#include "ei_uart.h"
#include "timer.h"
#include <string.h> // memcpy

static uint32_t clock_init(void);

extern void clk_init(void); // retarget.c
static void copy_vtor_table_to_ram();

/**
 * @brief 
 * 
 */
void peripheral_init(void)
{
    copy_vtor_table_to_ram();

    BOARD_Pinmux_Init();

    BOARD_LED1_Control(BOARD_LED_STATE_LOW);
    BOARD_LED2_Control(BOARD_LED_STATE_LOW);

    /* Initialize the SE services */
    se_services_port_init();
    
    if (clock_init() != 0) {

        BOARD_LED1_Control(BOARD_LED_STATE_HIGH);
        BOARD_LED2_Control(BOARD_LED_STATE_HIGH);

        while(1);
    }

    ei_uart_init();    

    timer_us_init();
}

/**
 * @brief 
 * 
 */
static uint32_t clock_init(void)
{
    uint32_t service_error_code = 0;

    /* Enable Clocks */
    uint32_t error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_CLK_100M, true, &service_error_code);
    if (error_code || service_error_code){
        //printf("SE: 100MHz clock enable error_code=%u se_error_code=%u\n", error_code, service_error_code);
        return error_code;
    }

//#if (RTE_UART2_CLK_SOURCE == 0) /* CLK_38.4MHz */
    error_code = SERVICES_clocks_enable_clock(se_services_s_handle, CLKEN_HFOSC, true, &service_error_code);
    if (error_code || service_error_code){
        //printf("SE: HFOSC enable error_code=%u se_error_code=%u\n", error_code, service_error_code);
        return error_code;
    }
//#endif

    return error_code;
}


static VECTOR_TABLE_Type MyVectorTable[496] __attribute__((aligned (2048))) __attribute__((section (".bss.noinit.ram_vectors")));
static void copy_vtor_table_to_ram()
{
    if (SCB->VTOR == (uint32_t) MyVectorTable) {
        return;
    }
    memcpy(MyVectorTable, (const void *) SCB->VTOR, sizeof MyVectorTable);
    __DMB();
    // Set the new vector table into use.
    SCB->VTOR = (uint32_t) MyVectorTable;
    __DSB();
}