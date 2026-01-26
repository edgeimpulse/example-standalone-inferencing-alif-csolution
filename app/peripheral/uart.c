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
#include "ei_uart.h"
#include "pinconf.h"
#include "board.h"
#include <RTE_Device.h>
#include <RTE_Components.h>
#include CMSIS_device_header

#include "Driver_USART.h"


/* UART Driver instance */
#if (defined BOARD_IS_ALIF_APPKIT_B1_VARIANT)
    #define UART_ISTANCE      BOARD_UART1_INSTANCE
    const IRQn_Type uart_irq_nr = UART2_IRQ_IRQn;

#elif (defined BOARD_IS_ALIF_DEVKIT_B0_VARIANT)
    #define UART_ISTANCE      BOARD_UART2_INSTANCE
    const IRQn_Type uart_irq_nr = UART4_IRQ_IRQn;

#elif defined(BOARD_IS_ALIF_DEVKIT_E1C_VARIANT)
    #define UART_ISTANCE      BOARD_UART1_INSTANCE
    const IRQn_Type uart_irq_nr = UART2_IRQ_IRQn;
#else
    #define UART_ISTANCE      0
    #error "Unsupported board variant"
#endif

/* UART Driver */
extern ARM_DRIVER_USART ARM_Driver_USART_(UART_ISTANCE);

/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(UART_ISTANCE);

#define UART_CB_TX_EVENT          1U << 0
#define UART_CB_RX_EVENT          1U << 1
#define UART_CB_RX_TIMEOUT        1U << 2
static volatile uint32_t event_flags_uart;

static void ei_uart_callback(uint32_t event);

static bool initialized = false;

int ei_uart_init(void)
{    
    int32_t ret    = ARM_DRIVER_OK;

    /* pin init */
    //uart_hw_init();

    /* Initialize UART driver */
    ret = USARTdrv->Initialize(ei_uart_callback);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Power up UART peripheral */
    ret = USARTdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Configure UART to 115200 Bits/sec */
    ret =  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                             ARM_USART_DATA_BITS_8       |
                             ARM_USART_PARITY_NONE       |
                             ARM_USART_STOP_BITS_1       |
                             ARM_USART_FLOW_CONTROL_NONE, 115200);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Transmitter line */
    ret =  USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    /* Receiver line */
    ret =  USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }

    initialized = true;
    return ret;

}

/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 */
void ei_uart_send(char* buf, unsigned int len)
{
    int ret = 0;
    if (initialized)
    {
        event_flags_uart = 0;
        int32_t ret = USARTdrv->Send(buf, len);
        if(ret != ARM_DRIVER_OK)
        {
            return;
        }

        while ( (event_flags_uart & ARM_USART_EVENT_SEND_COMPLETE) == 0) {
            __WFE();
        }
    }
}

static void ei_uart_callback(uint32_t event)
{    
    if (event & ARM_USART_EVENT_SEND_COMPLETE) {
        /* Send Success */
        event_flags_uart |= UART_CB_TX_EVENT;
    }

    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        /* Receive Success */
        event_flags_uart |= UART_CB_RX_EVENT;
    }

    if (event & ARM_USART_EVENT_RX_TIMEOUT) {
        /* Receive Success with rx timeout */
        event_flags_uart |= UART_CB_RX_TIMEOUT;
    }
}

static void uart_hw_init(void)
{
#if UART_NUM == 2
    /* UART2_RX_A */
    pinconf_set( PORT_1, PIN_0, PINMUX_ALTERNATE_FUNCTION_1, PADCTRL_READ_ENABLE);

    /* UART2_TX_A */
    pinconf_set( PORT_1, PIN_1, PINMUX_ALTERNATE_FUNCTION_1, 0);
#elif UART_NUM == 4
    /* UART4_RX_B */
    pinconf_set( PORT_12, PIN_1, PINMUX_ALTERNATE_FUNCTION_2, PADCTRL_READ_ENABLE);

    /* UART4_TX_B */
    pinconf_set( PORT_12, PIN_2, PINMUX_ALTERNATE_FUNCTION_2, 0);
#endif
}
