Feel free to add new words to this glossary if you found they took some time to
understand. There are some jargon words that have come into this project that
need to be clarified.



# Glossary


### AV2

Strictly refers to the PCB which *Australis* currently targets.

### Australis

Strictly refers to this firmware project. It is not necessarily for *AV2*, but at
the moment this is the case.

### Broadcast Queue

The broadcast queue (here referred to as bqueue) is an data structure defined
by Australis. It consists of a bqueue, which can have bqueue subscribers. When
a message is broadcast for a bqueue, every subscriber will receive a copy. It is
based on the FreeRTOS queue structure.

### BQueue

See *Broadcast Queue*.

### CAN

CAN stands for Controller Area Network. It is a communication protocol where
there is one 'bus' which many devices can listen to, and messages are broadcast
such that any listening device can receive it. A CAN packet has an ID; this does
not identify the device, but the kind of data being sent. A CAN packet can hold
between one and eight bytes of data.

### Target
