# On This Document

Every todo item should contain a description, be given a justification,
especially if it concerns changes to the system rather than new features,
and any of the subgoals or relevant notes that may help a programmer.



# Goals - Passive

## Have well documented code.

- Refer to docs/code-style.txt, section 'Code Commenting' for more information
  on good comment style.

## Remove Doxygen sections.

- Doxygen was planned to be used, but was never applied in practice. Currently,
  Doxygen comment sections serve only as empty source padding.
- Australis is not sufficiently complex to require Doxygen.
- Doxygen is unlikely to be maintained by all programmers with the diligence
  necessary to make it useful.
- Tailored documentation better suits our bespoke system, and will be more
  helpful for new firmware developers.



# Goals - IREC Priority

## Fix the GPS telemetry failure.

- AV2 PCB seems fine, checks out with datasheet.
- SAM_M10Q driver seemed odd. Datasheets show different default UART baud rates
  which may have been the problem, but the old driver also depended on string
  processing that can be faulty. Changing to binary protocol format.
- SAM_M10Q may depend on RTC clock, which could be a problem in new AV2 boards.

## Update the RFM95 and SX1272 drivers.

- CRC should be enabled.
- Then ensure that positions are correct, because they look very wrong, espe-
  cially the RFM CRC being set in CONFIG2 rather than CONFIG1.
- Consider a total changeover to RFM95 chips; there are variants for both 433MHz
  and 915MHz.

##  Overhaul the logging system.

- Need to be able to do the following
  - Log all flight data, sensor readings as they happen, recovery event, etc.
  - Data as a binary form of time, key, and field.
  - A desktop tool to take data from the flash over USB into a binary blob.
  - A desktop tool for extracting data from the retrieved blob.

## Add a new state for landing.

- Detect when acceleration has been zero for a long time, so data collection
  can be halted.



# Goals - Short Term

## Add a task that spawns other tasks.

- Certain tasks such as the one which records data to flash, should not run
  until some condition is met, such as moving from prelaunch state.

## Fix the tilt angle failure.

- Not yet sure why tilt angle does not change. Could  be the mathematical
  implementation.
- Not priority if we are not using the aerobrakes system, which depends on tilt
  angle calculation.

## Deprecate the pub-sub system

- The pub-sub design was inappropiate for an MCU, relied on string processing
  which can be dangerous, and has not been shown to be functional.
- A similar function has been filled with the 'broadcast queue' data structures,
  thus pub-sub is unnecessary.
  
## Detect 'up' direction automatically on launch.

- A problem with Australis is that code must be changed to match orientation.
- Implement some layer on top of the axis that registers the sudden upwards
  acceleration as the 'up' direction.



# Goals - Long Term

## Move GPIO configuration into driver sections, change to subsystem initialization.

- Currently, it is clunky to create a new target. We are unlikely to change
  the AV2 MCU or the AV2 in general, as the price difference of a lower cost
  MCU is insignificant compared to the PCB expense.
  Moving all this boilerplate out of the target code will make it easier to make
  new targets, which is likely in the future if we want to create new targets,
  which we do.
- We should change to a more abstract 'subsystem' design, similar to the SDL
  library where individual systems can be brought up and down as needed.

## Create a new target that contains minimal code to make new targets from.

## Create a new target for unit testing.

- A unit testing target that aims to verify individual components of the
  Australis system. Essential to verify system operation, test individual
  peripherals, and move away from a clunky shell CLI system.
  However, it would be useful to have that for output.

## Create a new target for the n-modular implementation.

- This will be similar to the AV2-dual target, but will require also CAN comms
  with the boards, voting systems, multiple object files for all N computers.

## Improve rocketry state estimation.

- Compare current state estimation algorithm with literature.
- Consider failure cases more thoroughly, such as failure to reach apogee, and
  failure to reach sufficient thrust velocity off the pad.

## Start testing the capabilities of recovery charge deployment.

- Depends: improved rocketry state estimation, mentioned above.
- Do a fake deployment (set a flag in code) to see if recovery event matches
  with Telemega and Blue Raven recovery events.



# Changes for consideration

## Deprecation of DeviceList

- The purpose of DeviceList is unclear at the moment. Usage should be traced and
  then a decision made.

## Separation of data acquisition items.

- Segment low and high data acquisition sections into smaller chunks.
  The system is idle often enough that context switching will be insignificant.
- Use event group bits to synchronize state updates, depending on which systems
  are up.
- Ties into subsystem separation.

## Using the SAM_M10Q as another altitude measurement device.

- The same request that returns longitude/latitude data returns height above sea
  level. We can measure its performance against the barometric sensors.

## Add new inc/ files to hold AV2 specific macros for components.

- Depends on the long-term goal "Move GPIO configuration ...", low priority.
- AV2 specific code was intended to be held in the target files. This has made
  it difficult to setup new targets, but moving away from this would also mean
  we cannot pivot to a new flight computer. A solution would be to create new
  header files for everything in components, which describe their position on a
  board, if they are present.
- For example, in the SAM_M10Q driver section, its /inc folder could hold a file
  'sam_m10q.h', and also a 'sam_m10q_av2.h'. It can be conditionally included in
  the 'sam_m10q.h' file if an AV2 macro is defined.
- An extra benefit to this means that if a new board uses different chips for a
  function, it is easier to determine which sensors are used on which boards.
