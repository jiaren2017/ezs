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
#define SAMPLING_TASK_PRIORITY 12
#define PROCRASTINATION_TASK_PRIORITY 13
#define DISPLAY_SIGNAL_TASK_PRIORITY 14
#define DISPLAY_PDS_TASK_PRIORITY 16
#define ANALYSIS_TASK_PRIORITY 15
#define DECODING_TASK_PRIORITY 17
#define POLLING_TASK_PRIORITY 11
#define STATEMACHINE_TASK_PRIORITY 10

#define SAMPLING_TASK_PERIOD 4
#define PROCRASTINATION_TASK_PERIOD 10
#define DISPLAY_SIGNAL_TASK_PERIOD 250
#define DISPLAY_PDS_TASK_PERIOD 1000
#define ANALYSIS_TASK_PERIOD 1000
#define POLLING_TASK_PERIOD 30      //24 

#define PROCRASTINATION_WCET 1

#define SAMPLING_TASK_PHASE 0
#define PROCRASTINATION_TASK_PHASE 0.5
#define ANALYSIS_TASK_PHASE 0
#define DISPLAY_SIGNAL_TASK_PHASE 0
#define DISPLAY_PDS_TASK_PHASE 110
#define POLLING_TASK_PHASE 1.5


#define STACKSIZE    (CYGNUM_HAL_STACK_SIZE_MINIMUM+4096)

#define PDS_LENGTH 32
#define TIME_DOMAIN_LENGTH (2 * PDS_LENGTH)
static cyg_uint32 s_time_domain[TIME_DOMAIN_LENGTH];
static float s_frequency_domain[PDS_LENGTH];
static unsigned int s_position = 0;

#define SERIAL_BUFFER_LENGTH 15
static char s_serial_buffer[SERIAL_BUFFER_LENGTH];
static unsigned int s_serial_position = 0;
static bool s_command_decodable = false;


static cyg_uint32 status;   //used in stopwatch

static volatile char read;           //used for DSR

enum CommandStatus
{
	CommandComplete,
	CommandIncomplete
};

static enum CommandStatus packet_receive(char c)
{
	assert(s_serial_position < SERIAL_BUFFER_LENGTH);
	s_serial_buffer[s_serial_position] = c;
    if (c == '\n'){
        s_serial_buffer[s_serial_position] = '\0';
        s_serial_position = 0;
        return CommandComplete;                                                            // falls laenge <12 trotzdem CommandIncomplete
    } else {
        s_serial_position = (s_serial_position + 1) % SERIAL_BUFFER_LENGTH;
        return CommandIncomplete;
    }
    /*if (decode_command() == DisplayTime){
        actual = DisplayTime;
        return CommandComplete;
    } else if (decode_command() == DisplayPDS){
        actual = DisplayPDS;
        return CommandComplete;
    }
    return CommandIncomplete;*/
    
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
	Time = (1 << 1),
	PDS = (1 << 2),
};
static enum State actual;        //used in t8



//Little helper functions.
static cyg_tick_count_t ms_to_cyg_ticks(cyg_uint32 ms)
{
	
    cyg_uint64 newMs = ms;
	cyg_resolution_t resolution = cyg_clock_get_resolution(cyg_real_time_clock());
    //ezs_printf("ms: %d\n", ms);
    //ezs_printf("dividend: %d\n", resolution.dividend);
    //ezs_printf("divisor: %d\n", resolution.divisor);
    newMs *= 1e6;
    newMs = newMs * resolution.divisor;
    newMs = newMs / resolution.dividend;
    //ezs_printf("ICH WAR DA!! %d\n", newMs);
	return newMs;
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
	if (!strncmp("display signal\0", s_serial_buffer, 15)){  // \0 nicht noetig
        ret = DisplayTime;
    }
    else if (!strncmp("display pds\0", s_serial_buffer, 12)){
        ret = DisplayPDS;
    }

	return ret;
}

cyg_uint32 serial_isr_handler(cyg_vector_t vector, cyg_addrword_t data)
{
	
	if (ezs_serial_char_available())
	{
		read = ezs_serial_getc();
		cyg_interrupt_acknowledge(vector);
		return CYG_ISR_CALL_DSR;
	}
	else
	{
		return CYG_ISR_HANDLED;
	}

}

static cyg_handle_t  polling_task_handle;
static cyg_handle_t  hinter_task_handle;
static cyg_uint8 received = 0;
void serial_dsr_handler(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{   
    //ezs_printf("%c", read);
    //diag_printf("DSR\n");
	if(packet_receive(read) == CommandComplete){        //hintergrundbetrieb
        //ezs_watch_start(&status);
        //decode_command();                             //unterbrecherbetrieb
        received = 1;
        cyg_thread_resume(hinter_task_handle);        //hintergrundbetrieb
        
    }
        
}

static cyg_flag_t flag;

// hintergrundbetrieb
// T7
static cyg_uint8     hinter_task_stack[STACKSIZE];

static cyg_thread    hinter_task_thread;
static cyg_handle_t statemachine_task_handle;

static void hinter_task_entry(cyg_addrword_t data)
{
	while (1)
	{
		enum Command modus = decode_command();
        //ezs_printf(s_serial_buffer);
        //cyg_uint32 time = ezs_watch_stop(&status);
        //ezs_printf("%d\n", time);
        received = 0;
        
        if (modus == DisplayTime || modus == DisplayPDS){
            ezs_printf("Setbits\n");
            cyg_flag_setbits(&flag, modus);
        }
        //cyg_thread_resume(statemachine_task_handle);  //passiert schon bei setbits
		cyg_thread_suspend(cyg_thread_self());
        
        
	}
}

static cyg_uint8     polling_task_stack[STACKSIZE];

static cyg_thread    polling_task_thread;
static void polling_task_entry(cyg_addrword_t data)
{
	while (1)
	{
        //ezs_printf("Waked up\n");
        if(received){
            decode_command();
            received = 0;
            ezs_printf("Polled\n");
        }
        //ezs_printf("Hallo\n");
        //cyg_uint32 time = ezs_watch_stop(&status);
        //ezs_printf("%d\n", time);
        cyg_thread_suspend(cyg_thread_self());
        
        
	}
}

// Zustandsmaschine
// T8
static cyg_uint8     statemachine_task_stack[STACKSIZE];
static cyg_handle_t  statemachine_task_handle;
static cyg_thread    statemachine_task_thread;


static bool alarm3 = false;
static bool alarm4 = false;
static bool alarm5 = false;
static cyg_handle_t analysis_task_alarm_handle;
static cyg_handle_t display_signal_task_alarm_handle;
static cyg_handle_t display_pds_task_alarm_handle;

static void statemachine_task_entry(cyg_addrword_t data)
{
	while (1)
	{
        ezs_printf("T8\n");
		actual = cyg_flag_wait(&flag, (Time | PDS), CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
        ezs_printf("T8 aktiviert\n");
        if (actual == Time){
            ezs_printf("Time aktiviert\n");
            cyg_alarm_disable(analysis_task_alarm_handle);
            alarm3 = false;
            cyg_alarm_disable(display_pds_task_alarm_handle);
            alarm5 = false;
            if (!alarm4){
                cyg_alarm_enable(display_signal_task_alarm_handle);
                alarm4 = true;
            }
        }
        if (actual == PDS){
            ezs_printf("PDS aktiviert\n");
            cyg_alarm_disable(display_signal_task_alarm_handle);
            alarm4 = false;
            if (!alarm3){
                cyg_alarm_enable(analysis_task_alarm_handle);
                alarm3 = true;
            }
            if (!alarm5){
                cyg_alarm_enable(display_pds_task_alarm_handle);
                alarm5 = true;
            }
        }
      //cyg_thread_suspend(cyg_thread_self());  
        
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
        //ezs_printf("sampling\n");
        cyg_uint8 wert = ezs_adc_read();
        s_time_domain[s_position] = wert;
        s_position += 1;
        s_position = s_position%TIME_DOMAIN_LENGTH;

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
static cyg_uint32 new_time_domain[TIME_DOMAIN_LENGTH];

static void analysis_task_entry(cyg_addrword_t data)
{
	while (1)
	{
        //ezs_printf("analysis\n");
        //counter = 0;
        //max = 0;
        //while(counter < 10000){
            //ezs_watch_start(&status);
        cyg_uint8 actualPosition = s_position;
        cyg_uint32 i = 0;
        for (i = 0; i < TIME_DOMAIN_LENGTH; i++){
            new_time_domain[i] = s_time_domain[actualPosition];
            actualPosition = (actualPosition + 1) % TIME_DOMAIN_LENGTH;
        }
            
            ezs_easy_pds(new_time_domain, s_frequency_domain, TIME_DOMAIN_LENGTH);
            //cyg_uint32 time = ezs_watch_stop(&status);
            //ezs_printf("%d\n", time);display_pds
           // if (time > max){
           //     max = time;
           // }
          //  counter ++;
        //}
       // ezs_printf("%d\n", max);
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
        //ezs_printf("display_signal\n");
        //counter = 0;
        //max = 0;
        //while(counter < 10000){
            //ezs_watch_start(&status);
        ezs_printf("T4\n");
        cyg_uint8 actualPosition = s_position;
        cyg_uint32 i = 0;
        for (i = 0; i < TIME_DOMAIN_LENGTH; i++){
            new_time_domain[i] = s_time_domain[actualPosition];
            actualPosition = (actualPosition + 1) % TIME_DOMAIN_LENGTH;
        }
        ezs_plot(new_time_domain, TIME_DOMAIN_LENGTH, CYG_FB_DEFAULT_PALETTE_BLACK, CYG_FB_DEFAULT_PALETTE_BLACK);
            //packet_receive('\0');
            //cyg_uint32 time = ezs_watch_stop(&status);
            //ezs_printf("%d\n", time);
            //if (time > max){
                //max = time;
            //}
            //counter ++;
        //}
        //ezs_printf("%d\n", max);


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
        //ezs_printf("display_pds\n");
        //counter = 0;
        //max = 0;
        //while(counter < 1000){
            //ezs_watch_start(&status);
            ezs_plot_pds(s_frequency_domain, PDS_LENGTH, CYG_FB_DEFAULT_PALETTE_WHITE, CYG_FB_DEFAULT_PALETTE_BLACK);  //koennte von analyse unterbrochen werden
            //cyg_uint32 time = ezs_watch_stop(&status);
            //ezs_printf("%d\n", time);
            //if (time > max){
            //    max = time;
            //}
           // counter ++;
        //}
        //ezs_printf("%d\n", max);

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

static void polling_task_alarmfn(cyg_handle_t alarmH, cyg_addrword_t data)
{
	cyg_thread_resume(polling_task_handle);
}


static cyg_handle_t sampling_task_alarm_handle;
static cyg_handle_t procrastination_task_alarm_handle;

static cyg_handle_t polling_task_alarm_handle;

static cyg_alarm sampling_task_alarm;
static cyg_alarm procrastination_task_alarm;
static cyg_alarm analysis_task_alarm;
static cyg_alarm display_signal_task_alarm;
static cyg_alarm display_pds_task_alarm;
static cyg_alarm polling_task_alarm;


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
    cyg_flag_init(&flag);


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
    
    cyg_thread_create(DECODING_TASK_PRIORITY, &hinter_task_entry, 0, "hintergrundbetrieb",
			hinter_task_stack, STACKSIZE,
			&hinter_task_handle, &hinter_task_thread);
    
    cyg_thread_create(POLLING_TASK_PRIORITY, &polling_task_entry, 0, "periodischer zusteller",
			polling_task_stack, STACKSIZE,
			&polling_task_handle, &polling_task_thread);
    
    cyg_thread_create(STATEMACHINE_TASK_PRIORITY, &statemachine_task_entry, 0, "statemachine",
            statemachine_task_stack, STACKSIZE,
            &statemachine_task_handle, &statemachine_task_thread);
    
    
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
    alarm3 = true;
    alarm4 = true;
    alarm5 = true;
    
    cyg_thread_resume(statemachine_task_handle);
    
    //cyg_alarm_create(real_time_counter, polling_task_alarmfn, data_dummy, &polling_task_alarm_handle, &polling_task_alarm);
	//cyg_alarm_initialize(polling_task_alarm_handle, timebase + ms_to_cyg_ticks(POLLING_TASK_PHASE), ms_to_cyg_ticks(POLLING_TASK_PERIOD));
}
