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

    /* Enable MIPI power. TODO: To be changed to aiPM call */
    enable_mipi_dphy_power();
    disable_mipi_dphy_isolation();
    
    if (clock_init() != 0) {

        BOARD_LED1_Control(BOARD_LED_STATE_HIGH);
        BOARD_LED2_Control(BOARD_LED_STATE_HIGH);

        while(1);
    }

    ei_uart_init();    

    //clk_init(); // for time.h clock()

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