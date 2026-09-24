# F411 Table Clock

An embedded table clock built around the STM32F411CEU6 microcontroller. The firmware drives a 16x2 HD44780-compatible character LCD, keeps time with the STM32 RTC and an external 32.768 kHz crystal, and provides a button-operated menu for configuring the clock.

## Project Status

The core clock, date and time editing, 12/24-hour display format, menu timeout, LCD backlight control, and low-power sleep between minute updates are implemented.

The `Alarm`, `Stop Watch`, and `Timer` menu entries are reserved for future functionality and are not currently implemented.

## Hardware

- **MCU:** STM32F411CEU6, UFQFPN48 package
- **Display:** 16x2 HD44780-compatible LCD in 4-bit parallel mode
- **Time base:** RTC clocked from an external 32.768 kHz LSE crystal
- **Input:** Four active-low push buttons with falling-edge EXTI interrupts
- **Programming/debugging:** SWD through an ST-LINK probe
- **Firmware supply assumption:** 3.3 V logic

### LCD Wiring

| LCD signal | STM32F411 pin |
| --- | --- |
| RS | PA0 |
| EN | PA1 |
| D4 | PA2 |
| D5 | PA3 |
| D6 | PA4 |
| D7 | PA5 |
| Backlight control | PB9 |

Connect the LCD power, contrast potentiometer, and ground according to the LCD module datasheet. The firmware controls the backlight on PB9.

### Button Wiring

| Button | STM32F411 pin | Active state |
| --- | --- | --- |
| UP | PB5 | Low |
| BACK | PB6 | Low |
| DOWN | PB7 | Low |
| ENTER | PB8 | Low |

The button inputs are configured for falling-edge interrupts. Provide suitable external pull-ups or a board-level input bias because the firmware configures these pins without an internal pull resistor.

### Board and Schematic Reference

Project repository and board reference:

<https://github.com/DRIFTYY777/F411_TableClock>

The local Git metadata supplied for this project is located at `c:\Users\dhima\Downloads\New folder\F411_TableClock\.git`. No schematic or PCB source files are included in the current firmware workspace checkout.

## User Interface

| Action | Function |
| --- | --- |
| Long-press ENTER from the home screen | Open the main menu |
| UP / DOWN | Move through menu items or change the selected value |
| ENTER | Select a menu item or advance to the next editable field |
| BACK | Return to the previous field or screen |

The home screen displays the current time and date. The menu backlight is enabled while a menu screen is active and the menu returns to the home screen after 30 seconds without input.

## Firmware Behavior

- The RTC retains the selected 12/24-hour format in an RTC backup register.
- Date and time are edited field by field through the LCD menu.
- The home screen displays the weekday, date, and current time.
- While the home screen is active, the MCU enters STOP mode and wakes at the next minute boundary or when ENTER is pressed.
- After STOP mode, the system clock is restored before normal processing resumes.

## Software Requirements

- STM32CubeIDE
- STM32Cube firmware package for STM32F4, version 1.28.3 or compatible
- GNU Tools for STM32 / `arm-none-eabi-gcc` toolchain
- ST-LINK or another compatible SWD programmer/debugger

The project was generated for the STM32CubeIDE GCC toolchain and uses the `F411_TableClock.ioc` configuration file.

## Build

### STM32CubeIDE

1. Import the project into STM32CubeIDE.
2. Select the `F411_TableClock` project.
3. Build the Debug configuration.
4. The generated ELF and HEX files are placed in `Debug/`.

### Command line

From the `Debug` directory, with the ARM GNU toolchain and `make` available on `PATH`:

```sh
make
```

The build produces `F411_TableClock.elf`, `F411_TableClock.hex`, and the corresponding map/list files.

## Flash and Debug

1. Connect an ST-LINK probe to the board's SWDIO, SWCLK, 3.3 V, and GND signals.
2. Build the project in STM32CubeIDE.
3. Use the included `F411_TableClock Debug.launch` configuration to program and debug the target.

The debug configuration uses SWD, verifies the flash download, and stops at `main`.

## Project Layout

```text
Core/Inc/       Application headers and pin definitions
Core/Src/       Application, LCD, input, and UI implementation
Core/Startup/   STM32F411 startup assembly
Drivers/        CMSIS and STM32F4 HAL sources
Debug/          STM32CubeIDE generated build output and makefiles
*.ioc           STM32CubeMX/CubeIDE peripheral configuration
*.ld            Flash and RAM linker scripts
```

## License

No project-specific license file is currently included. The STM32 HAL and CMSIS components retain their respective upstream license files.