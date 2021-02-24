#include <stdio.h>

#include <rtems.h>
#include <rtems/test-info.h>

#include "tmacros.h"


#define CPU_COUNT 2

#define SCHEDULER_A rtems_build_name('S','C','H','A')
#define SCHEDULER_B rtems_build_name('S','C','H','B')
#define SCHEDULER_C rtems_build_name('S','C','H','C')
#define SCHEDULER_D rtems_build_name('S','C','H','D')


const char rtems_test_name[] = "SMP QEMU";
static volatile bool runnable;

struct task_info {
    rtems_task (*entry)(rtems_task_argument);
    rtems_id thread;
    rtems_id scheduler_name;
    rtems_task_priority priority;
    uint32_t cpu_affinity;
};

#ifndef BIT
#define BIT(n) (0x1 << (n))
#endif

#define _TASK_INIT(_n, _prio, _sched_name, _affinity) \
    [_n] = {\
        .entry = test_task__##_n, \
        .thread = 0, \
        .scheduler_name = _sched_name, \
        .priority = _prio, \
        .cpu_affinity = _affinity \
    }


#define TASK_DEFINE(_n) \
    static rtems_task test_task__##_n(rtems_task_argument arg) \
    { \
        for ( ; ; ) { \
            printf("Core_%u: T" #_n "\n", rtems_scheduler_get_processor()); \
            rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(50)); \
        } \
    }

TASK_DEFINE(0);
TASK_DEFINE(1);
TASK_DEFINE(2);
TASK_DEFINE(3);
TASK_DEFINE(4);
TASK_DEFINE(5);
TASK_DEFINE(6);
TASK_DEFINE(7);
TASK_DEFINE(8);

static struct task_info _task_table[] = {
    _TASK_INIT(0, 100, 0, 0),
    _TASK_INIT(1, 100, 0, 0),
    _TASK_INIT(2, 100, 0, 0),
    _TASK_INIT(3, 100, 0, 0),
    _TASK_INIT(4, 100, 0, 0),
    _TASK_INIT(5, 100, 0, 0),
    _TASK_INIT(6, 100, 0, 0),
    _TASK_INIT(7, 100, 0, 0),
    _TASK_INIT(8, 100, 0, 0),

};

static rtems_task test_task_1(rtems_task_argument arg)
{
    runnable = true;
    for ( ; ; ) {
        printf("Core_%u: Test1-Task\n", rtems_scheduler_get_processor());
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(1234));
    }
}

static rtems_task test_task_2(rtems_task_argument arg)
{
    for ( ; ; ) {
        printf("Core_%u: T2\n", rtems_scheduler_get_processor());
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(978));
    }
}

static rtems_task test_task_3(rtems_task_argument arg)
{
    for ( ; ; ) {
        printf("Core_%u: T3\n", rtems_scheduler_get_processor());
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(1000));
    }
}

static rtems_task test_task_4(rtems_task_argument arg)
{
    for ( ; ; ) {
        printf("Core_%u: T4\n", rtems_scheduler_get_processor());
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(1000));
    }
}

static rtems_status_code thread_set_scheduler(rtems_id thread, 
    rtems_name sched_name, rtems_task_priority priority)
{
    rtems_id scheduler;
    rtems_status_code sc;
    sc = rtems_scheduler_ident(sched_name, &scheduler);
    _Assert(sc == RTEMS_SUCCESSFUL);
    return rtems_task_set_scheduler(thread, scheduler, priority);
}


static void create_tasks(rtems_id *thread_pool, size_t n)
{
    for (int i = 0; i < n; i++) {
        sc = rtems_task_create(rtems_build_name('t','s','t','0'+i), 100, 4096, 
            RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &thread_pool[i]);
        rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    }
}

static void start_tasks(rtems_id *thread_pool, size_t n)
{
    for (int i = 0; i < n; i++) {
        sc = (rtems_task_start(tsk1, test_task_1, 0));
        rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    }
}

rtems_task Init(rtems_task_argument arg)
{
    rtems_status_code sc;
    rtems_id tsk1;
    rtems_id tsk2;
    rtems_id tsk3;
    rtems_id tsk4;
    cpu_set_t cpu;

    rtems_test_assert(rtems_scheduler_get_processor_maximum() == CPU_COUNT);

    sc = rtems_task_create(rtems_build_name('t','s','t','1'), 100, 4096, 
        RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tsk1);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = rtems_task_create(rtems_build_name('t','s','t','2'), 110, 4096, 
        RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tsk2);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = rtems_task_create(rtems_build_name('t','s','t','3'), 120, 4096, 
        RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tsk3);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = rtems_task_create(rtems_build_name('t','s','t','4'), 120, 4096, 
        RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tsk4);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    // thread_set_scheduler(rtems_task_self(), SCHEDULER_A, 1);
    // thread_set_scheduler(tsk1, SCHEDULER_B, 200);
    // thread_set_scheduler(tsk2, SCHEDULER_C, 190);
 
    CPU_ZERO(&cpu);
    CPU_SET(1, &cpu);
    //rtems_task_set_affinity(tsk1, sizeof(cpu), &cpu);
    rtems_task_set_affinity(tsk2, sizeof(cpu), &cpu);
    rtems_task_set_affinity(tsk3, sizeof(cpu), &cpu);
    rtems_task_set_affinity(tsk4, sizeof(cpu), &cpu);
    
 #if 0    
    CPU_ZERO(&cpu);
    CPU_SET(0, &cpu);
    rtems_task_set_affinity(tsk1, sizeof(cpu), &cpu);

    CPU_ZERO(&cpu);
    CPU_SET(1, &cpu);
    rtems_task_set_affinity(tsk2, sizeof(cpu), &cpu);
#endif
   
    sc = (rtems_task_start(tsk1, test_task_1, 0));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = (rtems_task_start(tsk2, test_task_2, 0));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = (rtems_task_start(tsk3, test_task_3, 0));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = (rtems_task_start(tsk4, test_task_4, 0));
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    while (!runnable);
     
    for ( ; ; ) {
        printf("Core_%u: Init-Task\n", rtems_scheduler_get_processor());
        rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(500));
    }
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_SIMPLE_CONSOLE_DRIVER
#define CONFIGURE_MAXIMUM_PROCESSORS CPU_COUNT
#define CONFIGURE_SCHEDULER_EDF_SMP
//#define CONFIGURE_SCHEDULER_PRIORITY_SMP


#include <rtems/scheduler.h>

#if 0
RTEMS_SCHEDULER_PRIORITY_SMP(a, 255);
RTEMS_SCHEDULER_PRIORITY_SMP(b, 255);

#define CONFIGURE_SCHEDULER_TABLE_ENTRIES \
  RTEMS_SCHEDULER_TABLE_PRIORITY_SMP(a, SCHEDULER_A), \
  RTEMS_SCHEDULER_TABLE_PRIORITY_SMP(b, SCHEDULER_B)

#define CONFIGURE_SCHEDULER_ASSIGNMENTS \
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
  RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL)
  
//#else
RTEMS_SCHEDULER_EDF_SMP(a);
RTEMS_SCHEDULER_EDF_SMP(b);
RTEMS_SCHEDULER_EDF_SMP(c);
RTEMS_SCHEDULER_EDF_SMP(d);

#define CONFIGURE_SCHEDULER_TABLE_ENTRIES \
  RTEMS_SCHEDULER_TABLE_EDF_SMP(a, SCHEDULER_A), \
  RTEMS_SCHEDULER_TABLE_EDF_SMP(b, SCHEDULER_B), \
  RTEMS_SCHEDULER_TABLE_EDF_SMP(c, SCHEDULER_C), \
  RTEMS_SCHEDULER_TABLE_EDF_SMP(d, SCHEDULER_D)

#define CONFIGURE_SCHEDULER_ASSIGNMENTS \
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
  RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(2, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(3, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL)
  
#endif

#define CONFIGURE_MAXIMUM_TASKS 10

#define CONFIGURE_MAXIMUM_TIMERS 1

#define CONFIGURE_INITIAL_EXTENSIONS RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

