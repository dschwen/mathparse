#include "contrib/sljit_src/sljitLir.h"

#include <cmath>

#include <stdio.h>
#include <stdlib.h>

typedef long SLJIT_CALL (*func3_t)(long a, long b, long c);

static long SLJIT_CALL
print_num(long a)
{
  auto & r = *reinterpret_cast<double *>(a);
  printf("in call %lx %f\n", a, r);
  r = std::sin(r);
  printf("in call %lx %f\n", a, r);
  return 0;
}

static int
func_call()
{
  void * code;
  unsigned long len;
  func3_t func;
  double (*sinptr)(double) = std::sin;
  double buf = 1.0;

  struct sljit_jump * out;
  struct sljit_jump * print_c;

  /* Create a SLJIT compiler */
  struct sljit_compiler * C = sljit_create_compiler(NULL);

  sljit_emit_enter(C, 0, 3, 1, 3, 0, 0, 0);

  /*  a & 1 --> R0 */
  sljit_emit_op2(C, SLJIT_AND, SLJIT_R0, 0, SLJIT_S0, 0, SLJIT_IMM, 1);
  /* R0 == 0 --> jump print_c */
  print_c = sljit_emit_cmp(C, SLJIT_EQUAL, SLJIT_R0, 0, SLJIT_IMM, 0);

  /* R0 = S1; print_num(R0) */
  sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)&buf);
  sljit_emit_ijump(C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(print_num));
  // sljit_emit_ijump(C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sinptr));

  /* jump out */
  out = sljit_emit_jump(C, SLJIT_JUMP);
  /* print_c: */
  sljit_set_label(print_c, sljit_emit_label(C));

  /* R0 = c; print_num(R0); */
  // sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_S2, 0);
  sljit_emit_op1(C, SLJIT_MOV, SLJIT_R0, 0, SLJIT_IMM, (sljit_sw)&buf);
  sljit_emit_ijump(C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(print_num));
  // sljit_emit_ijump(C, SLJIT_CALL1, SLJIT_IMM, SLJIT_FUNC_OFFSET(sinptr));

  /* out: */
  sljit_set_label(out, sljit_emit_label(C));
  sljit_emit_return(C, SLJIT_MOV, SLJIT_R0, 0);

  /* Generate machine code */
  code = sljit_generate_code(C);
  len = sljit_get_generated_code_size(C);

  printf("buf %f %lx\n", buf, &buf);

  /* Execute code */
  func = (func3_t)code;
  printf("func return %ld\n", func(0, 1, 2));

  printf("buf %f\n", buf);

  /* dump_code(code, len); */

  /* Clean up */
  sljit_free_compiler(C);
  sljit_free_code(code);
  return 0;
}

int
main()
{
  return func_call();
}
