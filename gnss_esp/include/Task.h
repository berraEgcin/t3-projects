#ifndef TASK_H
#define TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

void gnss_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif