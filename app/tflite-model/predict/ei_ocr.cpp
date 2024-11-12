/*
 * Copyright (c) 2024 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Include ----------------------------------------------------------------- */
#include "ei_ocr.h"
// #include "tflite_learn_9_compiled.h"
#include "bitmap_helper.h"


#include "edge-impulse-sdk/classifier/ei_aligned_malloc.h"

#define IMAGE_COLS 160

static const char dict[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', ' ', ' ' };


EiOcr::EiOcr(int32_t rows, int32_t cols) : ROWS(rows), COLS(cols)
{
    TfLiteStatus status;

    status = model_predict_init(ei_aligned_calloc);
    if (status != kTfLiteOk) {
        ei_printf("model_predict_init failed (%d)\n", status);
        return;
    }

    status = model_predict_input(0, &predict_input);
    if (status != kTfLiteOk) {
        ei_printf("model_predict_input failed (%d)\n", status);
        return;
    }

    status = model_predict_output(0, &predict_output);
    if (status != kTfLiteOk) {
        ei_printf("model_predict_output failed (%d)\n", status);
        return;
    }

    features = (float*)ei_malloc(COLS * ROWS * 3 * sizeof(float));
    if (features == NULL) {
        ei_printf("Failed to allocate memory for features\n");
        return;
    }
}

EiOcr::~EiOcr() 
{
    TfLiteStatus status;

    status = model_predict_reset(ei_aligned_free);
    if (status != kTfLiteOk) {
        ei_printf("model_predict_reset failed (%d)\n", status);
        // return;
    }

    ei_free(features);
}

int32_t EiOcr::predict(float *input_features, int32_t row_start, int32_t row_end, int32_t col_start, int32_t col_end)
{
    TfLiteStatus status;
    memset(output_text, 0, 128);
    output_ix = 0;
    
    int cutout_rows = row_end - row_start;
    int cutout_cols = col_end - col_start;
    float *cutout = (float*)ei_malloc(cutout_rows * cutout_cols * sizeof(float));

    if(cutout == NULL) {
        ei_printf("Failed to allocate memory for cutout\n");
        return 0;
    }

    for (int row = 0; row < cutout_rows; row++) {
        for (int col = 0; col < cutout_cols; col++) {
            cutout[(row * cutout_cols) + col] = input_features[((row + row_start) * IMAGE_COLS) + (col + col_start)];
        }
    }

    float *resized_cutout = (float*)ei_malloc(COLS * ROWS * sizeof(float));

    if(resized_cutout == NULL) {
        ei_printf("Failed to allocate memory for resized cutout\n");
        return 0;
    }


    resize_image(cutout, resized_cutout, cutout_cols, cutout_rows, COLS, ROWS);

    for (size_t ix = 0; ix < COLS * ROWS; ix++) {
        float pixel = resized_cutout[ix];
        float r = (float)((int)pixel >> 16 & 0xff) / 255.0f;
        float g = (float)((int)pixel >> 8 & 0xff) / 255.0f;
        float b = (float)((int)pixel & 0xff) / 255.0f;
        features[(ix * 3) + 0] = b;
        features[(ix * 3) + 1] = g;
        features[(ix * 3) + 2] = r;
    }


    float py_zero_point = predict_input.params.zero_point;
    float py_scale = predict_input.params.scale;

    for (size_t ix = 0; ix < COLS * ROWS * 3; ix++){
        float v = features[ix];
        v = (v - 0.5) / 0.5;
        int vq = (v / py_scale) + py_zero_point;
        if (vq < -128)
        {
            vq = -128;
        }
        if (vq > 127)
        {
            vq = 127;
        }
        predict_input.data.int8[ix] = vq;
    }

    ei_free(cutout);

    // invoke model
    status = model_predict_invoke();
    if (status != kTfLiteOk) {
        ei_printf("model_predict_invoke failed (%d)\n", status);
        return 0;
    }

    float highest_confidence = 0.0f;
    float lowest_confidence = 1.0f;

    for (size_t row = 0; row < 20; row++) {
        int hi_val = -1000;
        int hi_ix = -1;

        // printf("row=%d: ", (int)row);
        for (int col = 0; col < 97; col++) {
            int v = predict_output.data.int8[(row * 97) + col];
            if (v > hi_val) {
                hi_val = v;
                hi_ix = col;
            }
        }
        // printf("hi_val=%f, ix=%d\n", hi_val, hi_ix);
        if (hi_ix != 0) {
            // ei_printf("%c (%f)\n", dict[hi_ix - 1], hi_val);
            output_text[output_ix++] = dict[hi_ix - 1];
        }

        if (hi_val > highest_confidence) {
            highest_confidence = hi_val;
        }
        if (hi_val < lowest_confidence) {
            lowest_confidence = hi_val;
        }
    }

    ei_free(resized_cutout);

    return output_ix;
}