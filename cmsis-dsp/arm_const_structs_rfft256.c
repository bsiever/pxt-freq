/* Minimal CMSIS-DSP const structs for 256-sample Q15 RFFT (Apache-2.0).
 * Contains only arm_cfft_sR_q15_len128, which is the CFFT instance used
 * internally by arm_rfft_init_q15() for a 256-point real FFT.
 */
#include "arm_compiler_specific.h"

const arm_cfft_instance_q15 arm_cfft_sR_q15_len128 ARM_DSP_TABLE_ATTRIBUTE = {
  128, twiddleCoef_128_q15, armBitRevIndexTable_fixed_128, ARMBITREVINDEXTABLE_FIXED_128_TABLE_LENGTH
};
