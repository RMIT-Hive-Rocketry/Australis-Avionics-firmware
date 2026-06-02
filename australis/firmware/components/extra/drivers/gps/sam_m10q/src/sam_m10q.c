// sam_m10q.c

#include "sam_m10q.h"

// #include "string.h"
// #include "stdio.h"
// #include "stdlib.h"


//TODO: Need to add STOP configuration to UART Config
static UART_Config UART_Config_SAM_M10Q = {
  .OVER8  = UART_OVER8,            // Required
  .M      = UART_WORD8,            // Required (SAM M10Q)
  .WAKE   = UART_WAKEUP_IDLE,      // Required
  .PCE    = false,                 // Required (SAM M10Q)
  .PS     = UART_PARITY_EVEN,      // Value does not matter
  .PEIE   = false,                 // Disabled
  .TXEIE  = false,                 // Disabled
  .TCIE   = false,                 // Disabled
  .RXNEIE = false,                 // Disabled (Enable in future)
  .IDLEIE = false,                 // Disabled
  .TE     = true,                  // Required
  .RE     = true,                  // Required
  .RWU    = UART_RECEIVER_ACTIVE   // Active Mode
};


const UBX_Message_t UBX_ACK_ACK    = { .class = 0x05, .id = 0x01 };
const UBX_Message_t UBX_ACK_NAK    = { .class = 0x05, .id = 0x00 };
const UBX_Message_t UBX_CFG_VALSET = { .class = 0x06, .id = 0x8a };
const UBX_Message_t UBX_NAV_POSLLH = { .class = 0x01, .id = 0x02 };



static SAM_M10Q_t _gps;
SAM_M10Q_t* gps = &_gps;

const uint8_t preamble_sync_1 = 0xb5;
const uint8_t preamble_sync_2 = 0x62;



/*******************************************************************************
    PRIVATE FUNCTIONS
*******************************************************************************/



// Fletcher's checksum algorithm [1, pp.44]
static UBX_Checksum_t
UBX_Checksum(UBX_Frame_t* frame) {

  UBX_Checksum_t checksum;
  checksum.ck_a = 0x00;
  checksum.ck_b = 0x00;

  // Apply Fletcher algorithm, as per SAM_M10Q datasheets.
  for (uint16_t i = 0; i < frame->length; i++) {
    checksum.ck_a += frame->payload[i];
    checksum.ck_b += checksum.ck_a;
  }

  return checksum;

}




/*******************************************************************************
    PUBLIC FUNCTIONS
*******************************************************************************/



bool
SAM_M10Q_Init() {
    
  // Enable RX/TX ports.
  GPIO_Config gpio_config_uart_tx = {
    .mode  = GPIO_MODE_AF,
    .type  = GPIO_TYPE_PUSHPULL,
    .speed = GPIO_SPEED_MEDIUM,
    .pupd  = GPIO_PUPD_NONE,
    .afr   = GPIO_AF7
  };

  GPIO_Config gpio_config_uart_rx = {
    .mode  = GPIO_MODE_AF,
    .type  = GPIO_TYPE_PUSHPULL,
    .speed = GPIO_SPEED_MEDIUM,
    .pupd  = GPIO_PUPD_PULLUP,
    .afr   = GPIO_AF7
  };

  gps->_tx = GPIOpin_init(GPS_PORT, GPS_TX_PIN, &gpio_config_uart_tx); // &GPIO_Config_Default);
  gps->_rx = GPIOpin_init(GPS_PORT, GPS_RX_PIN, &gpio_config_uart_rx); // &GPIO_Config_Input);

  // Keep the GPS RESET_N pin high to take it out of reset mode [2, Table 9].
  gps->_reset = GPIOpin_init(GPS_RESET_PORT, GPS_RESET_PIN, &GPIO_Config_Default);
  gps->_reset.set(&gps->_reset);


  
  // We initialize the UART line with the defualt baud rate of the SAM_M10Q.
  // Once the lines are up, we will then mutually configure the line to be
  // a higher baud rate.
  gps->uart = UART_init(GPS_INTERFACE, GPS_BAUD_STARTUP, &UART_Config_SAM_M10Q);
  

  //TODO: Need to increase the baud rate of the UART line, otherwise a packet
  // may take too long

  // A 100ms delay is necessary after reconfiguring the UART, according to the
  // SAM_M10Q datasheets.
  //vTaskDelayUntil(pdMS_TO_TICKS(100));


  UBX_Configure_Default();
  
  return true;
}








bool
SAM_M10Q_Receive() {

  static uint8_t HISTORY[256];
  static uint16_t HISTORY_I = 0;

  uint8_t byte = gps->uart.receive(&gps->uart);
  if (HISTORY_I < 256) HISTORY[HISTORY_I++] = byte; else HISTORY_I = 0;

  uint8_t* frame = (uint8_t*) &gps->_message_frame;

  // _message_complete should be cleared by other parts of the software.
  if (gps->_message_state == SAM_M10Q_Frame_State__Complete) {
    return false;
  }
  
  // Check if we are in the middle of a message.
  // If so, then process this first.
  if (gps->_message_state == SAM_M10Q_Frame_State__Incoming) {

    // Copy data at start of frame, and payload as well.
    if ((gps->_message_position < PAYLOAD_BYTE_OFFSET)               ||
        (gps->_message_position < PAYLOAD_BYTE_OFFSET + gps->_message_frame.length)) {
      frame[gps->_message_position] = byte;
    } // Copy the checksum seperately.
    else if (gps->_message_position == PAYLOAD_BYTE_OFFSET + gps->_message_frame.length + 1) {
      gps->_message_frame.checksum.ck_a = byte;
    }
    else if (gps->_message_position == PAYLOAD_BYTE_OFFSET + gps->_message_frame.length + 2) {
      gps->_message_frame.checksum.ck_b = byte;
      gps->_message_state = SAM_M10Q_Frame_State__Complete;
    }

    // Increment position.
    gps->_message_position++;    
  }

  
  // Check if we are waiting for sync bits to come through.
  if (gps->_message_state == SAM_M10Q_Frame_State__Sync_Wait) {
    gps->_message_frame.sync[0] = gps->_message_frame.sync[1];
    gps->_message_frame.sync[1] = byte;

    // On successful synchronization, trigger message start and copy bytes until
    // the checksum.
    if ((gps->_message_frame.sync[0] == 0xb5) &&
        (gps->_message_frame.sync[1] == 0x62)) {
      gps->_message_state = SAM_M10Q_Frame_State__Incoming;
      gps->_message_position = 2;      
    }
  }


  // if the message was completed in a prior section...
  if (gps->_message_state == SAM_M10Q_Frame_State__Complete) {    

    // If the checksum matches, return success.
    // If not, drop the packet and look for a different one.
    UBX_Checksum_t checksum = UBX_Checksum(&gps->_message_frame);
    if ((checksum.ck_a == gps->_message_frame.checksum.ck_a) &&
        (checksum.ck_b == gps->_message_frame.checksum.ck_b)) {
      return true;
    } else {
      gps->_message_state = SAM_M10Q_Frame_State__Sync_Wait;
    }
  }

  // Default return false value.
  return false;

}


void
SAM_M10Q_Transmit(UBX_Frame_t* frame) {

  UART_t* uart = &gps->uart;
  uint8_t* ptr_start;
  uint8_t* ptr_end;

  // Prepare parts of the frame.
  frame->sync[0] = 0xb5;
  frame->sync[1] = 0x62;
  frame->checksum = UBX_Checksum(frame);

  // Send the sync, class, ID, and length.
  gps->uart.send(uart, frame->sync[0]);
  gps->uart.send(uart, frame->sync[1]);
  gps->uart.send(uart, frame->message.class);
  gps->uart.send(uart, frame->message.id);
  gps->uart.send(uart, (uint8_t) (frame->length & 0xFF));
  gps->uart.send(uart, (uint8_t) (frame->length >> 8));
  
  // ptr_start = (uint8_t*)frame;
  // ptr_end   = (uint8_t*)(&frame->payload);
  // for (uint8_t* ptr = ptr_start; ptr < ptr_end; ptr++) {
  //   gps->uart.send(uart, *ptr);
  // }

  // Send the payload.
  uint8_t* payload = (uint8_t*)&frame->payload;
  for (uint8_t i = 0; i < frame->length; i++) {
    gps->uart.send(uart, payload[i]);
  }

  // Send the checksum.
  gps->uart.send(uart, frame->checksum.ck_a);
  gps->uart.send(uart, frame->checksum.ck_b);

  // Transmission is complete.
}



static GPS_Coordinates_t
UBX_Parse_NAV_POSLLH(UBX_Frame_t* frame) {

  GPS_Coordinates_t coordinates;

  UBX_Payload_NAV_POSLLH_t* payload = (UBX_Payload_NAV_POSLLH_t*) &frame->payload;

  // Scale values according to Section 3.15.10 of datasheet.
  coordinates.longitude = ((double)payload->lon)  * 1e-7;
  coordinates.latitude  = ((double)payload->lat)  * 1e-7;
  coordinates.accuracy  = ((double)payload->hAcc) * 1e-3;

  return coordinates;
  
}


// Must be used carefully by a programmer. Assumes data will be of sufficient
//  size to contain the output from the parsing functions.
bool
UBX_Parse(UBX_Frame_t* frame, void* buffer) {

  uint8_t* _buffer = (uint8_t*) buffer;
  uint8_t* data;
  size_t data_length;
  
  if ((frame->message.class == UBX_NAV_POSLLH.class ) &&
      (frame->message.id    == UBX_NAV_POSLLH.id    )) {

    GPS_Coordinates_t gps_coordinates;
    gps_coordinates = UBX_Parse_NAV_POSLLH(frame);
    data = (uint8_t*) &gps_coordinates;
    data_length = sizeof(GPS_Coordinates_t);
  }
  else {
    return false;
  }

  // Copy data into the buffer.
  for (size_t i = 0; i < data_length; i++) {
    _buffer[i] = data[i];
  }

  return true;
  
}


static void
Store_Word_Unaligned(uint32_t word, uint8_t* buffer)
{
  uint8_t* word_bytes = (uint8_t*)&word;
  for (uint8_t i = 0; i < sizeof(word); i++) {
    buffer[i] = word_bytes[i];
  }
}



// Though multiple key-value pairs can be packed into one frame, for now it is
// simpler to do just one per frame.
bool
UBX_Configure(UBX_Configuration_t config) {

  uint8_t* ptr_source;
  uint8_t* ptr_destination;

  UBX_Frame_t frame;
  frame.message.class = UBX_CFG_VALSET.class;
  frame.message.id    = UBX_CFG_VALSET.id;

  
  UBX_Payload_CFG_VALSET_t* payload = (UBX_Payload_CFG_VALSET_t*) &frame.payload;
  payload->version = 0x00;
  payload->layers = 0x81; // Store in RAM layer.
  payload->_reserved0[0] = 0x00;
  payload->_reserved0[1] = 0x00;
  
  // Copy the configuration key.
  ptr_destination = (uint8_t*)&payload->config;
  ptr_source      = (uint8_t*)&config.key;
  for (uint8_t i = 0; i < sizeof(config.key); i++) {
    ptr_destination[i] = ptr_source[i];
  }

  // Copy the configuration value.
  ptr_destination += sizeof(config.key);
  ptr_source       = (uint8_t*)&config.value;
  for (uint8_t i = 0; i < config.value_size; i++) {
    ptr_destination[i] = ptr_source[i];
  }

  // Set length as required for CFG-VALSET [1, pp.55]
  frame.length = 4 + sizeof(config.key) + config.value_size;

  // Send configuration message.
  SAM_M10Q_Transmit(&frame);

  return true;
  
}





void
UBX_Configure_Default() {

  UBX_Configuration_t configurations[] = {
    // Startup enabled messages we wish to disable.
    { CFG_MSGOUT_NMEA_ID_GGA_I2C   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GGA_SPI   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GGA_UART1 , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GLL_I2C   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GLL_SPI   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GLL_UART1 , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GSA_I2C   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GSA_SPI   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GSA_UART1 , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GSV_I2C   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GSV_SPI   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_GSV_UART1 , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_RMC_I2C   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_RMC_SPI   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_RMC_UART1 , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_VTG_I2C   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_VTG_SPI   , 0 , 1 },
    { CFG_MSGOUT_NMEA_ID_VTG_UART1 , 0 , 1 },
    // Messages we wish to enable.
    { CFG_MSGOUT_UBX_NAV_POSLLH_UART1, 10, 1 },
  };

  // For all items in the configurations list,
  for (uint8_t i = 0; i < sizeof(configurations) / sizeof(UBX_Configuration_t); i++) {
    // Configure the respective option.
    UBX_Configure(configurations[i]);
  }
    

}
