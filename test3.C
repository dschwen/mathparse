#include "contrib/sljit_src/sljitLir.h"

#include <cmath>

#include <stdio.h>
#include <stdlib.h>

static void
dump_code(void * code, sljit_uw len)
{
  FILE * fp = fopen("/tmp/slj_dump", "wb");
  if (!fp)
    return;
  fwrite(code, len, 1, fp);
  fclose(fp);
#if defined(SLJIT_CONFIG_X86_64)
  system("gobjdump -b binary -m l1om -D /tmp/slj_dump");
#elif defined(SLJIT_CONFIG_X86_32)
  system("gobjdump -b binary -m i386 -D /tmp/slj_dump");
#endif
}

typedef long SLJIT_CALL (*func3_t)(long a, long b, long c);

static long SLJIT_CALL
compute(long a)
{
  auto & r = *reinterpret_cast<double *>(a);
  printf("in call %lx %f\n", a, r);
  r = std::sin(r);
  printf("in call %lx %f\n", a, r);
  return 0;
}

int
main()
{
  void * code;
  unsigned long len;
  func3_t func;

  double (*sinptr)(double) = std::sin;
  double abuf = 1.0;
  double bbuf = 1.3;

  /* Create a SLJIT compiler */
  struct sljit_compiler * C = sljit_create_compiler(NULL);

  sljit_emit_enter(C, 0, 3, 1, 3, 0, 0, 0);

  /* R0 = &buf; compute(R0) */
  // sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)&abuf);
  // sljit_emit_ijump(C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(compute));

  sljit_emit_fop2(C,
                  SLJIT_SUB_F64,
                  SLJIT_MEM,
                  (sljit_sw)&abuf,
                  SLJIT_MEM,
                  (sljit_sw)&abuf,
                  SLJIT_MEM,
                  (sljit_sw)&bbuf);

  sljit_emit_return(C, SLJIT_MOV, SLJIT_R0, 0);

  /* Generate machine code */
  code = sljit_generate_code(C);
  len = sljit_get_generated_code_size(C);

  printf("buf %f\n", abuf);

  /* Execute code */
  func = (func3_t)code;
  printf("func return %ld\n", func(0, 1, 2));

  printf("buf %f\n", abuf);

  dump_code(code, len);

  /* Clean up */
  sljit_free_compiler(C);
  sljit_free_code(code);
  return 0;
}
