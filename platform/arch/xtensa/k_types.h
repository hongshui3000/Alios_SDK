#ifndef TYPES_H
#define TYPES_H

#define YUNOS_NO_WAIT                0u
#define YUNOS_WAIT_FOREVER           0xffffffffu     /* 32 bit value, if tick type is 64 bit, you need change it to 64 bit */
#define YUNOS_TASK_STACK_OVF_MAGIC   0xdeadbeafu     /* 32 bit or 64 bit stack overflow magic value */
#define YUNOS_INTRPT_STACK_OVF_MAGIC 0xdeaddeadu     /* 32 bit or 64 bit stack overflow magic value */
#define YUNOS_MM_FRAG_ALLOCATED      0xabcddcabu     /* 32 bit value, if 64 bit system, you need change it to 64 bit */
#define YUNOS_MM_FRAG_FREE           0xfefdecdbu     /* 32 bit value, if 64 bit system, you need change it to 64 bit */
#define YUNOS_INLINE                 static __inline /* inline keyword, it may change under different compilers */

typedef char     name_t;
typedef uint32_t sem_count_t;
typedef uint32_t cpu_stack_t;

typedef uint32_t hr_timer_t;
typedef uint32_t lr_timer_t;

typedef uint32_t tick_t;
typedef uint64_t idle_count_t;
typedef uint64_t sys_time_t;
typedef uint32_t mutex_nested_t;
typedef uint8_t  suspend_nested_t;

typedef uint64_t ctx_switch_t;

/* keil compiler not define ssize_t */
#ifdef __CC_ARM
typedef int32_t ssize_t;
#endif

#endif /* TYPES_H */

