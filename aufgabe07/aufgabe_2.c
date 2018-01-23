#include <cyg/kernel/kapi.h>
#include <stdio.h>

#include "ezs_stopwatch.h"
#include "ezs_io.h"

#include "common.h"


// in milliseconds

#define T1_TASK_PERIOD 20
#define T2_TASK_PERIOD 50
#define T3_TASK_PERIOD 100
#define T4_TASK_PERIOD 200


#define T1_TASK_PHASE 7
#define T2_TASK_PHASE 5
#define T3_TASK_PHASE 3
#define T4_TASK_PHASE 1

#define T1_TASK_PRIORITY 11
#define T2_TASK_PRIORITY 12
#define T3_TASK_PRIORITY 13
#define T4_TASK_PRIORITY 14



cyg_handle_t  t1_task_handle;
cyg_handle_t  t2_task_handle;
cyg_handle_t  t3_task_handle;
cyg_handle_t  t4_task_handle;

static cyg_uint8     t1_task_stack[STACKSIZE];
static cyg_thread    t1_task_thread;

static cyg_uint8     t2_task_stack[STACKSIZE];
static cyg_thread    t2_task_thread;

static cyg_uint8     t3_task_stack[STACKSIZE];
static cyg_thread    t3_task_thread;

static cyg_uint8     t4_task_stack[STACKSIZE];
static cyg_thread    t4_task_thread;

static void t1_task_entry(cyg_addrword_t data);
static void t2_task_entry(cyg_addrword_t data);
static void t3_task_entry(cyg_addrword_t data);
static void t4_task_entry(cyg_addrword_t data);

static cyg_mutex_t mutex2;
static cyg_mutex_t mutex3;
static cyg_mutex_t mutex4;



static void t1_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(t1_task_handle);
}

static void t2_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(t2_task_handle);
}

static void t3_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(t3_task_handle);
}

static void t4_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(t4_task_handle);
}

static cyg_handle_t s_t1_task_alarm_handle;
static cyg_alarm s_t1_task_alarm;
static cyg_handle_t s_t2_task_alarm_handle;
static cyg_alarm s_t2_task_alarm;
static cyg_handle_t s_t3_task_alarm_handle;
static cyg_alarm s_t3_task_alarm;
static cyg_handle_t s_t4_task_alarm_handle;
static cyg_alarm s_t4_task_alarm;



void init_tasks(void)
{
    cyg_mutex_init(&mutex2);
    cyg_mutex_init(&mutex3);
    cyg_mutex_init(&mutex4);
    cyg_mutex_set_protocol(&mutex2, CYG_MUTEX_INHERIT);
    cyg_mutex_set_protocol(&mutex3, CYG_MUTEX_INHERIT);
    cyg_mutex_set_protocol(&mutex4, CYG_MUTEX_INHERIT);
	cyg_thread_create(T1_TASK_PRIORITY, &t1_task_entry, 0, "t1",
			t1_task_stack, STACKSIZE,
			&t1_task_handle, &t1_task_thread);
	cyg_thread_create(T2_TASK_PRIORITY, &t2_task_entry, 0, "t2",
			t2_task_stack, STACKSIZE,
			&t2_task_handle, &t2_task_thread);
	cyg_thread_create(T3_TASK_PRIORITY, &t3_task_entry, 0, "t3",
			t3_task_stack, STACKSIZE,
			&t3_task_handle, &t3_task_thread);
	cyg_thread_create(T4_TASK_PRIORITY, &t4_task_entry, 0, "t4",
			t4_task_stack, STACKSIZE,
			&t4_task_handle, &t4_task_thread);

	cyg_alarm_create(s_real_time_counter, t1_task_alarmfn, data_dummy, &s_t1_task_alarm_handle, &s_t1_task_alarm);
	cyg_alarm_initialize(s_t1_task_alarm_handle, T1_TASK_PHASE, T1_TASK_PERIOD);
	cyg_alarm_create(s_real_time_counter, t2_task_alarmfn, data_dummy, &s_t2_task_alarm_handle, &s_t2_task_alarm);
	cyg_alarm_initialize(s_t2_task_alarm_handle, T2_TASK_PHASE, T2_TASK_PERIOD);
	cyg_alarm_create(s_real_time_counter, t3_task_alarmfn, data_dummy, &s_t3_task_alarm_handle, &s_t3_task_alarm);
	cyg_alarm_initialize(s_t3_task_alarm_handle, T3_TASK_PHASE, T3_TASK_PERIOD);
	cyg_alarm_create(s_real_time_counter, t4_task_alarmfn, data_dummy, &s_t4_task_alarm_handle, &s_t4_task_alarm);
	cyg_alarm_initialize(s_t4_task_alarm_handle, T4_TASK_PHASE, T4_TASK_PERIOD);
    
    
}

static void t1_task_entry(cyg_addrword_t data)
{
	while (1) {
        lose_time_us(1000, 0);
        while(!cyg_mutex_lock(&mutex2)){
            lose_time_us(1,100);
        }
		lose_time_us(1000, 0);
        cyg_mutex_unlock(&mutex2);
        cyg_thread_suspend(cyg_thread_self());

	}
}

static void t2_task_entry(cyg_addrword_t data)
{
	while (1) {
        lose_time_us(1000, 0);
        while(!cyg_mutex_lock(&mutex2)){
            lose_time_us(1,100);
        }
		lose_time_us(2000, 0);
        while(!cyg_mutex_lock(&mutex3)){
            lose_time_us(1,100);
        }
        lose_time_us(2000, 0);
        cyg_mutex_unlock(&mutex3);
        lose_time_us(1000, 0);
        cyg_mutex_unlock(&mutex2);
		cyg_thread_suspend(cyg_thread_self());

	}
}

static void t3_task_entry(cyg_addrword_t data)
{
	while (1) {
        lose_time_us(1000, 0);
        while(!cyg_mutex_lock(&mutex3)){
            lose_time_us(1,100);
        }
		lose_time_us(3000, 0);
        while(!cyg_mutex_lock(&mutex4)){
            lose_time_us(1,100);
        }
        lose_time_us(2000, 0);
        cyg_mutex_unlock(&mutex3);
        cyg_mutex_unlock(&mutex4);
		cyg_thread_suspend(cyg_thread_self());

	}
}

static void t4_task_entry(cyg_addrword_t data)
{
	while (1) {
        lose_time_us(1000, 0);
        while(!cyg_mutex_lock(&mutex4)){
            lose_time_us(1,100);
        }
		lose_time_us(9000, 0);
        cyg_mutex_unlock(&mutex4);
		cyg_thread_suspend(cyg_thread_self());

	}
}

