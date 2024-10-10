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
#include CMSIS_device_header
#include <stdio.h>
#include "peripheral/peripheral.h"
#include "npu/npu_handler.h"
#include "ei_run_classifier.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "model-parameters/model_metadata.h"
#include "board.h"

static const float features[] = { 
        // copy raw features here (for example from the 'Live classification' page)

};

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr)
{
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

int main (void)
{
    ei_impulse_result_t result = {nullptr};

    peripheral_init();
    if (npu_init()) {
        BOARD_LED1_Control(BOARD_LED_STATE_TOGGLE);
        BOARD_LED2_Control(BOARD_LED_STATE_TOGGLE);
        ei_sleep(1000);
    }

    cpu_cache_enable();

    ei_printf("Edge Impulse standalone inferencing (Alif csolution)\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %d items, but had %u\n",
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        return 1;
    }

    while(1) {
        // the features are stored into flash, and we don't want to load everything into RAM
        signal_t features_signal;
        features_signal.total_length = sizeof(features) / sizeof(features[0]);
        features_signal.get_data = &raw_feature_get_data;

        // invoke the impulse
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);

        if (res != 0) {
            ei_printf("ERR: run_classifier returned: %d\n", res);
            return 1;
        }
            
        ei_printf("Predictions (DSP: %ld us., Classification: %ld us., Anomaly: %ld us.): \n",
                    (int32_t)result.timing.dsp_us, (int32_t)result.timing.classification_us, (int32_t)result.timing.anomaly_us);

            // Print the prediction results (object detection)
        #if EI_CLASSIFIER_OBJECT_DETECTION == 1
            ei_printf("Object detection bounding boxes:\r\n");
            for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
                ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
                if (bb.value == 0) {
                    continue;
                }
                ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                        bb.label,
                        bb.value,
                        bb.x,
                        bb.y,
                        bb.width,
                        bb.height);
            }

            // Print the prediction results (classification)
        #else
            ei_printf("Predictions:\r\n");
            for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
                ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
                ei_printf("%.5f\r\n", result.classification[i].value);
            }
        #endif

            // Print anomaly result (if it exists)
        #if EI_CLASSIFIER_HAS_ANOMALY
            ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
        #endif

        #if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
            ei_printf("Visual anomalies:\r\n");
            for (uint32_t i = 0; i < result.visual_ad_count; i++) {
                ei_impulse_result_bounding_box_t bb = result.visual_ad_grid_cells[i];
                if (bb.value == 0) {
                    continue;
                }
                ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                        bb.label,
                        bb.value,
                        bb.x,
                        bb.y,
                        bb.width,
                        bb.height);
            }
            ei_printf("Visual anomaly values: Mean : %.3f Max : %.3f\r\n",
            result.visual_ad_result.mean_value, result.visual_ad_result.max_value);
        #endif

        ei_sleep(2000);
    }

    return  0;
}
