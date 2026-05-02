/***********************************************************************************
 * @file        tasklist.c                                                       *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Task_Management                                                  *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "tasklist.h"

#include "string.h"

static TaskHandle_t tasklist[MAX_TASKS];

/* ============================================================================================== */
/**
 * @brief  Retrieve task handle from list by name string
 *
 * @param  name String representation of task assigned to handle
 *
 * @return Pointer to task handle with matching name, NULL if not found.
 **
 * ============================================================================================== */
TaskHandle_t TaskList_getTaskByName(char *name) {
  // Iterate through task handle list
  for (int i = 0; i < MAX_TASKS; i++) {
    if (!strcmp(pcTaskGetName(tasklist[i]), name)) {
      // Return task handle if name matches
      return tasklist[i];
    }
  }
  // Otherwise, return NULL if not found
  return NULL;
}

/* ============================================================================================== */
/**
 * @brief  Retrieve a pointer to the first empty task handle in list
 *
 * @return Pointer to empty task handle if available, NULL if list is full.
 **
 * ============================================================================================== */
TaskHandle_t *TaskList_new() {
  // Iterate through task handle list
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasklist[i] == NULL) {
      // Return first empty handle
      return &tasklist[i];
    }
  }
  // Return NULL if list is full
  return NULL;
}

/* ============================================================================================== */
/**
 * @brief
 *
 * @return
 **
 * ============================================================================================== */
void TaskList_forEach(void (*func)(TaskHandle_t)) {
  // Iterate through task handle list
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasklist[i] == NULL) {
      break;
    }
    func(tasklist[i]);
  }
}

/** @} */
