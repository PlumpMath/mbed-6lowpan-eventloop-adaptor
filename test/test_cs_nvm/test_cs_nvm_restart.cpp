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
#include "test_cs_nvm_restart.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "TESTAPPL"

const char test_nvm_key_restart[] = "com.arm.nanostack.restart";

void test5_nvm_write_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test5_nvm_write_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);
    ret = platform_nvm_flush(test_common_nvm_flush_callback, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test5_nvm_read_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test5_nvm_read_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_READ);

    nvm_data_read.buffer[0] = nvm_data_read.buffer[0] + 1;
    ret = platform_nvm_write(test5_nvm_write_callback, test_nvm_key_restart, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_WRITE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test5_nvm_create_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test4_nvm_create_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_CREATE);

    // read restart key value
    ret = platform_nvm_read(test5_nvm_read_callback, test_nvm_key_restart, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test5_nvm_delete_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test5_nvm_write_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_DELETE);

    // create restart key and update its data to get flush callback called
    ret = platform_nvm_key_create(test5_nvm_create_callback, test_nvm_key_restart, nvm_data_write.buffer_length, 0, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test_cs_nvm_restart(void)
{
    tr_info("test_cs_nvm_restart()");
    platform_nvm_status ret;

    /* delete the progress key */
    ret = platform_nvm_key_delete(test5_nvm_delete_callback, test_nvm_key_test_progress, TEST_CONTEXT_DELETE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}
