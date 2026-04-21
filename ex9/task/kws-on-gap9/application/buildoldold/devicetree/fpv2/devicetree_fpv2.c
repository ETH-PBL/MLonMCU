/*
 * Copyright (C) 2026 GreenWaves Technologies
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dt.h>
#include <bsp/flash_partition_v2.h>

// Flash partition v2 memories *************************************************

pi_fpv2_memory_ref_t fpv2_memory_refs[2] = {
    { .ref = { PI_FPV2_DEV_TYPE_OCTOSPI, 0, 0}, .memory = PI_FLASH_MX25U51245G},
    { .ref = { PI_FPV2_DEV_TYPE_MRAM,    0, 0}, .memory = PI_FLASH_MRAM},
};

// Devices FPV2 accessors ******************************************************

pi_fpv2_memory_ref_t* dt_fpv2_memory_ref_get(void)
{
    return &fpv2_memory_refs[0];
}

uint32_t dt_fpv2_memory_ref_size_get(void)
{
    return sizeof(fpv2_memory_refs)/sizeof(fpv2_memory_refs[0]);
}
