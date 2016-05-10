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
#include "test_cs_nvm_create_delete.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "TESTAPPL"

const char test_nvm_key_test_create[] = "com.arm.nanostack.create";

void test1_nvm_write_progress_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_write_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    ret = platform_nvm_flush(test_common_nvm_flush_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test1_nvm_pre_write_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_write_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    test_progress_data.round = TEST_ROUND_CREATE_DELETE;
    ret = test_common_test_progress_write(test1_nvm_write_progress_callback, &test_progress_data);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test1_nvm_create1_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_create1_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_CREATE);

    ret = test_common_test_progress_write(test1_nvm_pre_write_callback, &test_progress_data);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test1_nvm_flush2_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_flush2_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_FLUSH);

    nvm_data_write.buffer_length = DATA_BUF_LEN;
    ret = platform_nvm_key_create(test1_nvm_create1_callback, test_nvm_key_test_create, nvm_data_write.buffer_length, 0, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}


void test1_nvm_delete_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_delete_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);

    ret = platform_nvm_flush(test1_nvm_flush2_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test1_nvm_write_resized_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_create_resize_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    // buffer length  has been updated to actual resized size
    TEST_EQ(nvm_data_write.buffer_length, DATA_BUF_LEN/2);

    // delete tests
    // test delete with bad arguments
    ret = platform_nvm_key_delete(NULL, test_nvm_key_test_create, TEST_CONTEXT_DELETE);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    ret = platform_nvm_key_delete(test1_nvm_delete_callback, NULL, TEST_CONTEXT_DELETE);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* delete the key */
    ret = platform_nvm_key_delete(test1_nvm_delete_callback, test_nvm_key_test_create, TEST_CONTEXT_DELETE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test1_nvm_create_resize_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_create_resize_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_RESIZE_CREATE);

    /* write to key that has been resized */
    nvm_data_write.buffer_length = DATA_BUF_LEN;
    ret = platform_nvm_write(test1_nvm_write_resized_callback, test_nvm_key_test_create, nvm_data_write.buffer, &nvm_data_write.buffer_length, TEST_CONTEXT_WRITE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test1_nvm_flush_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_flush_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_FLUSH);

    /* buffer length resized - OK */
    nvm_data_write.buffer_length = DATA_BUF_LEN/2;
    ret = platform_nvm_key_create(test1_nvm_create_resize_callback, test_nvm_key_test_create, nvm_data_write.buffer_length, 0, TEST_CONTEXT_RESIZE_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test1_nvm_write_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_write_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);
    // buffer length  has been updated to actual write size
    TEST_EQ(nvm_data_write.buffer_length, 0);

    ret = platform_nvm_flush(test1_nvm_flush_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}


void test1_nvm_create_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test1_nvm_create_callback status=%d args=%d", (int)status, (int)args);

    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_CREATE);

    /* write to key that has 0 length */
    ret = platform_nvm_write(test1_nvm_write_callback, test_nvm_key_test_create, nvm_data_write.buffer, &nvm_data_write.buffer_length, TEST_CONTEXT_WRITE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test_cs_nvm_create_delete(void)
{
    tr_info("test_cs_nvm_create_delete()");
    platform_nvm_status ret;

    test_common_nvm_read_data_init(&nvm_data_read);
    test_common_nvm_write_data_init(&nvm_data_write, NULL);

    /* create key with invalid params - no callback, NOK */
    ret = platform_nvm_key_create(NULL, test_nvm_key_test_create, nvm_data_write.buffer_length, 0, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* create key with invalid params - no key, NOK */
    ret = platform_nvm_key_create(test1_nvm_create_callback, NULL, nvm_data_write.buffer_length, 0, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_ERROR);

    /* create key with invalid params -  buffer length 1 - OK */
    ret = platform_nvm_key_create(test1_nvm_create_callback, test_nvm_key_test_create, 0, 0, TEST_CONTEXT_CREATE);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}
