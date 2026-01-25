<!-- Document Start -->

![](https://github.com/RMIT-Hive-Rocketry/Australis-Avionics-firmware/raw/master/img/banner.png)

<div align="center">

## FreeRTOS Based Firmware For Australis Series Flight Computers

[![Static Badge](https://img.shields.io/badge/documentation%20-%20Wiki%20-%20dodgerblue?style=flat&logo=gitbook)](https://github.com/s3785111/Australis-Avionics-firmware/wiki)
[![Static Badge](https://img.shields.io/badge/documentation%20-%20API%20Reference%20-%20dodgerblue?style=flat&logo=gitbook)](https://rmit-competition-rocketry.github.io/Australis-Avionics-firmware/)
[![Static Badge](https://img.shields.io/badge/documentation%20-%20Hardware%20%28Members%20Only%29%20-%20dodgerblue?style=flat&logo=gitbook)](https://github.com/RMIT-Competition-Rocketry/AV2-Hardware)

[![Static Badge](https://img.shields.io/badge/%20-%20projects%20-%20grey?style=flat&logo=git&logoColor=white)](https://github.com/s3785111/Australis-Avionics-firmware/projects?query=is%3Aopen)
[![Static Badge](https://img.shields.io/badge/%20-%20pull%20requests%20-%20grey?style=flat&logo=github&logoColor=white)](https://github.com/s3785111/Australis-Avionics-firmware/pulls)

<details>
<summary>Want to submit an issue?</summary>
<p float='right'><br>
<a href='https://github.com/s3785111/Australis-Avionics-firmware/issues/new?template=bug_report.md'><img src='https://img.shields.io/badge/report%20issue%20-%20Bug%20-%20tomato?style=flat'/></a>
<a href='https://github.com/RMIT-Competition-Rocketry/Australis-Avionics-firmware/issues/new?template=feature-proposal.md'><img src='https://img.shields.io/badge/report%20issue%20-%20Feature%20-%20tomato?style=flat'/></a>
<a href='https://github.com/RMIT-Competition-Rocketry/Australis-Avionics-firmware/issues/new?template=refactor-declaration.md'><img src='https://img.shields.io/badge/report%20issue%20-%20Refactor%20-%20tomato?style=flat'/></a>
</p> 
</details>

---

_Operating avionics hardware for winning rocket of Technical Excellence Award and runner's up overall in AURC 2024_ - Aurora V

_Powering IREC & AURC 2025 entries for avionics subsystem_ - Legacy III

</div>

> [!WARNING]
> The firmware system and this repository are still under active development. Documentation and source code are not yet considered stable.
>
> Pre-release builds are being made available as progress is made, however the user should be aware that there may be undocumented or unfinished features in some parts of the code.

## Overview

This repository contains the firmware for the Australis series flight computers, developed using FreeRTOS. The project includes source code, build tooling, and documentation to facilitate development and deployment.

<div align="center">
 
![Australis](https://github.com/RMIT-Hive-Rocketry/Australis-Avionics-firmware/raw/master/img/Australis.png)
 
Australis version 2 (_**AV2**_) flight-computer hardware platform
</div>

> [!NOTE]
> The firmware is designed to run on various hardware platforms, with explicit support for `STM32F439` microcontrollers, however the _**AV2**_ system (pictured above) is the basis for implementation. While other platforms are supported unofficially as extended targets, correct operation is not guaranteed.

## Table of Contents

1. [Getting Started](#getting-started)
   - [Docker Environment](#docker-environment)
   - [Documentation](#documentation)
2. [FAQ](#faq)
3. [Contributions](#contributions)
4. [Acknowledgements](#acknowledgements)

## Getting Started

For a detailed guide to get started with the project, follow the instructions in the [Firmware README](firmware/README.md). This includes information on setting up the build environment, building the firmware, and running it on supported hardware platforms.

### Docker Environment

This repository provides a containerised development environment to simplify the process of setting the project up on different systems and platforms. To make use of this environment, first ensure `Docker` is installed and running on your system and then follow these steps to get started:

1. If you haven't already, clone this repository to your local system:

   ```bash
   git clone https://github.com/RMIT-Competition-Rocketry/Australis-Avionics-firmware.git
   ```

2. Navigate to the repository root directory:

   ```bash
   cd /path/to/Australis-Avionics-firmware
   ```

3. Inside the root directory, run the following command. **This may take a couple of minutes to finish when running for the first time**:

   ```bash
   docker compose run --rm australis-dev
   ```

   This will build the image, if not already available, and enter a running container in the project root: `/firmware` (`/src/firmware` on the container's filesystem).

   The `--rm` flag deletes the container on exit, this is desirable as the container mounts the repository in the host filesystem into the running container, enabling editing on the host while using the container to manage building and deployment.

4. Follow the steps outlined in the [Firmware README](firmware/README.md) to build and deploy the project.

#### Common Issues

> The `/src` directory inside the container is empty/unavailable

Consider investigating the [file sharing configuration](https://docs.docker.com/desktop/settings-and-maintenance/settings/#file-sharing) of your Docker installation.

> I followed the steps but the container is taking a long time to start

Configuring the Docker environment can take a bit of time on the first run as it needs to build the image before creating the container. Once the image is built, the container should be much faster to start.

### Documentation

For detailed information about the project, including build instructions, code guidelines, and hardware targets, refer to the following resources:

- [Project Wiki](https://github.com/s3785111/Australis-Avionics-firmware/wiki): Comprehensive documentation about the project.
- [API Reference](https://rmit-competition-rocketry.github.io/Australis-Avionics-firmware/): Detailed API documentation for the project.
- [Firmware README](firmware/README.md): Detailed information about the firmware source structure, build process, and target platforms.

## FAQ

> Q: What is Australis?

Australis is a combined firmware and hardware platform designed for high degrees of flexibility and redundancy in its implementation. The system supports a core architecture that provides all the tools necessary for creating and operating a complete flight computer for any rocket configuration.

> Q: What makes the system flexible?

Australis sports various hardware interconnects and a range of highly modular firmware systems provided to enable your designs. _**AV2**_ boards each provide two directly connected microcontrollers, and may be interconnected with other boards, primarily via `CAN`.

> Q: What makes the system redundant?

Redundant apogee detection and recovery deployment is currently under development for official firmware targetting _**AV2**_. The approach follows a _triple-mode-redundant_ (TMR) architecture designed for a dual-board setup with two stages of redundancy:

1. In hardware, one microcontroller on the bus is dedicated the role of **arbiter**. The **arbiter** is in charge of collating "votes" from the other three microcontrollers, and when it determines a majority vote it activates the recovery output interlocks and detonates the energetics
2. Each remaining microcontroller provides the role of **voter**, maintaining an active estimate of their flight-state dynamics. The physical system state is considered a global input to each **voter**, where the processed state variables determine the input redundancy stage through a "vote" on the output when at least two of the following three conditions are met:
   - _Vertical velocity (ground axis referenced) is negative_
   - _Barometric pressure is increasing_
   - _Tilt angle (ground axis referenced) is greater than 90°_

While designed for dual-board systems for a TMR configuration, the level of _N-Modular Redundancy_ is highly flexible and can easily be extended for greater values of _N_. Additional redundancy is also being considered for support as _flexible redundancy_, where a TMR configuration could fall-back to other forms of redundancy such as _hot-standby_ in the event of a number of failures that cannot be masked in the implemented redundancy configuration by _N_ modules.

## Contributions

Contributions to the project are welcome. Please refer to the [contributing guidelines](firmware/README.md#contributing) in the firmware README for more information on how to contribute.

## Acknowledgements

### Key Contributors

| Name              | Role                           |
| :---------------- | :----------------------------- |
| Matthew Ricci     | Principal firmware developer   |
| William Houlahan  | Initial driver implementations |
| Benjamin Wilsmore | Initial driver implementations |

### Special Thanks

Other members of the Aurora V Avionics team:

- Hugo Begg - _avionics hardware_
- Jonathan Chandler - _ground control station_
- Jeremy Timotius - _data analysis_
- Lucas Webb - _ground control station_

Thank you to Aurora & Legacy project team leads Patrick Underwood and Brodie Alexander for providing the opportunity and environment to work on these rockets as part of the team, and thank you to everyone who helped make them a reality!

![](./img/footer.png)
