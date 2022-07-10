/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "CarTasks/CarTasks.h"
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
osThreadId initMotorDirectHandle;
uint32_t initMotorDirectBuffer[ 1024 ];
osStaticThreadDef_t initMotorDirectControlBlock;
osThreadId MotorRoutineTaskHandle;
uint32_t MotorRoutineTaskBuffer[ 1024 ];
osStaticThreadDef_t MotorRoutineTaskControlBlock;
osThreadId SerialCmdProcTaskHandle;
uint32_t SerialCmdProcTaskBuffer[ 1024 ];
osStaticThreadDef_t SerialCmdProcTaskControlBlock;
osMessageQId qMotorTimeupHandle;
uint8_t qMotorTimeupBuffer[ 1 * sizeof( uint8_t ) ];
osStaticMessageQDef_t qMotorTimeupControlBlock;
osMessageQId qSerialPackHandle;
uint8_t qSerialPackBuffer[ 8 * sizeof( uint32_t ) ];
osStaticMessageQDef_t qSerialPackControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void initMotorDirectionTaskFunc(void const * argument);
extern void motorRoutineTaskFunc(void const * argument);
extern void serialCmdProcTaskFunc(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  ST_LOGE("task overflow detected: %s",pcTaskName);
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   ST_LOGE("malloc fail detected");
}
/* USER CODE END 5 */

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

  /* Create the queue(s) */
  /* definition and creation of qMotorTimeup */
  osMessageQStaticDef(qMotorTimeup, 1, uint8_t, qMotorTimeupBuffer, &qMotorTimeupControlBlock);
  qMotorTimeupHandle = osMessageCreate(osMessageQ(qMotorTimeup), NULL);

  /* definition and creation of qSerialPack */
  osMessageQStaticDef(qSerialPack, 8, uint32_t, qSerialPackBuffer, &qSerialPackControlBlock);
  qSerialPackHandle = osMessageCreate(osMessageQ(qSerialPack), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of initMotorDirect */
  osThreadStaticDef(initMotorDirect, initMotorDirectionTaskFunc, osPriorityNormal, 0, 1024, initMotorDirectBuffer, &initMotorDirectControlBlock);
  initMotorDirectHandle = osThreadCreate(osThread(initMotorDirect), NULL);

  /* definition and creation of MotorRoutineTask */
  osThreadStaticDef(MotorRoutineTask, motorRoutineTaskFunc, osPriorityRealtime, 0, 1024, MotorRoutineTaskBuffer, &MotorRoutineTaskControlBlock);
  MotorRoutineTaskHandle = osThreadCreate(osThread(MotorRoutineTask), NULL);

  /* definition and creation of SerialCmdProcTask */
  osThreadStaticDef(SerialCmdProcTask, serialCmdProcTaskFunc, osPriorityIdle, 0, 1024, SerialCmdProcTaskBuffer, &SerialCmdProcTaskControlBlock);
  SerialCmdProcTaskHandle = osThreadCreate(osThread(SerialCmdProcTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_initMotorDirectionTaskFunc */
/**
  * @brief  Function implementing the initMotorDirect thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_initMotorDirectionTaskFunc */
__weak void initMotorDirectionTaskFunc(void const * argument)
{
  /* USER CODE BEGIN initMotorDirectionTaskFunc */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END initMotorDirectionTaskFunc */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
