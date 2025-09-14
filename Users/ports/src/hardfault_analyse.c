#include <stdio.h>

#include "main.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

void print_cfsr_analysis(uint32_t cfsr) {
  uint8_t mmfsr = (cfsr >> 0) & 0xFF;
  uint8_t bfsr = (cfsr >> 8) & 0xFF;
  uint16_t ufsr = (cfsr >> 16) & 0xFFFF;

  printf("SCB->CFSR = 0x%08lX\r\n", cfsr);

  if (mmfsr) {
    printf("MemManage Fault:\r\n");
    if (mmfsr & (1 << 0))
      printf("- IACCVIOL: Instruction access violation\r\n");
    if (mmfsr & (1 << 1))
      printf("- DACCVIOL: Data access violation\r\n");
    if (mmfsr & (1 << 3))
      printf("- MUNSTKERR: Unstacking error during exception\r\n");
    if (mmfsr & (1 << 4))
      printf("- MSTKERR: Stacking error during exception\r\n");
    if (mmfsr & (1 << 7)) {
      printf("- MMARVALID: MMFAR holds valid fault address\r\n");
      printf("  MMFAR = 0x%08lX\r\n", SCB->MMFAR);
    }
  }

  if (bfsr) {
    printf("BusFault:\n");
    if (bfsr & (1 << 0))
      printf("- IBUSERR: Instruction bus error\r\n");
    if (bfsr & (1 << 1))
      printf("- PRECISERR: Precise data bus error\r\n");
    if (bfsr & (1 << 2))
      printf("- IMPRECISERR: Imprecise data bus error\r\n");
    if (bfsr & (1 << 3))
      printf("- UNSTKERR: Unstacking error during exception\r\n");
    if (bfsr & (1 << 4))
      printf("- STKERR: Stacking error during exception\r\n");
    if (bfsr & (1 << 7)) {
      printf("- BFARVALID: BFAR holds valid fault address\r\n");
      printf("  BFAR = 0x%08lX\r\n", SCB->BFAR);
    }
  }

  if (ufsr) {
    printf("UsageFault:\r\n");
    if (ufsr & (1 << 0))
      printf("- UNDEFINSTR: Undefined instruction\r\n");
    if (ufsr & (1 << 1))
      printf("- INVSTATE: Invalid state\r\n");
    if (ufsr & (1 << 2))
      printf("- INVPC: Invalid PC load by EXC_RETURN\r\n");
    if (ufsr & (1 << 3))
      printf("- NOCP: No coprocessor\r\n");
    if (ufsr & (1 << 8))
      printf("- UNALIGNED: Unaligned access error\r\n");
    if (ufsr & (1 << 9))
      printf("- DIVBYZERO: Divide by zero error\r\n");
  }
}

#ifdef USE_FREERTOS
void print_current_task_info(void) {
  TaskHandle_t curr_task = xTaskGetCurrentTaskHandle();
  const char *task_name = pcTaskGetName(curr_task);
  printf("FreeRTOS Current Task Name: %s\r\n", task_name ? task_name : "<No Task>");
  printf("Task Handle: %p\n", curr_task);

  UBaseType_t stack_left = uxTaskGetStackHighWaterMark(curr_task);
  printf("Task stack high water mark: %lu (words)\r\n",
         (unsigned long)stack_left);
}
#endif

__attribute__((used)) void hard_fault_handler_c(uint32_t *stack) {
  uint32_t r0 = stack[0];
  uint32_t r1 = stack[1];
  uint32_t r2 = stack[2];
  uint32_t r3 = stack[3];
  uint32_t r12 = stack[4];
  uint32_t lr = stack[5];
  uint32_t pc = stack[6];
  uint32_t psr = stack[7];

  // !!! 注意：printf 在异常环境下可能阻塞或死锁
  printf("\n========= HardFault Detected =========\n");
  printf("R0  = 0x%08lX\r\n", r0);
  printf("R1  = 0x%08lX\r\n", r1);
  printf("R2  = 0x%08lX\r\n", r2);
  printf("R3  = 0x%08lX\r\n", r3);
  printf("R12 = 0x%08lX\r\n", r12);
  printf("LR  = 0x%08lX\r\n", lr);
  printf("PC  = 0x%08lX\r\n", pc);
  printf("xPSR= 0x%08lX\r\n", psr);
  printf("Stack address: %p\r\n", stack);

  // 可以加栈合法性检查
  // if ((uint32_t)stack < RAM_START || (uint32_t)stack > RAM_END) {
  //   printf("Stack pointer out of RAM range!\n");
  // }

  uint32_t cfsr = SCB->CFSR;
  print_cfsr_analysis(cfsr);

#ifdef USE_FREERTOS
  print_current_task_info();
#endif

  printf("======================================\r\n");
  printf("Tip: Use PC/LR values to locate the faulting function in your map or symbol file.\r\n");
  printf("Analyze the CFSR & MMFAR/BFAR above for fault reason details.\r\n");

  // 关闭所有中断, 确保停机
  __disable_irq();

  while (1) {
    // 可以点灯/蜂鸣, 用于定位异常
  }
}

void HardFault_Handler(void) {
  __asm volatile("TST LR, #4            \n"
                 "ITE EQ                \n"
                 "MRSEQ R0, MSP         \n"
                 "MRSNE R0, PSP         \n"
                 "B hard_fault_handler_c\n");

  while (1) {
  }
}
