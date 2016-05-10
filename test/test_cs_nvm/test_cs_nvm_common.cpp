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

#include <stdio.h>
#include <string.h>
#include "platform/arm_hal_nvm.h"
#include "test_cs_nvm_common.h"
#include "test_cs_nvm_create_delete.h"
#include "test_cs_nvm_write.h"
#include "test_cs_nvm_read.h"
#include "test_cs_nvm_deinit.h"
#include "test_cs_nvm_restart.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "TESTAPPL"

void test_common_nvm_finalize_callback(platform_nvm_status status, void *args)
{
    test_progress_data.round_ready = true;  // indicate to print results
    test_common_proceed(&test_progress_data);
}

void test_common_nvm_flush_callback(platform_nvm_status status, void *args)
{
    tr_debug("test_common_nvm_flush_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_FLUSH);

    platform_nvm_finalize(test_common_nvm_finalize_callback, 0);
}

void test_common_nvm_write_data_init(nvm_data_t *nvm_data, test_progress_data_t *test_data)
{
    int i;
    int *round = (int*)&nvm_data->buffer[0];
    int *total = (int*)&nvm_data->buffer[4];
    int *failed = (int*)&nvm_data->buffer[8];

    if (test_data) {
        *round = test_data->round;
        *total = test_data->total;
        *failed = test_data->failed;
    } else {
        *round = 0;
        *total = 0;
        *failed = 0;
    }

    // data starts at  nvm_data->buffer[12], fill the remaining with 0123...
    for (i=12; i< DATA_BUF_LEN; i++) {
        nvm_data->buffer[i] = i-12;
    }

    nvm_data->buffer_length = DATA_BUF_LEN;
}

void test_common_nvm_read_data_init(nvm_data_t *nvm_data)
{
    memset(nvm_data, 0, sizeof(nvm_data_t));
    nvm_data->buffer_length = DATA_BUF_LEN;
}

void test_common_nvm_progress_read(nvm_data_t *nvm_data, test_progress_data_t *test_data)
{
    // data starts at  nvm_data->buffer[12]
    test_data->round = (int)nvm_data->buffer[0];
    test_data->total = (int)nvm_data->buffer[4];
    test_data->failed = (int)nvm_data->buffer[8];

    tr_debug("test round = %d", test_data->round);
    tr_debug("tests total = %d", test_data->total);
    tr_debug("tests failed = %d", test_data->failed);
}


void test_common_read_data_verify(nvm_data_t *data_read, nvm_data_t *data_write, int data_len)
{
    int i;

    for (i=0; i< data_len; i++)
    {
        if (data_write->buffer[i] != data_read->buffer[i]) {
            tr_error("Read data doesn't not match write data in pos =%d", i);
            tr_error("WR: %s", trace_array(data_write->buffer, data_len));
            tr_error("RD: %s", trace_array(data_read->buffer, data_len));
            TEST_EQ(data_write->buffer[i], data_read->buffer[i]);
            break;
        }
    }
}

void test_common_proceed(test_progress_data_t *test_progress)
{
    if (test_progress_data.round_ready == true) {
        tr_info("*******");
        tr_info("Test result: %s", test_progress->failed == 0 ? "PASS": "FAIL");
        tr_info("failed: %d", test_progress->failed);
        tr_info("total: %d", test_progress->total);
        tr_info("test round: %d", test_progress->round);
        tr_info("Reset the device for the next test round!");

        tr_info(" round 1 = init tests");
        tr_info(" round 2 = create/delete tests");
        tr_info(" round 3 = write tests");
        tr_info(" round 4 = read tests");
        tr_info(" round 5 = deinit/restart");

        tr_debug("*******");
    } else{
        switch(test_progress->round) {
            case TEST_ROUND_INIT:
                test_cs_nvm_create_delete();
                break;
            case TEST_ROUND_CREATE_DELETE:
                test_cs_nvm_write();
                break;
            case TEST_ROUND_WRITE:
                test_cs_nvm_read();
                break;
            case TEST_ROUND_READ:
                test_cs_nvm_deinit();
                break;
            case TEST_ROUND_DEINIT:
                test_cs_nvm_restart();
                break;
            default:
                tr_error("Bad test round %d", test_progress->round);
                break;
        }
    }
}

void test_common_progress_init(test_progress_data_t *progress)
{
    progress->failed = 0;
    progress->ok = false;
    progress->total = 0;
    progress->round = 0;
    progress->round_ready = false;
}

platform_nvm_status test_common_test_progress_write(nvm_callback *callback, test_progress_data_t *test_data)
{
    test_common_nvm_write_data_init(&nvm_data_write, test_data);
    return platform_nvm_write(callback, test_nvm_key_test_progress, nvm_data_write.buffer, &nvm_data_write.buffer_length, TEST_CONTEXT_WRITE);
}

