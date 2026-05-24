/* Minimal arm_rfft_init_q15 for 256-point Q15 RFFT only (Apache-2.0).
 * Stripped from CMSIS-DSP arm_rfft_init_q15.c — supports only fftLenReal=256.
 */
#include "arm_compiler_specific.h"

ARM_DSP_ATTRIBUTE arm_status arm_rfft_init_q15(
    arm_rfft_instance_q15 *S,
    uint32_t fftLenReal,
    uint32_t ifftFlagR,
    uint32_t bitReverseFlag)
{
    if (fftLenReal != 256U)
        return ARM_MATH_ARGUMENT_ERROR;

    S->fftLenReal        = 256U;
    S->pTwiddleAReal     = (q15_t *) realCoefAQ15;
    S->pTwiddleBReal     = (q15_t *) realCoefBQ15;
    S->ifftFlagR         = (uint8_t) ifftFlagR;
    S->bitReverseFlagR   = (uint8_t) bitReverseFlag;
    S->twidCoefRModifier = 32U;
    S->pCfft             = &arm_cfft_sR_q15_len128;

    return ARM_MATH_SUCCESS;
}
