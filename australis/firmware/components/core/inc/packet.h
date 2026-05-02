/**
 * @file packet.h
 */

// ALLOW FORMATTING
#ifndef PACKET_H
#define PACKET_H

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

typedef struct {
  uint8_t size;
  uint8_t *data;
} Field;

typedef struct {
  uint8_t id;
  uint8_t length;
  Field *fields;
} Packet;

bool Packet_asBytes(Packet *packet, uint8_t *out, uint8_t size);

#endif
