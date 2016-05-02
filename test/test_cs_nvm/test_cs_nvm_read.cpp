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
#include "test_cs_nvm_read.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "TESTAPPL"

const char test_nvm_key_test_read[] = "com.arm.nanostack.read";
const char test_nvm_key_not_avail[] = "com.arm.nanostack.keyNotAvailable";

void test3_nvm_write_progress_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test3_nvm_write_progress_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    ret = platform_nvm_flush(test_common_nvm_flush_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test3_nvm_read_byte_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test3_nvm_read_byte_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_READ);

    TEST_EQ(nvm_data_read.buffer_length, DATA_BUF_LEN/2);

    test_common_read_data_verify(&nvm_data_read, &nvm_data_write, 1);
    test_progress_data.round = TEST_ROUND_READ;
    ret = test_common_test_progress_write(test3_nvm_write_progress_callback, &test_progress_data);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test3_nvm_write_byte_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test3_nvm_write_byte_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    // Read the data from just write record
    test_common_nvm_read_data_init(&nvm_data_read);
    ret = platform_nvm_read(test3_nvm_read_byte_callback, test_nvm_key_test_read, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test3_nvm_read_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test3_nvm_read_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_READ);

    // Note! No data has been stored for the key yet

    // write 1 byte of data to created key
    nvm_data_write.buffer_length = 1;
    nvm_data_write.buffer[0] = 0xff;
    ret = platform_nvm_write(test3_nvm_write_byte_callback, test_nvm_key_test_read, nvm_data_write.buffer, &nvm_data_write.buffer_length, TEST_CONTEXT_WRITE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test3_nvm_create_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test3_nvm_create_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_CREATE);

    /* read with invalid params - no callback, NOK */
    ret = platform_nvm_read(NULL, test_nvm_key_test_read, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* read key with invalid params - no key, NOK */
    ret = platform_nvm_read(test3_nvm_read_callback, NULL, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* read key with invalid params -  buffer length NULL -NOK */
    ret = platform_nvm_read(test3_nvm_read_callback, test_nvm_key_test_read, nvm_data_read.buffer, NULL, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* create key with invalid params -  buffer is NULL */
    ret = platform_nvm_write(test3_nvm_read_callback, test_nvm_key_test_read, NULL, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* read data, there is nothing stored yet */
    ret = platform_nvm_write(test3_nvm_read_callback, test_nvm_key_test_read, nvm_data_read.buffer, &nvm_data_read.buffer_length, TEST_CONTEXT_READ);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test_cs_nvm_read(void)
{
    tr_info("test_cs_nvm_read()");
    platform_nvm_status ret;

    test_common_nvm_read_data_init(&nvm_data_read);
    nvm_data_write.buffer_length = DATA_BUF_LEN/2;

    ret = platform_nvm_key_create(test3_nvm_create_callback, test_nvm_key_test_read, nvm_data_write.buffer_length, 0, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_OK);

}
