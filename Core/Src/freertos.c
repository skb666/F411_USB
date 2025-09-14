/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "SEGGER_RTT.h"
#include "xcmd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId app_startHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void AppTaskStart(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
volatile static unsigned long tim_cnt = 0;

void tim_cnt_incrense(void) {
  tim_cnt++;
}

/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
  tim_cnt = 0;
}

__weak unsigned long getRunTimeCounterValue(void)
{
  return tim_cnt;
}

void RecordErrorInformationHere(const char *const pcFile, int ulLine) {
    printf("FreeRTOS Kernel Fault: %s, %d Assertion Fail!\r\n", pcFile, ulLine);
    __asm__ volatile("cpsid if");
    __asm__ volatile("b .");
}

void vAssertCalled(const char *pcFile, int ulLine) {
    RecordErrorInformationHere(pcFile, ulLine);
    /* Disable interrupts so the tick interrupt stops executing, then sit in a loop
     * so execution does not move past the line that failed the assertion. */
    taskDISABLE_INTERRUPTS();
    for (;;);
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   printf("FreeRTOS Kernel Fault:%20s Task StackOverFlow!\r\n", pcTaskName);
}
/* USER CODE END 4 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of app_start */
  osThreadDef(app_start, AppTaskStart, osPriorityNormal, 0, 256);
  app_startHandle = osThreadCreate(osThread(app_start), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_AppTaskStart */
#define ARRAY_SIZE(array_name) ((int)(sizeof(array_name) / sizeof(array_name[0])))

static void xcmd_entry(void *arg);

static struct TaskInit_t {
  TaskFunction_t pxTaskCode;
  const char *const pcName;
  const configSTACK_DEPTH_TYPE uxStackDepth;
  void *const pvParameters;
  UBaseType_t uxPriority;
} xTaskInitTable[] = {
  {xcmd_entry, "shell", 512, NULL, 6},
};
static TaskHandle_t xTaskHandles[ARRAY_SIZE(xTaskInitTable)];

/**
  * @brief  Function implementing the app_start thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_AppTaskStart */
__weak void AppTaskStart(void const * argument)
{
  /* USER CODE BEGIN AppTaskStart */
  BaseType_t ret;

  for (size_t i = 0; i < ARRAY_SIZE(xTaskInitTable); ++i) {
    ret = xTaskCreate(xTaskInitTable[i].pxTaskCode,
                      xTaskInitTable[i].pcName,
                      xTaskInitTable[i].uxStackDepth,
                      xTaskInitTable[i].pvParameters,
                      xTaskInitTable[i].uxPriority,
                      &xTaskHandles[i]);
    if (pdPASS != ret) {
        printf("\r\nTask \"%s\" failed to be created! (%ld)\r\n", xTaskInitTable[i].pcName, ret);
        Error_Handler();
    }
  }

  for (;;) {
    LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osDelay(500);
  }
  /* USER CODE END AppTaskStart */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void taskYieldToAll(void) {
  UBaseType_t uxSaved = uxTaskPriorityGet(NULL);
  vTaskPrioritySet(NULL, tskIDLE_PRIORITY);  // 降到 0
  /* 此时调度器立刻选下一个最高就绪任务 */
  vTaskPrioritySet(NULL, uxSaved);  // 再升回来
}

int cmd_get_char(uint8_t *ch) {
  return (1 == SEGGER_RTT_Read(0, ch, 1));
}

int cmd_put_char(uint8_t ch) {
  return SEGGER_RTT_PutChar(0, ch);
}

static void xcmd_entry(void *arg) {
  xcmd_init(cmd_get_char, cmd_put_char);

  for (;;) {
    xcmd_task();
    taskYieldToAll();
  }
}

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskRUNNING_CHAR ('X')
#define tskBLOCKED_CHAR ('B')
#define tskREADY_CHAR ('R')
#define tskDELETED_CHAR ('D')
#define tskSUSPENDED_CHAR ('S')

int cmd_ps(int argc, char **argv) {
  char pxWriteBuffer[256];
  TaskStatus_t *pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, x;
  uint32_t ulTotalRunTime, ulStatsAsPercentage;
  char cStatus;

  /* Take a snapshot of the number of tasks in case it changes while this
      function is executing. */
  uxArraySize = uxTaskGetNumberOfTasks();

  /* Allocate a TaskStatus_t structure for each task. An array could be
      allocated statically at compile time. */
  pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

  if (pxTaskStatusArray != NULL) {
    snprintf(pxWriteBuffer, sizeof(pxWriteBuffer),
              "%-*s %-*s %-*s %-*s %-*s %-*s %s\r\n",
              16, "thread",
              8, "status",
              8, "prio",
              8, "remain",
              8, "id",
              16, "tick",
              "used");
    printf("%s", pxWriteBuffer);
    snprintf(pxWriteBuffer, sizeof(pxWriteBuffer),
              "%-*s %-*s %-*s %-*s %-*s %-*s %s\r\n",
              16, "----------------",
              8, "--------",
              8, "--------",
              8, "--------",
              8, "--------",
              16, "----------------",
              "-----");
    printf("%s", pxWriteBuffer);

    /* Generate raw status information about each task. */
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

    /* For percentage calculations. */
    ulTotalRunTime /= 100UL;

    /* Avoid divide by zero errors. */
    if (ulTotalRunTime > 0) {
      /* For each populated position in the pxTaskStatusArray array,
          format the raw data as human readable ASCII data. */
      for (x = 0; x < uxArraySize; x++) {
        switch (pxTaskStatusArray[x].eCurrentState) {
          case eRunning:
            cStatus = tskRUNNING_CHAR;
            break;

          case eReady:
            cStatus = tskREADY_CHAR;
            break;

          case eBlocked:
            cStatus = tskBLOCKED_CHAR;
            break;

          case eSuspended:
            cStatus = tskSUSPENDED_CHAR;
            break;

          case eDeleted:
            cStatus = tskDELETED_CHAR;
            break;

          case eInvalid: /* Fall through. */
          default:       /* Should not get here, but it is included
                          * to prevent static checking errors. */
            cStatus = (char)0x00;
            break;
        }

        /* What percentage of the total run time has the task used?
            This will always be rounded down to the nearest integer.
            ulTotalRunTimeDiv100 has already been divided by 100. */
        ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;

        if (ulStatsAsPercentage > 0UL) {
          snprintf(pxWriteBuffer, sizeof(pxWriteBuffer),
                    "%-*s %-*c %-*u %-*u %-*u %-*lu %lu%%\r\n",
                    16, pxTaskStatusArray[x].pcTaskName,
                    8, cStatus,
                    8, (unsigned int)pxTaskStatusArray[x].uxCurrentPriority,
                    8, (unsigned int)pxTaskStatusArray[x].usStackHighWaterMark,
                    8, (unsigned int)pxTaskStatusArray[x].xTaskNumber,
                    16, pxTaskStatusArray[x].ulRunTimeCounter,
                    ulStatsAsPercentage);
          printf("%s", pxWriteBuffer);
        } else {
          /* If the percentage is zero here then the task has
              consumed less than 1% of the total run time. */
          snprintf(pxWriteBuffer, sizeof(pxWriteBuffer),
                    "%-*s %-*c %-*u %-*u %-*u %-*lu %s\r\n",
                    16, pxTaskStatusArray[x].pcTaskName,
                    8, cStatus,
                    8, (unsigned int)pxTaskStatusArray[x].uxCurrentPriority,
                    8, (unsigned int)pxTaskStatusArray[x].usStackHighWaterMark,
                    8, (unsigned int)pxTaskStatusArray[x].xTaskNumber,
                    16, pxTaskStatusArray[x].ulRunTimeCounter,
                    "<1%");
          printf("%s", pxWriteBuffer);
        }
      }
    }

    /* The array is no longer needed, free the memory it consumes. */
    vPortFree(pxTaskStatusArray);
  }

  snprintf(pxWriteBuffer, sizeof(pxWriteBuffer), ">>-------------------  free heap memory: %6d bytes  -------------------<<\r\n", xPortGetFreeHeapSize());
  printf("%s", pxWriteBuffer);

  return 0;
}
XCMD_EXPORT_CMD(ps, cmd_ps, "list task info");
/* USER CODE END Application */
