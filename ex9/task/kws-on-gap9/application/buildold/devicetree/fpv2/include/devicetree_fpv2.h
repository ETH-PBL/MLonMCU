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

#pragma once
#include <stdint.h>
#include <bsp/flash_partition_v2.h>

// Devices FPV2 accessors ******************************************************

/**
 * @brief Retrieve a reference to the FPV2 memory reference.
 *
 * This function returns a pointer to the global variable `fpv2_memory_refs`.
 * This global variable holds the relationship between devices from the device
 * tree and the storage device references.
 *
 * @return A pointer to `fpv2_memory_refs`.
 */
pi_fpv2_memory_ref_t* dt_fpv2_memory_ref_get(void);


/**
 * @brief Retrieve the number of storage devices in the devicetree
 *
 * This function returns a the number of item in `fpv2_memory_refs` table.
 * This global variable holds the relationship between devices from the device
 * tree and the storage device references.
 *
 * @return Storage devices number
 */
uint32_t dt_fpv2_memory_ref_size_get(void);

