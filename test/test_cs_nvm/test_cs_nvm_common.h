/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef TEST_TEST_CS_NVM_TEST_CS_NVM_COMMON_H_
#define TEST_TEST_CS_NVM_TEST_CS_NVM_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif
#include "ns_types.h"

#define DATA_BUF_LEN 12+10
typedef struct {
    int buffer_length;
    /* 12 first bytes reserved for test progress: round, failed, total */
    uint8_t buffer[DATA_BUF_LEN];
} nvm_data_t;

/* data for test progress */
typedef struct {
    int total;  // total tests made
    int failed; // failed test count
    bool ok;
    bool round_ready;   // is test round ready
    int round;  // round indication, one of TEST_ROUND_XXX
} test_progress_data_t;

#define TEST_CONTEXT_INIT           (void*)0x01
#define TEST_CONTEXT_CREATE         (void*)0x02
#define TEST_CONTEXT_RESIZE_CREATE  (void*)0x03
#define TEST_CONTEXT_FLUSH          (void*)0x04
#define TEST_CONTEXT_WRITE          (void*)0x05
#define TEST_CONTEXT_READ           (void*)0x06
#define TEST_CONTEXT_INITIAL_READ   (void*)0x07
#define TEST_CONTEXT_DELETE         (void*)0x09



#define TEST_ROUND_INIT             1
#define TEST_ROUND_CREATE_DELETE    2
#define TEST_ROUND_WRITE            3
#define TEST_ROUND_READ             4

#define TEST_PRINT tr_error
#define TEST_EQ(A,B)\
    (test_progress_data.total++,((A!=B)?(test_progress_data.ok = true, test_progress_data.failed++, TEST_PRINT("%s:%d " #A "!=" #B " [FAIL]\r\n", __func__, __LINE__),0):1))

extern nvm_data_t nvm_data_read;
extern nvm_data_t nvm_data_write;
extern test_progress_data_t test_progress_data;

extern const char test_nvm_key_test_progress[];

void test_common_nvm_flush_callback(platform_nvm_status status, void *args);
void test_common_nvm_write_data_init(nvm_data_t *nvm_data, test_progress_data_t *test_data);
void test_common_nvm_read_data_init(nvm_data_t *nvm_data);
void test_common_progress_init(test_progress_data_t *progress);
void test_common_nvm_progress_read(nvm_data_t *nvm_data, test_progress_data_t *test_data);
void test_common_proceed(test_progress_data_t *test_progress);
platform_nvm_status test_common_test_progress_write(nvm_callback *callback, test_progress_data_t *test_data);
void test_common_read_data_verify(nvm_data_t *data_read, nvm_data_t *data_write, int data_len);

#ifdef __cplusplus
}
#endif

#endif /* TEST_TEST_CS_NVM_TEST_CS_NVM_COMMON_H_ */
