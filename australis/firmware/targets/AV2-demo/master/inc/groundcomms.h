/**
 * @file groundcoms.h
 */

// ALLOW FORMATTING
#ifndef GROUNDCOMMS_H
#define GROUNDCOMMS_H

#define LORA_MESSAGE_INDEX_ID            1
#define LORA_MESSAGE_INDEX_CONTROL_FLAGS 2
#define LORA_MESSAGE_INDEX_CONTROL_INV   3
#define LORA_MESSAGE_INDEX_BCAST_FLAG    4
#define LORA_MESSAGE_ID_GCS_REQUEST      0x01
#define LORA_MESSAGE_CAM_FLAG            0b00010000
#define LORA_MESSAGE_BCAST_FLAG          0b10101010

void vGroundCommStateMachine(void *argument);

#endif
