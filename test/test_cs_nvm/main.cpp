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
#include "mbed-drivers/mbed.h"
#include "platform/arm_hal_nvm.h"
#include "platform/arm_hal_timer.h"
#include "nanostack-event-loop/eventOS_scheduler.h"
#include "nsdynmemLIB.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "TESTAPPL"

#include "test_cs_nvm_common.h"

static Serial &pc = get_stdio_serial();

#define NANOSTACK_HEAP_SIZE 30000
static uint8_t nanostack_heap[NANOSTACK_HEAP_SIZE+1];

nvm_data_t nvm_data_read;
nvm_data_t nvm_data_write;
test_progress_data_t test_progress_data = {0, 0, false, false, 0};

void test0_nvm_write_callback(platform_nvm_status status, void *args);

const char test_nvm_key_test_progress[] = "com.arm.nanostack.testdata";

void test0_nvm_read_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test0_nvm_read_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);

    if (args == TEST_CONTEXT_READ) {
        test_common_read_data_verify(&nvm_data_write, &nvm_data_read, DATA_BUF_LEN);
        ret = platform_nvm_flush(test_common_nvm_flush_callback, TEST_CONTEXT_FLUSH);
        TEST_EQ(ret, PLATFORM_NVM_OK);
    } else if (args == TEST_CONTEXT_INITIAL_READ) {
        test_common_nvm_progress_read(&nvm_data_read, &test_progress_data);
        if (test_progress_data.round < TEST_ROUND_LAST) {
            test_common_proceed(&test_progress_data);
        } else {
            // restart test from beginning
            test_common_progress_init(&test_progress_data);
            test_progress_data.round = TEST_ROUND_INIT;
            // write initial round to test data
            test_progress_data.round = TEST_ROUND_INIT;
            ret = test_common_test_progress_write(test0_nvm_write_callback, &test_progress_data);
        }
    } else {
        tr_error("test0 callback returned bad status %d", status);
        TEST_EQ(true, false);   //always fail
    }
}

void test0_nvm_write_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test0_nvm_write_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    ret = platform_nvm_read(test0_nvm_read_callback, test_nvm_key_test_progress, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test0_nvm_create_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test0_nvm_create_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_CREATE);

    // write initial round to test data
    test_progress_data.round = TEST_ROUND_INIT;
    ret = test_common_test_progress_write(test0_nvm_write_callback, &test_progress_data);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test0_nvm_initial_read_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test0_nvm_initial_read_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(args, TEST_CONTEXT_INITIAL_READ);

    if (status == PLATFORM_NVM_KEY_NOT_FOUND) {
        // first boot, create key and write test progress
        ret = platform_nvm_key_create(test0_nvm_create_callback, test_nvm_key_test_progress, nvm_data_write.buffer_length, 0, TEST_CONTEXT_CREATE);
        TEST_EQ(ret, PLATFORM_NVM_OK);
    } else if (status == PLATFORM_NVM_OK) {
        // direct call to callback
        test0_nvm_read_callback(status, TEST_CONTEXT_INITIAL_READ);
    } else {
        // error
        tr_error("Test0 initial buffer read returned error %d", status);
        TEST_EQ(0, 1);
    }
}

void test0_nvm_init_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;

    tr_debug("test0_nvm_init_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_INIT);

    // initialization with invalid callback fails
    ret = platform_nvm_init(NULL, TEST_CONTEXT_INIT);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    // initialization already done - failure
    ret = platform_nvm_init(test0_nvm_init_callback, TEST_CONTEXT_INIT);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    // try to read test_round from NVM
    ret = platform_nvm_read(test0_nvm_initial_read_callback, test_nvm_key_test_progress, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_INITIAL_READ);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

static void test0_nvm_init()
{
    test_common_progress_init(&test_progress_data);
    test_common_nvm_read_data_init(&nvm_data_read);
    test_common_nvm_write_data_init(&nvm_data_write, &test_progress_data);
    platform_nvm_status ret = platform_nvm_init(test0_nvm_init_callback, TEST_CONTEXT_INIT);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void app_start(int, char **)
{
    // set tracing baud rate
    pc.baud(115200);
    // initialize nanostack heap, timers and event scheduler and traces
    ns_dyn_mem_init(nanostack_heap, NANOSTACK_HEAP_SIZE, NULL, 0);
    platform_timer_enable();
    eventOS_scheduler_init();
    trace_init();
    tr_debug("Test application for Nanostack NVM adaptation");

    test0_nvm_init();
    return;
}
