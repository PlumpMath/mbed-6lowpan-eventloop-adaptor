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

void test4_nvm_write_callback(platform_nvm_status status, void *args)
{
    platform_nvm_status ret;
    tr_debug("test4_nvm_write_callback status=%d args=%d", (int)status, (int)args);
    TEST_EQ(status, PLATFORM_NVM_OK);
    TEST_EQ(args, TEST_CONTEXT_WRITE);

    ret = platform_nvm_flush(test_common_nvm_flush_callback, TEST_CONTEXT_FLUSH);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}

void test_cs_nvm_restart(void)
{
    tr_info("test_cs_nvm_restart()");
    platform_nvm_status ret;

    test_common_progress_init(&test_progress_data);
    test_progress_data.round = TEST_ROUND_INIT;
    ret = test_common_test_progress_write(test4_nvm_write_callback, &test_progress_data);
    TEST_EQ(ret, PLATFORM_NVM_OK);
}
