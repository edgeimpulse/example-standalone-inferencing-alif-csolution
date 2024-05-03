/* Edge Impulse inferencing library
 * Copyright (c) 2021 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "peripheral/ei_uart.h"
#include "peripheral/timer.h"

/* Extern function prototypes ---------------------------------------------- */

EI_IMPULSE_ERROR ei_run_impulse_check_canceled()
{
    return EI_IMPULSE_OK;
}

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
EI_IMPULSE_ERROR ei_sleep(int32_t time_ms)
{
    if (time_ms < 0) { 
        return EI_IMPULSE_OK; 
    }

    uint64_t start_time = ei_read_timer_ms();
    // cast so that we get correct wrap around behavior
    while ((ei_read_timer_ms() - start_time) < (uint64_t) time_ms)
        ;
    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_us()
{
    //return Get_SysTick_Cycle_Count() / ( EI_CORE_CLOCK_HZ / 1000000 );
    return timer_get_us();
}

uint64_t ei_read_timer_ms()
{
    return ei_read_timer_us() / 1000;
}

void ei_printf(const char *format, ...)
{
    char buffer[256] = {0};
    int length;

    va_list myargs;
    va_start(myargs, format);
    length = vsnprintf(buffer, sizeof(buffer), format, myargs);
    va_end(myargs);
    
    if (length > 0) {
        ei_uart_send(buffer, length);        
    }
    
}

void ei_printf_float(float f)
{
    float n = f;

    static double PRECISION = 0.00001;
    static int MAX_NUMBER_STRING_SIZE = 32;

    char s[MAX_NUMBER_STRING_SIZE];

    if (n == 0.0) {
        strcpy(s, "0");
    }
    else {
        int digit, m;
        char *c = s;
        int neg = (n < 0);
        if (neg) {
            n = -n;
        }
        // calculate magnitude
        m = log10(n);
        if (neg) {
            *(c++) = '-';
        }
        if (m < 1.0) {
            m = 0;
        }
        // convert the number
        while (n > PRECISION || m >= 0) {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0) {
                *(c++) = '.';
            }
            m--;
        }
        *(c) = '\0';
    }

    ei_printf("%s", s);
}

void ei_putchar(char c) 
{ 
    putchar(c); 
    fflush(stdout);
}

char ei_getchar(void)
{
    //auto c = UartGetcNoBlock();
    char c = 0xFF;
    if (c == 0xFF ) { return 0; } //weird ei convention
    else { return c; ei_printf("ch: %c\r\n", c);}
}

void *ei_malloc(size_t size)
{
    return malloc(size);
}

void *ei_calloc(size_t nitems, size_t size)
{
    return calloc(nitems, size);
}

void ei_free(void *ptr)
{
    free(ptr);
}

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif
    void
    DebugLog(const char *s)
{
    ei_printf("%s", s);
}
