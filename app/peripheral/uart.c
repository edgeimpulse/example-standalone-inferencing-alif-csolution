
#include "ei_uart.h"
#include "pinconf.h"
#include "board.h"
#include <RTE_Device.h>
#include <RTE_Components.h>
#include CMSIS_device_header

#include "Driver_USART.h"

#define UART_NUM 2


#if UART_NUM == 2
extern ARM_DRIVER_USART ARM_Driver_USART_(BOARD_UART1_INSTANCE);
/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(BOARD_UART1_INSTANCE);
#elif UART_NUM == 4
extern ARM_DRIVER_USART ARM_Driver_USART_(BOARD_UART2_INSTANCE);
/* UART Driver instance */
static ARM_DRIVER_USART *USARTdrv = &ARM_Driver_USART_(BOARD_UART2_INSTANCE);
#endif

#define UART_CB_TX_EVENT          1U << 0
#define UART_CB_RX_EVENT          1U << 1
#define UART_CB_RX_TIMEOUT        1U << 2
static volatile uint32_t event_flags_uart;

static void ei_uart_callback(uint32_t event);

static bool initialized;

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
