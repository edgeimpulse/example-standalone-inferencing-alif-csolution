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

#include "timer.h"
#include "Driver_UTIMER.h"
#include "clk.h"

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;

#define MICROSECONDS_TO_SECONDS (1000000)
#define MILLISECONDS_TO_SECONDS (1000)

#define US_TIMER_CHANNEL        ARM_UTIMER_CHANNEL0

static uint64_t timer_overflow_times;
static uint64_t div_ratio = 0;
static volatile bool sensor_interrupt;

/**
 * @function    void utimer_basic_mode_cb_func(event)
 * @brief       utimer basic mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_reload_cb (uint8_t event);
static void sensor_utimer_cb (uint8_t event);
static inline uint64_t get_timer_count(void);
static inline void set_timer_overflow_times(uint64_t value);

int timer_us_init(void)
{
    int32_t ret;

    uint32_t count_array[2];

    timer_overflow_times = 0;
    div_ratio = (GetSystemAXIClock() / MICROSECONDS_TO_SECONDS);

    count_array[0] = 0x00000000;   /*< initial counter value >*/
    count_array[1] = 0xFFFFFFFF;    /*< over flow count value >*/

    ret = ptrUTIMER->Initialize (US_TIMER_CHANNEL, utimer_reload_cb);
    if (ret != ARM_DRIVER_OK) {
        //printf("utimer channel %d failed initialize \n", channel);
        return ret;
    }

    ret = ptrUTIMER->PowerControl(US_TIMER_CHANNEL, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = ptrUTIMER->ConfigCounter(US_TIMER_CHANNEL, ARM_UTIMER_MODE_BASIC, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = ptrUTIMER->SetCount(US_TIMER_CHANNEL, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }   

    ret = ptrUTIMER->SetCount(US_TIMER_CHANNEL, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = ptrUTIMER->Start(US_TIMER_CHANNEL);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    return ret;
}

/**
 * @brief 
 * 
 * @param event 
 */
static void utimer_reload_cb (uint8_t event)
{
    if (event & ARM_UTIMER_EVENT_OVER_FLOW) {
        timer_overflow_times++;
    }
}
/**
 *
 * @return
 */
uint32_t timer_get_us(void)
{
    uint64_t overflow_time = ((uint64_t)1 << 32) / div_ratio;

    return (uint32_t)((timer_overflow_times * overflow_time)
         + (get_timer_count()/div_ratio));
}

/**
 *
 * @return
 */
static inline uint64_t get_timer_count(void)
{    
    return ptrUTIMER->GetCount(US_TIMER_CHANNEL, ARM_UTIMER_CNTR);
}
