#include "scheduler/scheduler.h"


bool task_switching_flag = false;


void disable_scheduler() {
    task_switching_flag = false;
}
void enable_scheduler() {
    task_switching_flag = true;
}