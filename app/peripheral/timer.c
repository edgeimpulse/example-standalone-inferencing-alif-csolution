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

#include "timer.h"
#include "Driver_UTIMER.h"

#if defined(M55_HE)
#define CORE_CLOCK_HZ 160000000
#elif defined(M55_HP)
#define CORE_CLOCK_HZ 400000000 //default to M55_0 core
#else
#error "Wrong core defined
#endif

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;

#define MICROSECONDS_TO_SECONDS 1000000

static uint64_t timer_overflow_times;
static uint64_t div_ratio = 0;
static const uint8_t channel = 0;

/**
 * @function    void utimer_basic_mode_cb_func(event)
 * @brief       utimer basic mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_reload_cb (uint32_t event);
static inline uint64_t get_timer_count(void);
static inline void set_timer_overflow_times(uint64_t value);

int timer_us_init(void)
{
    int32_t ret;

    uint32_t count_array[2];

    timer_overflow_times = 0;
    div_ratio = (CORE_CLOCK_HZ / MICROSECONDS_TO_SECONDS);

    count_array[0] = 0x00000000;   /*< initial counter value >*/
    count_array[1] = 0xFFFFFFFF;    /*< over flow count value >*/

    ret = ptrUTIMER->Initialize (channel, utimer_reload_cb);
    if (ret != ARM_DRIVER_OK) {
        //printf("utimer channel %d failed initialize \n", channel);
        return ret;
    }

    ret = ptrUTIMER->PowerControl(channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = ptrUTIMER->ConfigCounter(channel, ARM_UTIMER_MODE_BASIC, ARM_UTIMER_COUNTER_UP);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR, count_array[0]);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }   

    ret = ptrUTIMER->SetCount(channel, ARM_UTIMER_CNTR_PTR, count_array[1]);
    if (ret != ARM_DRIVER_OK) {
        return ret;
    }

    ret = ptrUTIMER->Start (channel);
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
static void utimer_reload_cb (uint32_t event)
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
    return ptrUTIMER->GetCount(channel, ARM_UTIMER_CNTR);
}
