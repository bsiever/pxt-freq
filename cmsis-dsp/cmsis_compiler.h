/* CMSIS compiler shim for CMSIS-DSP on micro:bit / CODAL (arm-none-eabi-gcc).
 *
 * Two distinct compilation contexts exist:
 *
 *  A) freq.cpp / pointers.cpp and other PXT C++ files: pxt.h is included first,
 *     which pulls in CODAL's cmsis_compiler.h → cmsis_gcc.h.  That header defines
 *     all DSP intrinsics (__QADD16, __SMUAD, …) as inline functions, and defines
 *     __SSAT/__USAT as macros using __extension__.  The one thing it does NOT
 *     define is __STATIC_FORCEINLINE (only __STATIC_INLINE).
 *
 *  B) The standalone CMSIS-DSP .c files (arm_cfft_radix4_q15.c etc.): they do not
 *     include pxt.h, so CODAL's cmsis_gcc.h has not run yet.  We must provide the
 *     DSP intrinsics ourselves.
 *
 * We detect which context we are in via __CMSIS_GCC_H (the include guard of
 * CODAL's cmsis_gcc.h).  If it is defined, CODAL already handled everything and we
 * only patch in __STATIC_FORCEINLINE.  If not, we provide the full set.
 */
#ifndef CMSIS_COMPILER_H_
#define CMSIS_COMPILER_H_

#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)

/* Basic compiler-portability macros — safe to define unconditionally because
 * CODAL's cmsis_gcc.h uses #ifndef guards for the ones it also defines. */
#ifndef __ASM
  #define __ASM                 __asm
#endif
#ifndef __INLINE
  #define __INLINE              inline
#endif
#ifndef __STATIC_INLINE
  #define __STATIC_INLINE       static inline
#endif
#ifndef __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE  static inline __attribute__((always_inline))
#endif
#ifndef __NO_RETURN
  #define __NO_RETURN           __attribute__((noreturn))
#endif
#ifndef __USED
  #define __USED                __attribute__((used))
#endif
#ifndef __WEAK
  #define __WEAK                __attribute__((weak))
#endif
#ifndef __PACKED
  #define __PACKED              __attribute__((packed))
#endif
#ifndef __PACKED_STRUCT
  #define __PACKED_STRUCT       struct __attribute__((packed))
#endif
#ifndef __ALIGNED
  #define __ALIGNED(x)          __attribute__((aligned(x)))
#endif
#ifndef __RESTRICT
  #define __RESTRICT            __restrict
#endif

/* -----------------------------------------------------------------------
 * DSP / SIMD intrinsics.
 * Only needed when CODAL's cmsis_gcc.h has NOT already been included.
 * In that case (__CMSIS_GCC_H undefined) we provide inline-asm versions
 * for the ARM Cortex-M4F hardware instructions.
 * ----------------------------------------------------------------------- */
#ifndef __CMSIS_GCC_H

__STATIC_FORCEINLINE int32_t __SSAT(int32_t val, uint32_t sat)
{
    int32_t result;
    __asm volatile ("ssat %0, %2, %1" : "=r"(result) : "r"(val), "I"(sat) : "cc");
    return result;
}

__STATIC_FORCEINLINE uint32_t __USAT(int32_t val, uint32_t sat)
{
    uint32_t result;
    __asm volatile ("usat %0, %2, %1" : "=r"(result) : "r"(val), "I"(sat) : "cc");
    return result;
}

#ifndef __CLZ
  #define __CLZ(x)  __builtin_clz(x)
#endif

__STATIC_FORCEINLINE uint32_t __ROR(uint32_t op1, uint32_t op2)
{
    return (op1 >> op2) | (op1 << (32U - op2));
}

#ifndef __PKHBT
  #define __PKHBT(ARG1, ARG2, ARG3) \
    ( (((uint32_t)(ARG1))            & 0x0000FFFFUL) | \
      (((uint32_t)(ARG2) << (ARG3))  & 0xFFFF0000UL) )
#endif
#ifndef __PKHTB
  #define __PKHTB(ARG1, ARG2, ARG3) \
    ( (((uint32_t)(ARG1))            & 0xFFFF0000UL) | \
      (((uint32_t)(ARG2) >> (ARG3))  & 0x0000FFFFUL) )
#endif

__STATIC_FORCEINLINE uint32_t __QADD16(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("qadd16 %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __QSUB16(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("qsub16 %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SHADD16(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("shadd16 %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SHSUB16(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("shsub16 %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __QADD8(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("qadd8 %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __QSUB8(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("qsub8 %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __QASX(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("qasx %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __QSAX(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("qsax %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SHASX(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("shasx %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SHSAX(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("shsax %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SMUAD(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("smuad %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SMUADX(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("smuadx %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SMUSD(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("smusd %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SMUSDX(uint32_t op1, uint32_t op2)
{ uint32_t r; __asm volatile ("smusdx %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE uint32_t __SMLAD(uint32_t op1, uint32_t op2, uint32_t op3)
{ uint32_t r; __asm volatile ("smlad %0,%1,%2,%3":"=r"(r):"r"(op1),"r"(op2),"r"(op3)); return r; }

__STATIC_FORCEINLINE uint32_t __SMLADX(uint32_t op1, uint32_t op2, uint32_t op3)
{ uint32_t r; __asm volatile ("smladx %0,%1,%2,%3":"=r"(r):"r"(op1),"r"(op2),"r"(op3)); return r; }

__STATIC_FORCEINLINE uint32_t __SMLSDX(uint32_t op1, uint32_t op2, uint32_t op3)
{ uint32_t r; __asm volatile ("smlsdx %0,%1,%2,%3":"=r"(r):"r"(op1),"r"(op2),"r"(op3)); return r; }

__STATIC_FORCEINLINE uint64_t __SMLALD(uint32_t op1, uint32_t op2, uint64_t acc)
{
    union { uint64_t u64; struct { uint32_t lo; uint32_t hi; } s; } r = { .u64 = acc };
    __asm volatile ("smlald %0,%1,%2,%3" : "+r"(r.s.lo),"+r"(r.s.hi) : "r"(op1),"r"(op2));
    return r.u64;
}

__STATIC_FORCEINLINE uint64_t __SMLALDX(uint32_t op1, uint32_t op2, uint64_t acc)
{
    union { uint64_t u64; struct { uint32_t lo; uint32_t hi; } s; } r = { .u64 = acc };
    __asm volatile ("smlaldx %0,%1,%2,%3" : "+r"(r.s.lo),"+r"(r.s.hi) : "r"(op1),"r"(op2));
    return r.u64;
}

__STATIC_FORCEINLINE uint32_t __SXTB16(uint32_t op1)
{ uint32_t r; __asm volatile ("sxtb16 %0,%1":"=r"(r):"r"(op1)); return r; }

__STATIC_FORCEINLINE int32_t __SMMLA(int32_t op1, int32_t op2, int32_t op3)
{ int32_t r; __asm volatile ("smmla %0,%1,%2,%3":"=r"(r):"r"(op1),"r"(op2),"r"(op3)); return r; }

__STATIC_FORCEINLINE int32_t __QADD(int32_t op1, int32_t op2)
{ int32_t r; __asm volatile ("qadd %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

__STATIC_FORCEINLINE int32_t __QSUB(int32_t op1, int32_t op2)
{ int32_t r; __asm volatile ("qsub %0,%1,%2":"=r"(r):"r"(op1),"r"(op2)); return r; }

#endif /* !__CMSIS_GCC_H */

#endif /* __GNUC__ || __clang__ */
#endif /* CMSIS_COMPILER_H_ */
