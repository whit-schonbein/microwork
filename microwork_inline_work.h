/*****************************************************************************
 *
 * microwork_inline.c
 *
 * A collection of busy-wait work loops for modeling workload latency.
 *
 * Author: Whit Schonbein
 * Email: whit.schonbein@gmail.com
 * Date: 2016
 * 
 * Written under auspices of Dorian Arnold and the University of New Mexico
 *
 * Matrix multiplication work loop based on code by Samuel Guiterrez and 
 * included in versions of his MRNetBench benchmarking tool.
 *
 * Assembly code informed by Intel document "How to benchmark code 
 * execution times on Intel IA-32 and IA-64 instruction set architectures" 
 *
 * Currently defined work:
 *
 * WORK_NULL_C
 * WORK_MXM_C
 * WORK_ASM_NOP_C
 * WORK_ASM_MUL_C
 * WORK ASM_FADD_C
 * WORK_ASM_FMUL_C
 *
 * To use, the variable loop_num should be defined and set 
 * before the location the fragment is inserted. This variable 
 * could be the number of iterations to use during a trial 
 * for calibration, or the number of iterations expected to 
 * require some amount of time to execute.
 *
 */

#if !defined( __MICROWORK_WORK_H_ )
#define __MICROWORK_WORK_H_

/*******************************************************************
 * No work at all.
 *******************************************************************/
#define WORK_NULL_C  \
  {                  \
  };                    

/*******************************************************************
 * Original MXM work loop with builtin iteration
 *******************************************************************/
#define _NUM_ELEMS (32)
#define WORK_MXM_C                                    \
  int _i, _j, _k, _t;                                 \
                                                      \
  double _A[_NUM_ELEMS][_NUM_ELEMS];                  \
  double _B[_NUM_ELEMS][_NUM_ELEMS];                  \
  double _C[_NUM_ELEMS][_NUM_ELEMS];                  \
                                                      \
  for(_t = 0;_t<loop_num;++_t) {                      \
                                                      \
    for (int _i = 0; _i < _NUM_ELEMS; ++_i) {         \
      for (int _j = 0; _j < _NUM_ELEMS; ++_j) {       \
          _A[_i][_j] = 10.2343;                       \
          _B[_i][_j] = 2.23429;                       \
      }                                               \
    }                                                 \
                                                      \
    for (int _i = 0; _i < _NUM_ELEMS; ++_i)           \
      for (int _j = 0; _j < _NUM_ELEMS; ++_j)         \
        for (int _k = 0; _k < _NUM_ELEMS; ++_k)       \
          _C[_i][_j] += _A[_i][_k] * _B[_k][_j];      \
  }
/* END MXM WORK LOOP */

/*******************************************************************
 * Asm NOP work loop (WORK_ASM_NOP)
 *******************************************************************/
#define WORK_ASM_NOP_C                                                                    \
  uint64_t _sc, _tc, _cc, _sh, _sl, _ch, _cl;                                             \
  __asm__ __volatile__ (  "CPUID        ;"                                                \
                          "RDTSC        ;"                                                \
                          "mov %%rdx, %0;"                                                \
                          "mov %%rax, %1;"                                                \
                          : "=r" (_sh), "=r" (_sl) : : "%rax", "%rbx", "%rcx", "%rdx");   \
  _sc = ( ((uint64_t)_sh << 32) | _sl );                                                  \
  _tc = _sc + loop_num;                                                                   \
  do {                                                                                    \
    __asm__ __volatile__ (  "nop          ;"                                              \
                            "CPUID        ;"                                              \
                            "RDTSC        ;"                                              \
                            "mov %%rdx, %0;"                                              \
                            "mov %%rax, %1;"                                              \
                            "CPUID        ;"                                              \
                            : "=r" (_ch), "=r" (_cl) : : "%rax", "%rbx", "%rcx", "%rdx"); \
    _cc = ( ((uint64_t)_ch << 32) | _cl );                                                \
  } while (_cc <= _tc);
/* END ASM NOP WORK LOOP */

/*******************************************************************
 * Asm integer multiplication work loop (WORK_ASM_MUL)
 *******************************************************************/
#define WORK_ASM_MUL_C                                                                   \
  uint64_t _sc, _tc, _cc, _sh, _sl, _ch, _cl;                                             \
  int _aint = 1099;                                                                       \
  int _bint = 266;                                                                        \
  __asm__ __volatile__ (  "CPUID          ;"                                              \
                          "RDTSC          ;"                                              \
                          "mov %%rdx, %0  ;"                                              \
                          "mov %%rax, %1  ;"                                              \
                          : "=r" (_sh), "=r" (_sl) : : "%rax", "%rbx", "%rcx", "%rdx");   \
  _sc = ( ((uint64_t)_sh << 32) | _sl );                                                  \
  _tc = _sc + loop_num;                                                                   \
  do {                                                                                    \
    __asm__ __volatile__ ("movl %2, %%eax       ;"                                        \
                          "movl %3, %%ebx       ;"                                        \
                          "imull %%ebx, %%eax   ;"                                        \
                          "CPUID                ;"                                        \
                          "RDTSC                ;"                                        \
                          "mov %%rdx, %0        ;"                                        \
                          "mov %%rax, %1        ;"                                        \
                          : "=r" (_ch), "=r" (_cl) : "g" (_aint), "g" (_bint) : "%eax", "%ebx", "%rax", "%rbx" );\
    _cc = ( ((uint64_t)_ch << 32) | _cl );                                                \
  } while (_cc <= _tc);
/* END ASM MUL WORK LOOP */

/*******************************************************************
 * Asm floating point addition work loop (WORK_ASM_FADD)
 *******************************************************************/
#define WORK_ASM_FADD_C                                                                 \
  uint64_t _sc, _tc, _cc, _sh, _sl, _ch, _cl;                                             \
  float _bf = 21.1198213341;                                                              \
  float _o;\
  __asm__ __volatile__ (  "CPUID          ;"                                                \
                          "RDTSC          ;"                                                \
                          "mov %%rdx, %0  ;"                                                \
                          "mov %%rax, %1  ;"                                                \
                          : "=r" (_sh), "=r" (_sl) : : "%rax", "%rbx", "%rcx", "%rdx");     \
  _sc = ( ((uint64_t)_sh << 32) | _sl );                                                    \
  _tc = _sc + loop_num;                                                                     \
  do {                                                                                      \
    __asm__ __volatile__ ("flds %3;"                                          \
                          "faddp;"                                          \
                          "CPUID;"                                          \
                          "RDTSC;"                                          \
                          "mov %%rdx, %1;"                                          \
                          "mov %%rax, %2;"                                          \
                          : "=&t" (_o), "=r" (_ch), "=r" (_cl) : "m" (_bf), "0" (5.35667) : "st(1)", "%eax", "%ebx", "%rax", "%rdx" );\
    _cc = ( ((uint64_t)_ch << 32) | _cl );                                                  \
  } while (_cc <= _tc);
/* END ASM FLOAT ADD LOOP */

/*******************************************************************
 * Asm floating point multiplication work loop (WORK_ASM_FMUL)
 *
 * Note on how this works:
 *  1. The constraints put the value 5.35667 on the top of the floating point stack 
 *  (st(0)). The "0" in the 2nd input constraint means the constraint 
 *  uses the same register as the 0th constraint, i.e., the one labeled "=&t". This 
 *  will be st(0), the top of the floating point stack.
 *  2. flds %3 pushes the value at _bf onto the top of the floating point stack. That is, 
 *  curent value of st(0) -> st(1), and st(0) = value at _bf
 *  3. fmulp : multiply st(0) and st(1); result is in st(1) and st(0) is popped, so it ends up in st(0)
 *  then, the "=&t" constraint puts the value back into location _o
 * "st(1)" is included in clobber list because we mess with it but do not reference it 
 *  explicitly as an input or output
 *******************************************************************/
#define WORK_ASM_FMUL_C                                                                 \
  uint64_t _sc, _tc, _cc, _sh, _sl, _ch, _cl;                                             \
  float _bf = 21.1198213341;                                                              \
  float _o;\
  __asm__ __volatile__ (  "CPUID          ;"                                                \
                          "RDTSC          ;"                                                \
                          "mov %%rdx, %0  ;"                                                \
                          "mov %%rax, %1  ;"                                                \
                          : "=r" (_sh), "=r" (_sl) : : "%rax", "%rbx", "%rcx", "%rdx");     \
  _sc = ( ((uint64_t)_sh << 32) | _sl );                                                    \
  _tc = _sc + loop_num;                                                                     \
  do {                                                                                      \
    __asm__ __volatile__ ("flds %3;"                                          \
                          "fmulp;"                                          \
                          "CPUID;"                                          \
                          "RDTSC;"                                          \
                          "mov %%rdx, %1;"                                          \
                          "mov %%rax, %2;"                                          \
                          : "=&t" (_o), "=r" (_ch), "=r" (_cl) : "m" (_bf), "0" (5.35667) : "st(1)", "%eax", "%ebx", "%rax", "%rdx" );\
    _cc = ( ((uint64_t)_ch << 32) | _cl );                                                  \
  } while (_cc <= _tc);
/* END ASM FLOATING POINT MUL C */

#endif /* __MICROWORK_WORK_H_ */

