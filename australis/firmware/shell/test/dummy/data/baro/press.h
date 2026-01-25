#ifndef PRESS_H
#define PRESS_H

#define PRESS_TERMINATION 0x00000000
#define PRESS_START       0x00000000
#define PRESS_FINISH      0x00003A5C
#define PRESS_LENGTH      0x00003A5C

extern const unsigned long press_termination;
extern const unsigned long press_start;
extern const unsigned long press_finish;
extern const unsigned long press_length;
extern const unsigned short press[];

#endif /* PRESS_H */
