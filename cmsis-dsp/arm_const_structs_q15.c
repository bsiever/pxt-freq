/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_const_structs_q15.c
 * Description:  Q15 constant structs for 2048-point real FFT (trimmed)
 *
 * $Date:        23 April 2021
 * $Revision:    V1.9.0
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arm_compiler_specific.h"
#include "arm_common_tables.h"
#include "arm_const_structs.h"

#if (!defined(ARM_MATH_MVEI)  || defined(ARM_MATH_AUTOVECTORIZE)) && !defined(ARM_MATH_NEON)

const arm_cfft_instance_q15 arm_cfft_sR_q15_len1024 ARM_DSP_TABLE_ATTRIBUTE = {
  1024, twiddleCoef_1024_q15, armBitRevIndexTable_fixed_1024, ARMBITREVINDEXTABLE_FIXED_1024_TABLE_LENGTH
};

const arm_cfft_instance_q15 arm_cfft_sR_q15_len2048 ARM_DSP_TABLE_ATTRIBUTE = {
  2048, twiddleCoef_2048_q15, armBitRevIndexTable_fixed_2048, ARMBITREVINDEXTABLE_FIXED_2048_TABLE_LENGTH
};

#endif /* !defined(ARM_MATH_MVEI) */
