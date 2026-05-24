/* Minimal self-contained header for 256-sample Q15 RFFT on micro:bit v2.
 *
 * Pre-defines include guards for all CMSIS-DSP headers that either can't be
 * found from the dsp/ subdirectory search path or are not needed for this
 * use case. Provides all required types, macros, table declarations, and
 * function declarations directly.
 *
 * Must be included before any other CMSIS-DSP header in every translation unit.
 */
#ifndef RFFT_Q15_API_H_
#define RFFT_Q15_API_H_

/* Pre-empt headers that the source files will try to include but that either
 * can't be resolved from their relative paths or pull in unwanted content. */
#define ARM_MATH_TYPES_H_
#define ARM_MATH_MEMORY_H_
#define ARM_CONST_STRUCTS_H
#define ARM_COMMON_TABLES_H
#define NONE_H_
#define ARM_MATH_UTILS_H_
#define TRANSFORM_FUNCTIONS_H_
#define BASIC_MATH_FUNCTIONS_H_
#define COMPLEX_MATH_FUNCTIONS_H_
#define FAST_MATH_FUNCTIONS_H_

/* GCC compiler macros (normally from cmsis_compiler.h / cmsis_gcc.h) */
#if defined(__GNUC__) && !defined(__STATIC_FORCEINLINE)
  #define __STATIC_FORCEINLINE  __attribute__((always_inline)) static inline
#endif
#if defined(__GNUC__) && !defined(__STATIC_INLINE)
  #define __STATIC_INLINE       static inline
#endif
#if defined(__GNUC__) && !defined(__ALIGNED)
  #define __ALIGNED(x)          __attribute__((aligned(x)))
#endif
#if defined(__GNUC__) && !defined(__WEAK)
  #define __WEAK                __attribute__((weak))
#endif

/* ARM SSAT instruction — saturate signed value to N bits (N = 1..32).
 * Cortex-M4 has the ssat instruction natively; use inline asm so the
 * compiler emits a single instruction instead of a branch sequence.
 * The "I" constraint requires ARG2 to be a compile-time constant,
 * which it is in every CMSIS-DSP call site we use. */
#if defined(__GNUC__) && !defined(__SSAT)
  #define __SSAT(ARG1, ARG2) \
  __extension__ \
  ({ \
    int32_t __RES, __ARG1 = (ARG1); \
    __asm__ ("ssat %0, %1, %2" : "=r" (__RES) : "I" (ARG2), "r" (__ARG1)); \
    __RES; \
  })
#endif

/* CMSIS-DSP attribute macros */
#ifndef ARM_DSP_ATTRIBUTE
  #define ARM_DSP_ATTRIBUTE
#endif
#ifndef ARM_DSP_TABLE_ATTRIBUTE
  #define ARM_DSP_TABLE_ATTRIBUTE
#endif

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

/* Basic CMSIS-DSP numeric types */
typedef int16_t  q15_t;
typedef int32_t  q31_t;
typedef float    float32_t;

/* Return status */
typedef enum {
    ARM_MATH_SUCCESS          =  0,
    ARM_MATH_ARGUMENT_ERROR   = -1,
    ARM_MATH_LENGTH_ERROR     = -2,
    ARM_MATH_SIZE_MISMATCH    = -3,
    ARM_MATH_NANINF           = -4,
    ARM_MATH_SINGULAR         = -5,
    ARM_MATH_TEST_FAILURE     = -6
} arm_status;

/* Legacy radix-4 CFFT instance (used internally by arm_cfft_radix4_q15.c) */
typedef struct {
    uint16_t        fftLen;
    uint8_t         ifftFlag;
    uint8_t         bitReverseFlag;
    const q15_t    *pTwiddle;
    const uint16_t *pBitRevTable;
    uint16_t        twidCoefModifier;
    uint16_t        bitRevFactor;
} arm_cfft_radix4_instance_q15;

/* CFFT instance (non-MVE path) */
typedef struct {
    uint16_t        fftLen;
    const q15_t    *pTwiddle;
    const uint16_t *pBitRevTable;
    uint16_t        bitRevLength;
} arm_cfft_instance_q15;

/* RFFT instance */
typedef struct {
    uint16_t                      fftLenReal;
    uint8_t                       ifftFlagR;
    uint8_t                       bitReverseFlagR;
    uint32_t                      twidCoefRModifier;
    q15_t                        *pTwiddleAReal;
    q15_t                        *pTwiddleBReal;
    const arm_cfft_instance_q15  *pCfft;
} arm_rfft_instance_q15;

/* Table length macro used by arm_const_structs_rfft256.c */
#define ARMBITREVINDEXTABLE_FIXED_128_TABLE_LENGTH  ((uint16_t)112)

/* External table declarations (defined in arm_common_tables_rfft256.c) */
extern const q15_t    twiddleCoef_128_q15[192];
extern const uint16_t armBitRevIndexTable_fixed_128[112];
extern const q15_t    realCoefAQ15[8192];
extern const q15_t    realCoefBQ15[8192];

/* CFFT instance declarations (arm_cfft_sR_q15_len128 defined in
 * arm_const_structs_rfft256.c; others referenced by arm_rfft_init_q15.c
 * for FFT sizes we don't use but must be declared for compilation) */
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len16;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len32;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len64;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len128;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len256;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len512;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len1024;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len2048;
extern const arm_cfft_instance_q15 arm_cfft_sR_q15_len4096;

/* Function declarations */
arm_status arm_rfft_init_q15(arm_rfft_instance_q15 *S, uint32_t fftLenReal,
                             uint32_t ifftFlagR, uint32_t bitReverseFlag);
void arm_rfft_q15(const arm_rfft_instance_q15 *S, q15_t *pSrc, q15_t *pDst);
void arm_cfft_q15(const arm_cfft_instance_q15 *S, q15_t *p1,
                  uint8_t ifftFlag, uint8_t bitReverseFlag);
void arm_bitreversal_16(uint16_t *pSrc, const uint16_t bitRevLen,
                        const uint16_t *pBitRevTable);
void arm_shift_q15(const q15_t *pSrc, int8_t shiftBits, q15_t *pDst,
                   uint32_t blockSize);

#ifdef __cplusplus
}
#endif

#endif /* RFFT_Q15_API_H_ */
