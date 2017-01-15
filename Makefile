
GCC = gcc

CFLAGS = -Wall -g -O0 -w 
#LDFLAGS = -lrt -lm
LDFLAGS = -lm

#### Compiling with WORK_NULL

microwork_null.o: microwork_inline.c 
	$(GCC) $(CFLAGS) -D WORK_NULL -c $< -o $@ $(LDFLAGS)

mit_null.x: microwork_null.o microwork_inline_test.c
	$(GCC) $(CFLAGS) -D WORK_NULL $^ -o $@ $(LDFLAGS)

#### Compiling with WORK_MXM

microwork_mxm.o: microwork_inline.c 
	$(GCC) $(CFLAGS) -D WORK_MXM -c $< -o $@ $(LDFLAGS)

mit_mxm.x: microwork_mxm.o microwork_inline_test.c
	$(GCC) $(CFLAGS) -D WORK_MXM $^ -o $@ $(LDFLAGS)

#### Compiling with WORK_ASM_NOP

microwork_nop.o: microwork_inline.c 
	$(GCC) $(CFLAGS) -D WORK_ASM_NOP -c $< -o $@ $(LDFLAGS)

mit_nop.x: microwork_nop.o microwork_inline_test.c
	$(GCC) $(CFLAGS) -D WORK_ASM_NOP $^ -o $@ $(LDFLAGS)

#### Compiling with WORK_ASM_MUL

microwork_mul.o: microwork_inline.c 
	$(GCC) $(CFLAGS) -D WORK_ASM_MUL -c $< -o $@ $(LDFLAGS)

mit_mul.x: microwork_mul.o microwork_inline_test.c
	$(GCC) $(CFLAGS) -D WORK_ASM_MUL $^ -o $@ $(LDFLAGS)

#### Compiling with WORK_ASM_FADD

microwork_fadd.o: microwork_inline.c 
	$(GCC) $(CFLAGS) -D WORK_ASM_FADD -c $< -o $@ $(LDFLAGS)

mit_fadd.x: microwork_fadd.o microwork_inline_test.c
	$(GCC) $(CFLAGS) -D WORK_ASM_FADD $^ -o $@ $(LDFLAGS)

#### Compiling with WORK_ASM_FMUL

microwork_fmul.o: microwork_inline.c 
	$(GCC) $(CFLAGS) -D WORK_ASM_FMUL -c $< -o $@ $(LDFLAGS)

mit_fmul.x: microwork_fmul.o microwork_inline_test.c
	$(GCC) $(CFLAGS) -D WORK_ASM_FMUL $^ -o $@ $(LDFLAGS)


all: mit_null.x mit_mxm.x mit_nop.x mit_mul.x mit_fadd.x mit_fmul.x

clean:
	rm -f *.o 
	rm -f microwork_test.x
	rm -f foo*.x
	rm -f mit*.x
	rm -rf *.x.dSYM
