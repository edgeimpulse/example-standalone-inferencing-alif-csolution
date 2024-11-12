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
#ifndef EI_OCR_H
#define EI_OCR_H

/* Include ----------------------------------------------------------------- */
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "tflite_learn_9_compiled.h"

class EiOcr {

    public:
        EiOcr(int32_t rows, int32_t cols);
        ~EiOcr();
        int32_t predict(float *input_features, int32_t row_start, int32_t row_end, int32_t col_start, int32_t col_end);
        uint8_t* get_output() { return output_text; }

    private:
        int32_t ROWS;
        int32_t COLS;
        uint8_t output_text[128];
        int32_t output_ix = 0;
        float *features;
        TfLiteTensor predict_input;
        TfLiteTensor predict_output;
};  




#endif // EI_OCR_H