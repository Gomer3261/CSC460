/**
 * @file   error_code.h
 *
 * @brief Error messages returned in OS_Abort().
 *        Green errors are initialization errors
 *        Red errors are runt time errors
 *
 * CSC 460/560 Real Time Operating Systems - Mantis Cheng
 *
 * @author Scott Craig
 * @author Justin Tanner
 */
#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

enum {

/** Initialize time errors. */

/** Worst case runtime is greated than period */
ERR_1_WORST_CASE_GT_PERIOD,

/** Maximum services have been initialized. */
ERR_2_MAX_SERVICES_REACHED,

/** Run time errors. */

/** User called OS_Abort() */
ERR_RUN_1_USER_CALLED_OS_ABORT,

/** Too many tasks created. Only allowed MAXPROCESS at any time.*/
ERR_RUN_2_TOO_MANY_TASKS,

/** PERIODIC task still running at end of time slot. */
ERR_RUN_3_PERIODIC_TOOK_TOO_LONG,

/** ISR made a request that only tasks are allowed. */
ERR_RUN_4_ILLEGAL_ISR_KERNEL_REQUEST,

/** RTOS Internal error in handling request. */
ERR_RUN_5_RTOS_INTERNAL_ERROR,

/** PERIODIC task collision */
ERR_RUN_6_PERIODIC_TASK_COLLISION,

/** PERIODIC task attempted to subscribe to a service */
ERR_RUN_7_PERIODIC_TASK_SUBSCRIBED,

/** PERIODIC task was found subscribed to a service
 * may happen when a task is destroyed and recreated as periodic before a publish occurs. */
ERR_RUN_8_PERIODIC_TASK_FOUND_SUBSCRIBED,

};


#endif
