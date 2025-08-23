#include "test.h"

extern uint8_t __task_start;
extern uint8_t __task_run;
extern uint8_t __task_end;


uint8_t *get_task_start() {
    return &__task_start;
}

uint8_t *get_task_run() {
    return &__task_run;
}

uint8_t *get_task_end() {
    return &__task_end;
}