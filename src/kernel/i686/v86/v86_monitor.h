#pragma once

/**
 * Runs Virtual 8086 code
 * Details:
 * - All V86 tasks must have a start and end point that can be accessed from C, which will be copied to the stack.
 * - The V86 monitor will only register the GPF handler for the time the task is run - when it exits, the handler is unregistered and control is passed back
 *      to the general handler.
 * - A custom interrupt (0xFE) will always run to handle exits from the task back to the kernel - if the command is called then EFLAGS, CS and EIP are no longer
 *      pushed to the stack and the function returns.
 * - Every V86 task will use the same offset in memory
 */

// Macro that defines the code segment that V86 runs
#define V86_CODE_SEGMENT 0x1000
// Macro that defines the data segment that V86 runs
#define V86_DATA_SEGMENT 0x0000
// Macro that defines the stack segment that V86 runs
#define V86_STACK_SEGMENT 0x8000

/**
 * @brief Initializes the V86 monitor. Does nothing for now.
 */
void v86_monitor_initialize();

/**
 * @brief Loads the V86 task into memory and begins execution.
 * @param p_task_start Starting memory location of the task
 * @param p_task_end End memory location of the task
 */
void v86_load_task(void *p_task_start, void *p_task_end);