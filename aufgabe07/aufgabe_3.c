#include <cyg/kernel/kapi.h>
#include <stdio.h>

#include "ezs_stopwatch.h"
#include "ezs_io.h"

#include "common.h"



// in milliseconds
#define HIGH_TASK_PHASE   3
#define MEDIUM_TASK_PHASE 8
#define LOW_TASK_PHASE    1

#define HIGH_TASK_PERIOD   20
#define MEDIUM_TASK_PERIOD 50
#define LOW_TASK_PERIOD    200

#define HIGH_TASK_PRIORITY   11
#define MEDIUM_TASK_PRIORITY 12
#define LOW_TASK_PRIORITY    13


static cyg_uint8     high_task_stack[STACKSIZE];
static cyg_thread    high_task_thread;

static cyg_uint8     medium_task_stack[STACKSIZE];
static cyg_thread    medium_task_thread;

static cyg_uint8     low_task_stack[STACKSIZE];
static cyg_thread    low_task_thread;

static void high_task_entry(cyg_addrword_t data);
static void medium_task_entry(cyg_addrword_t data);
static void low_task_entry(cyg_addrword_t data);

cyg_handle_t  high_task_handle;
cyg_handle_t  medium_task_handle;
cyg_handle_t  low_task_handle;

static cyg_mutex_t mutex5;
static cyg_mutex_t mutex6;

static void high_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(high_task_handle);
}

static void medium_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(medium_task_handle);
}

static void low_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(low_task_handle);
}

cyg_handle_t s_high_task_alarm_handle;
cyg_alarm s_high_task_alarm;
cyg_handle_t s_medium_task_alarm_handle;
cyg_alarm s_medium_task_alarm;
cyg_handle_t s_low_task_alarm_handle;
cyg_alarm s_low_task_alarm;


void init_tasks(void)
{
    cyg_mutex_init(&mutex5);
    cyg_mutex_init(&mutex6);
    cyg_mutex_set_protocol(&mutex5, CYG_MUTEX_CEILING);
    cyg_mutex_set_protocol(&mutex6, CYG_MUTEX_CEILING);
    cyg_mutex_set_ceiling(&mutex6, 10);
    cyg_mutex_set_ceiling(&mutex5, 10);
	cyg_thread_create(HIGH_TASK_PRIORITY, &high_task_entry, 0, "high priority task",
			high_task_stack, STACKSIZE,
			&high_task_handle, &high_task_thread);
	cyg_thread_create(MEDIUM_TASK_PRIORITY, &medium_task_entry, 0, "medium priority task",
			medium_task_stack, STACKSIZE,
			&medium_task_handle, &medium_task_thread);
	cyg_thread_create(LOW_TASK_PRIORITY, &low_task_entry, 0, "low priority task",
			low_task_stack, STACKSIZE,
			&low_task_handle, &low_task_thread);


	cyg_alarm_create(s_real_time_counter, high_task_alarmfn, data_dummy, &s_high_task_alarm_handle, &s_high_task_alarm);
	cyg_alarm_initialize(s_high_task_alarm_handle, HIGH_TASK_PHASE, HIGH_TASK_PERIOD);
	cyg_alarm_create(s_real_time_counter, medium_task_alarmfn, data_dummy, &s_medium_task_alarm_handle, &s_medium_task_alarm);
	cyg_alarm_initialize(s_medium_task_alarm_handle, MEDIUM_TASK_PHASE, MEDIUM_TASK_PERIOD);
	cyg_alarm_create(s_real_time_counter, low_task_alarmfn, data_dummy, &s_low_task_alarm_handle, &s_low_task_alarm);
	cyg_alarm_initialize(s_low_task_alarm_handle, LOW_TASK_PHASE, LOW_TASK_PERIOD);
}

static void high_task_entry(cyg_addrword_t data)
{
	while (1) {
        lose_time_us(1000, 0);
        while(!cyg_mutex_lock(&mutex5)){
            lose_time_us(1,100);
        }
		lose_time_us(3000, 0);
        while(!cyg_mutex_lock(&mutex6)){
            lose_time_us(1,100);
        }
        lose_time_us(1000, 0);
        cyg_mutex_unlock(&mutex6);
        lose_time_us(1000, 0);
        cyg_mutex_unlock(&mutex5);
		cyg_thread_suspend(cyg_thread_self());
	}
}

static void medium_task_entry(cyg_addrword_t data)
{
	while (1) {
        lose_time_us(12000, 0);
		cyg_thread_suspend(cyg_thread_self());

	}
}

static void low_task_entry(cyg_addrword_t data)
{
	while (1) {
        lose_time_us(1000, 0);
        while(!cyg_mutex_lock(&mutex6)){
            lose_time_us(1,100);
        }
		lose_time_us(4000, 0);
        while(!cyg_mutex_lock(&mutex5)){
            lose_time_us(1,100);
        }
        lose_time_us(1000, 0);
        cyg_mutex_unlock(&mutex6);
        cyg_mutex_unlock(&mutex5);
		cyg_thread_suspend(cyg_thread_self());

	}
}

