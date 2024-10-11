/* Generated by Edge Impulse
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
// Generated on: 29.05.2024 11:33:13

#ifndef model_detect_GEN_H
#define model_detect_GEN_H

#include "edge-impulse-sdk/tensorflow/lite/c/common.h"

// Sets up the model with init and prepare steps.
TfLiteStatus model_detect_init( void*(*alloc_fnc)(size_t,size_t) );
// Returns the input tensor with the given index.
TfLiteStatus model_detect_input(int index, TfLiteTensor* tensor);
// Returns the output tensor with the given index.
TfLiteStatus model_detect_output(int index, TfLiteTensor* tensor);
// Runs inference for the model.
TfLiteStatus model_detect_invoke();
//Frees memory allocated
TfLiteStatus model_detect_reset( void (*free)(void* ptr) );


// Returns the number of input tensors.
inline size_t model_detect_inputs() {
  return 1;
}
// Returns the number of output tensors.
inline size_t model_detect_outputs() {
  return 1;
}

#endif
