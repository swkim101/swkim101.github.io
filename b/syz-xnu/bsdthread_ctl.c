#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define CARRY_FLAG_BIT 1
#define RETURN_SYSCALL_RESULT(result, flags) return (flags & CARRY_FLAG_BIT) ? -result : result;
__attribute__((noinline)) static int64_t __syscall(int64_t num, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5, int64_t arg6, int64_t arg7, int64_t arg8, int64_t arg9)
{
        int64_t result;
        int64_t flags;

        __asm__ __volatile__(
            "subq $0x20, %%rsp\n"
            "movq %6, %%r10\n"
            "movq %7, %%r8\n"
            "movq %8, %%r9\n"
            "movq %9, %%r11\n"
            "movq %%r11, 8(%%rsp)\n"       /* arg7 -> rsp+8  (within 0..31) */
            "movq %10, %%r11\n"
            "movq %%r11, 16(%%rsp)\n"      /* arg8 -> rsp+16 */
            "movq %11, %%r11\n"
            "movq %%r11, 24(%%rsp)\n"      /* arg9 -> rsp+24 (within 0..31) */
            "syscall\n"
            "movq $0, %%r12\n"
            "addq $0x20, %%rsp\n"
            "movq %%r11, %1\n"
            : "=a"(result), "=r"(flags)
            : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3),
              "r"(arg4), "r"(arg5), "r"(arg6), "m"(arg7), "m"(arg8), "m"(arg9)
            : "%r10", "%r8", "%r9", "%rcx", "%r11", "%r12", "memory"
        );

        RETURN_SYSCALL_RESULT(result, flags);
}

#define SYS_bsdthread_ctl (0x2000000 + 478)
int main() {
  int ret = 0;
  ret = __syscall(SYS_bsdthread_ctl, 0x100, 0xc00, 0xffffffffffffffff, 0xacd4, 0, 0, 0, 0, 0);
  if (ret < 0) {
    perror("bsdthread_ctl failed");
    return 1;
  }
  printf("bsdthread_ctl succeeded with ret=%d\n", ret);

  return 0;
}