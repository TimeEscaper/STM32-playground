# STM32-playground
Repository contains different examples for STM32 platform (STM32F4DISCOVERY board). Each example is contained in separate branch.
All examples use HAL library. Initial code is generated with STM32CubeMx for System Workbench for STM32 (SW4STM).

This is an example of using I2C and UART on FreeRTOS in separate tasks. I2C and UART are used in DMA mode. STM32F4DISCOVERY board acts as a master, Arduino Nano v3 acts as a slave.
