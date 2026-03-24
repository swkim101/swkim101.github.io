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
            "movq %%r11, 8(%%rsp)\n"
            "movq %10, %%r11\n"
            "movq %%r11, 16(%%rsp)\n"
            "movq %11, %%r11\n"
            "movq %%r11, 24(%%rsp)\n"
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

#define MPO_CONTEXT_AS_GUARD               0x01    /* Add guard to the port */
#define MPO_QLIMIT                         0x02    /* Set qlimit for the port msg queue */
#define MPO_TEMPOWNER                      0x04    /* Set the tempowner bit of the port */
#define MPO_IMPORTANCE_RECEIVER            0x08    /* Mark the port as importance receiver */
#define MPO_INSERT_SEND_RIGHT              0x10    /* Insert a send right for the port */
#define MPO_STRICT                         0x20    /* Apply strict guarding for port */
#define MPO_DENAP_RECEIVER                 0x40    /* Mark the port as App de-nap receiver */
#define MPO_IMMOVABLE_RECEIVE              0x80    /* Mark the port as immovable; protected by the guard context */
#define MPO_FILTER_MSG                     0x100   /* Allow message filtering */
#define MPO_TG_BLOCK_TRACKING              0x200   /* Track blocking relationship for thread group during sync IPC */
#define MPO_SERVICE_PORT                   0x400   /* Create a service port with the given name; should be used only by launchd */
#define MPO_CONNECTION_PORT                0x800   /* Derive new peer connection port from a given service port */
#define MPO_REPLY_PORT                     0x1000  /* Designate port as a reply port. */
#define MPO_ENFORCE_REPLY_PORT_SEMANTICS   0x2000  /* When talking to this port, local port of mach msg needs to follow reply port semantics.*/
#define MPO_PROVISIONAL_REPLY_PORT         0x4000  /* Designate port as a provisional reply port. */
#define MPO_EXCEPTION_PORT                 0x8000  /* Used for hardened exceptions - immovable */

#define SP_CONTEXT (0x1803)
#define NEW_SP_CONTEXT (0x0318)
#define SERVICE_NAME "com.apple.testservice"
#define SERVICE_DOMAIN (1)

#define MACH_SERVICE_PORT_INFO_STRING_NAME_MAX_BUF_LEN 255

#define SYS_task_self 0x100001C
#define SYS_mach_port_construct 0x1000018

typedef int natural_t;
typedef int boolean_t;
typedef uint64_t user_addr_t;
typedef int mach_port_t;
typedef int mach_port_rights_t;
typedef int mach_port_seqno_t;
typedef int mach_port_mscount_t;
typedef int mach_port_msgcount_t;
typedef int mach_port_name_t;
typedef int mach_port_flavor_t;
typedef int mach_msg_type_number_t;

typedef struct mach_service_port_info {
	char                    mspi_string_name[MACH_SERVICE_PORT_INFO_STRING_NAME_MAX_BUF_LEN]; /* Service port's string name */
	uint8_t                 mspi_domain_type;          /* Service port domain */
} mach_service_port_info_data_t;

struct mach_port_options {
	uint32_t                flags;          /* Flags defining attributes for port */
	unsigned int      mpl;            /* Message queue limit for port */
	union {
		uint64_t                   reserved[2];           /* Reserved */
    int32_t           work_interval_port;    /* Work interval port */
		uint32_t                   service_port_info32;   /* Service port (MPO_SERVICE_PORT) */
		uint64_t                   service_port_info64;   /* Service port (MPO_SERVICE_PORT) */
		int32_t           service_port_name;
	};
};

int main() {
  int ret = 0;
  mach_port_t port = __syscall(SYS_task_self, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  printf("task self %d\n", port);

  mach_port_t new_port = 0;

  mach_service_port_info_data_t si;
  memcpy(si.mspi_string_name, SERVICE_NAME, sizeof(SERVICE_NAME) - 1);
  si.mspi_string_name[sizeof(SERVICE_NAME)] = '\0';
  si.mspi_domain_type = SERVICE_DOMAIN;

  struct mach_port_options opt;
  opt.flags = MPO_SERVICE_PORT | MPO_CONTEXT_AS_GUARD;
  opt.mpl = 0x7;
  opt.service_port_info64 = (uint64_t*)&si;

  ret = __syscall(SYS_mach_port_construct, port, &opt, 0x40, &new_port, 0, 0, 0, 0, 0);

  printf("mach_port_construct %d newport %d\n", ret, new_port);

  return 0;
}