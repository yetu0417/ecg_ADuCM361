#include <main.h>

/**
 * ThreadDeclare.
 * A list store the threads that need to run.
 */
const ThreadDeclare thread_list[] = {
	{{Thread_adc, osPriorityNormal, 1, 0}, &tid_Thread_adc, NULL},
	{{Thread_uart, osPriorityNormal, 1, 0}, &tid_Thread_uart, NULL},
	{{Thread_led, osPriorityNormal, 1, 0}, &tid_Thread_led, NULL},
};

int main (void) {
	int i, list_len;
	
	// initialize CMSIS-RTOS 
    osKernelInitialize ();
	
    // Enable all clocks. 
	ClkDis(0);
	
	// Create threads.
	list_len = sizeof(thread_list) / sizeof(ThreadDeclare);
	for(i=0;i<list_len;++i)
	{
		*thread_list[i].tid = osThreadCreate(&thread_list[i].threadInfo, thread_list[i].argument);
	}
	
	// start thread execution 
	osKernelStart ();
}