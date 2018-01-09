#include <cyg/hal/hal_arch.h>
#include <cyg/kernel/kapi.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "ezs_counter.h"
#include "ezs_serial.h"
#include "ezs_sensor.h"
#include "ezs_stopwatch.h"
#include "ezs_adc.h"
#include "ezs_dac.h"
#include "ezs_io_fel.h"
#include "ezs_fft.h"
#include "ezs_interpolation.h"
#include "ezs_plot.h"
#include "ezs_plot_pds.h"


#define SERIAL_IRQ CYGNUM_HAL_INTERRUPT_UART2

/*
 * TODO: Code anpassen
 */
#define SAMPLING_TASK_PRIORITY 1
#define PROCRASTINATION_TASK_PRIORITY 2
#define DISPLAY_SIGNAL_TASK_PRIORITY 3
#define DISPLAY_PDS_TASK_PRIORITY 5
#define ANALYSIS_TASK_PRIORITY 4
#define DECODING_TASK_PRIORITY 0
#define POLLING_TASK_PRIORITY 0
#define STATEMACHINE_TASK_PRIORITY 0

#define SAMPLING_TASK_PERIOD 4
#define PROCRASTINATION_TASK_PERIOD 10
#define DISPLAY_SIGNAL_TASK_PERIOD 250
#define DISPLAY_PDS_TASK_PERIOD 1000
#define ANALYSIS_TASK_PERIOD 1000
#define POLLING_TASK_PERIOD 0

#define PROCRASTINATION_WCET 1

#define SAMPLING_TASK_PHASE 0
#define PROCRASTINATION_TASK_PHASE 0
#define ANALYSIS_TASK_PHASE 0
#define DISPLAY_SIGNAL_TASK_PHASE 0
#define DISPLAY_PDS_TASK_PHASE 0
#define POLLING_TASK_PHASE 0


#define STACKSIZE    (CYGNUM_HAL_STACK_SIZE_MINIMUM+4096)

#define PDS_LENGTH 32
#define TIME_DOMAIN_LENGTH (2 * PDS_LENGTH)
static cyg_uint32 s_time_domain[TIME_DOMAIN_LENGTH];
static float s_frequency_domain[PDS_LENGTH];
static unsigned int s_position = 0;

#define SERIAL_BUFFER_LENGTH 15
static char s_serial_buffer[SERIAL_BUFFER_LENGTH];
static unsigned int s_serial_position;
static bool s_command_decodable = false;


static cyg_uint32 status;   //used in stopwatch

enum CommandStatus
{
	CommandComplete,
	CommandIncomplete
};

static enum CommandStatus packet_receive(char c)
{
	assert(s_serial_position < SERIAL_BUFFER_LENGTH);
	/*
	 * TODO: Code ergaenzen
	 */

}

enum Command
{
	DisplayTime = (1 << 1),
	DisplayPDS  = (1 << 2),
	TriggerOn   = (1 << 3),
	TriggerOff  = (1 << 4),
	TLevelRise  = (1 << 5),
	TLevelFall  = (1 << 6),
	Invalid     = 0x00,
};

enum State
{
	ChangeMe =(1 << 1),
	/*
	 * TODO: Zustände ergaenzen
	 */
};



//Little helper functions.
static cyg_tick_count_t ms_to_cyg_ticks(cyg_uint32 ms)
{
	
    //newMs = newMs * 1e3;
    //newMs = newMs / 1e9;
	return 0;
}

static cyg_tick_count_t ms_to_ezs_ticks(cyg_uint32 ms) {
	ms *= 1e6;
    ms = ms / ezs_counter_get_resolution().dividend;
    ms = ms * ezs_counter_get_resolution().divisor;
	return ms;
}

enum Command decode_command(void)
{

	enum Command ret = Invalid;
	/*
	 * TODO: Code ergaenzen
	 */

	return ret;
}

cyg_uint32 serial_isr_handler(cyg_vector_t vector, cyg_addrword_t data)
{
	/*
	 * TODO: Code ergaenzen falls noetig
	 */
	if (ezs_serial_char_available())
	{
		/*
		 * TODO: Code ergaenzen
		 */
		ezs_serial_getc();
		cyg_interrupt_acknowledge(vector);
		return CYG_ISR_CALL_DSR;
	}
	else
	{
		return CYG_ISR_HANDLED;
	}

}


void serial_dsr_handler(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
	/*
	 * TODO: Code ergaenzen
	 */
}


// periodischer Zusteller -> Not used, included in decoding_task_entry, enabled via defines
// T7
static cyg_uint8     polling_task_stack[STACKSIZE];
static cyg_handle_t  polling_task_handle;
static cyg_thread    polling_task_thread;
static void polling_task_entry(cyg_addrword_t data)
{
	while (1)
	{
		/*
		 * TODO: Code ergaenzen
		 */
		cyg_thread_suspend(cyg_thread_self());
	}
}

// Zustandsmaschine
// T8
static cyg_uint8     statemachine_task_stack[STACKSIZE];
static cyg_handle_t  statemachine_task_handle;
static cyg_thread    statemachine_task_thread;
static void statemachine_task_entry(cyg_addrword_t data)
{
	while (1)
	{
		/*
		 * TODO: Code ergaenzen
		 */
	}
}

// T1
static cyg_uint8     sampling_task_stack[STACKSIZE];
static cyg_handle_t  sampling_task_handle;
static cyg_thread    sampling_task_thread;
static void sampling_task_entry(cyg_addrword_t data)
{
	while (1)
	{
        cyg_uint8 wert = ezs_adc_read();
        s_time_domain[s_position] = wert;
        s_position += 1;
        s_position = s_position%64;

		cyg_thread_suspend(cyg_thread_self());
	}
}

// T2
static cyg_uint8     procrastination_task_stack[STACKSIZE];
static cyg_handle_t  procrastination_task_handle;
static cyg_thread    procrastination_task_thread;
static void procrastination_task_entry(cyg_addrword_t data)
{
	while (1)
	{
		ezs_lose_time(ms_to_ezs_ticks(PROCRASTINATION_WCET), 100);

		cyg_thread_suspend(cyg_thread_self());
	}
}

// T3
static cyg_uint8     analysis_task_stack[STACKSIZE];
static cyg_handle_t  analysis_task_handle;
static cyg_thread    analysis_task_thread;
static cyg_uint32 counter = 0;
static cyg_uint32 max = 0;

static void analysis_task_entry(cyg_addrword_t data)
{
	while (1)
	{
        counter = 0;
        max = 0;
        while(counter < 10000){
            ezs_watch_start(&status);
            ezs_easy_pds(s_time_domain, s_frequency_domain, TIME_DOMAIN_LENGTH);
            cyg_uint32 time = ezs_watch_stop(&status);
            //ezs_printf("%d\n", time);
            if (time > max){
                max = time;
            }
            counter ++;
        }
        ezs_printf("%d\n", max);
		cyg_thread_suspend(cyg_thread_self());
	}
}


// T4
static cyg_uint8     display_signal_task_stack[STACKSIZE];
static cyg_handle_t  display_signal_task_handle;
static cyg_thread    display_signal_task_thread;
static void display_signal_task_entry(cyg_addrword_t data)
{
	while (1)
	{
		ezs_plot(s_time_domain, TIME_DOMAIN_LENGTH, FB_WHITE, FB_BLACK);


		cyg_thread_suspend(cyg_thread_self());
	}
}

// T5
static cyg_uint8     display_pds_task_stack[STACKSIZE];
static cyg_handle_t  display_pds_task_handle;
static cyg_thread    display_pds_task_thread;
static void display_pds_task_entry(cyg_addrword_t data)
{
	while (1)
	{
		ezs_plot_pds(s_frequency_domain, PDS_LENGTH, FB_WHITE, FB_BLACK);

		cyg_thread_suspend(cyg_thread_self());
	}
}


static void sampling_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(sampling_task_handle);
}

static void procrastination_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(procrastination_task_handle);
}

static void analysis_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(analysis_task_handle);
}

static void display_signal_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(display_signal_task_handle);
}

static void display_pds_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(display_pds_task_handle);
}


static cyg_handle_t sampling_task_alarm_handle;
static cyg_handle_t procrastination_task_alarm_handle;
static cyg_handle_t analysis_task_alarm_handle;
static cyg_handle_t display_signal_task_alarm_handle;
static cyg_handle_t display_pds_task_alarm_handle;

static cyg_alarm sampling_task_alarm;
static cyg_alarm procrastination_task_alarm;
static cyg_alarm analysis_task_alarm;
static cyg_alarm display_signal_task_alarm;
static cyg_alarm display_pds_task_alarm;


static cyg_handle_t real_time_counter;

static cyg_addrword_t data_dummy = 0;
static cyg_interrupt serial_intr;
static cyg_handle_t  serial_isr_handle;



void cyg_user_start(void)
{
	ezs_fb_init();
	ezs_counter_init();
	ezs_sensors_init();
	ezs_fel_serial_init();
	cyg_interrupt_create(SERIAL_IRQ,
			1,
			0,
			serial_isr_handler,
			serial_dsr_handler,
			&serial_isr_handle,
			&serial_intr) ;
	cyg_interrupt_attach(serial_isr_handle);
	cyg_interrupt_unmask(SERIAL_IRQ);


	cyg_thread_create(SAMPLING_TASK_PRIORITY, &sampling_task_entry, 0, "sampling task",
			sampling_task_stack, STACKSIZE,
			&sampling_task_handle, &sampling_task_thread);
	cyg_thread_create(PROCRASTINATION_TASK_PRIORITY, &procrastination_task_entry, 0, "procrastination task",
			procrastination_task_stack, STACKSIZE,
			&procrastination_task_handle, &procrastination_task_thread);
	cyg_thread_create(ANALYSIS_TASK_PRIORITY, &analysis_task_entry, 0, "analysis task",
			analysis_task_stack, STACKSIZE,
			&analysis_task_handle, &analysis_task_thread);
	cyg_thread_create(DISPLAY_SIGNAL_TASK_PRIORITY, &display_signal_task_entry, 0, "display signal task",
			display_signal_task_stack, STACKSIZE,
			&display_signal_task_handle, &display_signal_task_thread);
	cyg_thread_create(DISPLAY_PDS_TASK_PRIORITY, &display_pds_task_entry, 0, "display pds task",
			display_pds_task_stack, STACKSIZE,
			&display_pds_task_handle, &display_pds_task_thread);
    ezs_printf("Threads created\n");
	cyg_clock_to_counter(cyg_real_time_clock(), &real_time_counter);
	cyg_uint32 timebase = cyg_current_time() + 3;
	cyg_alarm_create(real_time_counter, sampling_task_alarmfn, data_dummy, &sampling_task_alarm_handle, &sampling_task_alarm);
	cyg_alarm_initialize(sampling_task_alarm_handle, timebase + ms_to_cyg_ticks(SAMPLING_TASK_PHASE), ms_to_cyg_ticks(SAMPLING_TASK_PERIOD));
	cyg_alarm_create(real_time_counter, procrastination_task_alarmfn, data_dummy, &procrastination_task_alarm_handle, &procrastination_task_alarm);
	cyg_alarm_initialize(procrastination_task_alarm_handle, timebase + ms_to_cyg_ticks(PROCRASTINATION_TASK_PHASE), ms_to_cyg_ticks(PROCRASTINATION_TASK_PERIOD));

	cyg_alarm_create(real_time_counter, analysis_task_alarmfn, data_dummy, &analysis_task_alarm_handle, &analysis_task_alarm);
	cyg_alarm_initialize(analysis_task_alarm_handle, timebase + ms_to_cyg_ticks(ANALYSIS_TASK_PHASE), ms_to_cyg_ticks(ANALYSIS_TASK_PERIOD));

	cyg_alarm_create(real_time_counter, display_signal_task_alarmfn, data_dummy, &display_signal_task_alarm_handle, &display_signal_task_alarm);
	cyg_alarm_initialize(display_signal_task_alarm_handle, timebase + ms_to_cyg_ticks(DISPLAY_SIGNAL_TASK_PHASE), ms_to_cyg_ticks(DISPLAY_SIGNAL_TASK_PERIOD));
	cyg_alarm_create(real_time_counter, display_pds_task_alarmfn, data_dummy, &display_pds_task_alarm_handle, &display_pds_task_alarm);
	cyg_alarm_initialize(display_pds_task_alarm_handle, timebase + ms_to_cyg_ticks(DISPLAY_PDS_TASK_PHASE), ms_to_cyg_ticks(DISPLAY_PDS_TASK_PERIOD));
}
