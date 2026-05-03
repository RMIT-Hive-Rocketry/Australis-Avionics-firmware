// sam_m10q.c

#include "sam_m10q.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

SAM_M10Q_t gps;

//TODO: Need to add STOP configuration to UART Config
const UART_Config UART_Config_SAM_M10Q = {
  .OVER8  = UART_OVER8,            // Required
  .M      = UART_WORD8,            // Required (SAM M10Q)
  .WAKE   = UART_WAKEUP_IDLE,      // Required
  .PCE    = false,                 // Required (SAM M10Q)
  .PS     = UART_PARITY_EVEN,      // Value does not matter
  .PEIE   = false,                 // Disabled
  .TXEIE  = false,                 // Disabled
  .TCIE   = false,                 // Disabled
  .RXNEIE = false,                 // Disabled
  .IDLEIE = false,                 // Disabled
  .TE     = true,                  // Required
  .RE     = true,                  // Required
  .RWU    = UART_RECEIVER_ACTIVE   // Active Mode
};



//TODO: Need to increase the baud rate of the UART line, otherwise a packet
// may take over 

bool
SAM_M10Q_Init() {
  
  // We initialize the UART line with the defualt baud rate of the SAM_M10Q.
  // Once the lines are up, we will then mutually configure the line to be
  // a higher baud rate.
  gps->uart = UART_init(GPS_INTERFACE, 38400, &UART_Config_SAM_M10Q);

  // Enable RX/TX ports.
  gps->_tx = GPIOpin_init(GPS_PORT, GPS_TX_PIN, &GPIO_Config_Default);
  gps->_rx = GPIOpin_init(GPS_PORT, GPS_RX_PIN, &GPIO_Config_Input);

  // Enable the GPS reset and keep it high to turn on the device.
  gps._reset = GPIOpin_init(GPS_RESET_PORT, GPS_RESET_PIN, &GPIO_Config_Default);
  gps._reset.set(&gps);


  //TODO Set the baud rate higher.

  //TODO Wait 100ms before starting.
  
  return true;
}


// Function
// Retrieve a frame from the GPS chip.
bool
SAM_M10Q_Receive(UART_t* uart, UBX_Frame_t* frame) {

  uint8_t* ptr_start, ptr_end;

  // Find the synchronization bits.
  if (gps->uart.receive(&uart) != 0xb5) {
    return false;
  }
  if (gps->uart.receive(&uart) != 0x62) {
    return false;
  }

  // A frame has been successfully identified, and will be read in.
  
  // Record the sync, class, ID, and length.
  ptr_start = (uint8_t*)frame;
  ptr_end   = (uint8_t*)&frame->payload;
  for (uint8_t* ptr = ptr_start; ptr < ptr_end; ptr++) {
    *ptr = gps->uart.receive(&uart);
  }

  // Record the payload.
  ptr_start = (uint8_t*)&frame->payload;
  ptr_end   = (uint8_t*)&frame->payload + frame.length;
  for (uint8_t* ptr = ptr_start; ptr < ptr_end; ptr++) {
    *ptr = gps->uart.receive(&uart);
  }

  // Record the checksum.
  frame.checksum.ck_a = gps->uart.receive(&uart);
  frame.checksum.ck_b = gps->uart.receive(&uart);

  // If the checksum doesn't match, then raise the error indicator.
  if (UBX_Checksum(frame) != frame.checksum) {
    frame.id = UBX_FAILED_MESSAGE;
  }

  // Successfully received a frame.
  return true;

}


void
SAM_M10Q_Transmit(UART_t* uart, UBX_Frame_t* frame) {

  uint8_t* ptr_start, ptr_end;

  // Prepare parts of the frame.
  frame->sync[0] = 0xb5;
  frame->sync[1] = 0x62;
  frame->checksum = UBX_Checksum(frame);

  // Send the sync, class, ID, and length.
  ptr_start = (uint8_t*)frame;
  ptr_end   = (uint8_t*)&frame->payload;
  for (uint8_t* ptr = ptr_start; ptr < ptr_end; ptr++) {
    gps->uart.send(uart, *ptr);
  }

  // Send the payload.
  ptr_start = (uint8_t*)&frame->payload;
  ptr_end   = (uint8_t*)&frame->payload + frame.length;
  for (uint8_t* ptr = ptr_start; ptr < ptr_end; ptr++) {
    gps->uart.send(uart, *ptr);
  }

  // Send the checksum.
  gps->uart.send(uart, frame->checksum.ck_a);
  gps->uart.send(uart, frame->checksum.ck_b);

  // Transmission is complete.
                 
}


bool
UBX_Parse(UBX_Frame_t* frame) {

  switch(frame->id) {
  case UBX_NAV_POSLLM:
    UBX_Parse_NAV_POSLLM(frame);
    
  default:
    // Just discard the message, do not raise error.
  }

}


void
UBX_Configure() {

  UBX_Frame_t frame;

  UBX_Payload_CFG_SETVAL_t* payload = (uint8_t*) (&frame.data)

  uint8_t config[10] = {
    
  };
  
}





GPS_Coordinates_t
UBX_Parse_NAV_POSLLM(UBX_Frame_t* frame) {

  GPS_Coordinates_t coordinates;

  UBX_Payload_NAV_POSLLH_t* payload = (UBX_Payload_NAV_POSLLH_t*) &frame->data;

  // Section 3.15.10 of datasheet.
  coordinates.longitude = ((double)payload->lon)  * 1e-7;
  coordinates.latitude  = ((double)payload->lat)  * 1e-7;
  coordinates.accuracy  = ((double)payload->hAcc) * 1e-3;

  return coordinates;
  
}




UBX_Checksum_t
UBX_Checksum(UBX_Frame_t* frame) {

  UBX_Checksum_t checksum;

  // Apply Fletcher algorithm, as per SAM_M10Q datasheets.
  for (uint16_t i = 0; i < frame->length; i++) {
    checksum.ck_a += frame->payload[i];
    checksum.ck_b += checksum.ck_a;
  }

  return checksum;

}



void checksum() {


  uint8_t ck_a = 0;
  uint8_t ck_b = 0;  
  
  for (int i = 0; i < data_length; i++) {
    ck_a = ck_a + data[i];
    ck_b = ck_b + ck_a;
  }


}
