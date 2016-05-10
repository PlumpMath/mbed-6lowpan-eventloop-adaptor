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

#include "test_cs_nvm_common.h"
#include "test_cs_nvm_deinit.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "TESTAPPL"

void test4_nvm_flush_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test4_nvm_flush_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    ret = platform_nvm_flush(test_common_nvm_flush_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_OK);

    test_common_progress_init(&test_progress_data);
}

void test4_nvm_write_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test4_nvm_write_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    ret = platform_nvm_flush(test_common_nvm_flush_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test4_nvm_init_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test4_nvm_init_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_INIT);

    test_progress_data.round = TEST_ROUND_DEINIT;
    ret = test_common_test_progress_write(test4_nvm_write_callback, &test_progress_data);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test4_nvm_dummy_callback(platform_nvm_status status, void *args)
{
    tr_debug("test4_nvm_dummy_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(true, false); // always fails when this callback is called
}

void test4_nvm_deinit_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test4_nvm_deinit_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_DEINIT);

    // try to deinitialize again
    ret = platform_nvm_finalize(test4_nvm_deinit_callback, TEST_CONTEXT_DEINIT);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    // try to create key
    ret = platform_nvm_key_create(test4_nvm_dummy_callback, test_nvm_key_test_progress, nvm_data_write.buffer_length, 0, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    // try to read from from NVM
    ret = platform_nvm_read(test4_nvm_dummy_callback, test_nvm_key_test_progress, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    // try to write to NVM
    ret = platform_nvm_write(test4_nvm_dummy_callback, test_nvm_key_test_progress, nvm_data_write.buffer, &nvm_data_write.buffer_length, TEST_CONTEXT_WRITE);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* try to delete the key */
    ret = platform_nvm_key_delete(test4_nvm_dummy_callback, test_nvm_key_test_progress, TEST_CONTEXT_DELETE);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    // try to flush NVM
    ret = platform_nvm_flush(test4_nvm_dummy_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    ret = platform_nvm_init(test4_nvm_init_callback, TEST_CONTEXT_INIT);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test_cs_nvm_deinit(void)
{
    tr_info("test_cs_nvm_deinit()");
    platform_nvm_status ret;

    ret = platform_nvm_finalize(test4_nvm_deinit_callback, TEST_CONTEXT_DEINIT);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}
