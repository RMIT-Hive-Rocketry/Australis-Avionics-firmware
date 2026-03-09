/***********************************************************************************
 * @file        AV2m.c                                                             *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

#include "stdint.h"

#include "stm32f439xx.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "task.h"

#include "AustralisCore.h"
#include "tasklist.h"
#include "gpiopin.h"
#include "rcc.h"
#include "can.h"

#include "interrupts/interrupts.h"

#include "devices.h"

#include "broadcast_queue.h"


typedef struct {
  int16_t x;
  int16_t y;
} Test_Data_t;


void vBQueueTest( void *pvParameters ) {

  Test_Data_t data;
  data.x = 34571;
  data.y = -234;

  Broadcast_Queue_t bqueue = Broadcast_Queue_Create(sizeof(Test_Data_t), 8);

  Broadcast_Queue_Member_t bqueue_a = Broadcast_Queue_Member_Create();
  Broadcast_Queue_Subscribe(&bqueue, &bqueue_a);

  Broadcast_Queue_Member_t bqueue_b = Broadcast_Queue_Member_Create();
  Broadcast_Queue_Subscribe(&bqueue, &bqueue_b);

  Broadcast_Queue_Broadcast(&bqueue, &data);


  Test_Data_t data_a;
  xQueueReceive( bqueue_a.queue,
                 &data_a, 0);

  Test_Data_t data_b;
  xQueueReceive( bqueue_b.queue,
                 &data_b, 0);

  int dummy = 0;

  xTaskDestroy(xTaskGetCurrentTaskHandle());
}

/* =============================================================================== */
/**
 * @brief
 *
 **
 * =============================================================================== */
int main() {

  xTaskCreate(vBQueueTest, "BQueueTest", 1000, NULL, 1, NULL );
  vTaskStartScheduler();
  while(1);

}
