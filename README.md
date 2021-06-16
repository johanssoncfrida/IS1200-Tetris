# Mini Project - Tetris (advanced project)

Assignment in course IS1200- Datateknik

The project should be done on a [ChipKIT Uno32](http://www.digilentinc.com/Products/Detail.cfm?Prod=CHIPKIT-UNO32) or a [ChipKIT uC32](http://store.digilentinc.com/chipkit-uc32-basic-microcontroller-board-with-uno-r3-headers/) development board together with a [ChipKIT Basic I/O shield](http://www.digilentinc.com/Products/Detail.cfm?&Prod=CHIPKIT-BASIC-IO-SHIELD) (see manuals on the Literature and Resources page) or other electronics that are connected to the board. 

### Requirements of the project


- You must use the PIC32 platform supplied by the course. Note that if you choose to include other electronic components, it is OK to use these components directly with the Uno32 board and remove the Basic I/O shield. However, you do not have to buy extra electronics; it is enough to use the Basic I/O shield as long as you fulfill the next requirement.
- The project should make use of some other device/component besides the LED lights, switches, and push buttons on the Basic I/O shield. This could for example be the temperature sensor, the display on the I/O board, an external accelerometer, an external motor etc. 
- The project should be non trivial and actually perform something. This means that there should typically be some interaction, use of I/O devices, and include some program logic. 
- You may use the MCB32 tool chain or the MPLAB X IDE, but not the MPIDE (a ported Arduino IDE). The reason is that you should learn about low level programming in this project.

To be able to counted as an advanced project, one of these requirements must be fulfilled. 

1. (Fulfilled) You create some project with external electronic components that are communicating over SPI, I2C, or some other non-trivial data protocol. Your application has a non-trivial logic.
2. You implement a program or a game using the basic I/O shield using the OLED graphical display, where you have graphics moving around on the whole screen, both in X and Y direction. Your program must interact with the whole screen pixel-by-pixel and not just using the image and text example code from the labs (graphical objects are moving over pages/segments of the screen). This means that you need to have at least one object of the minimal size of 2x2 pixels (for instance a small image) that can move in both X and Y direction, one pixel in each frame update. The game/program should also be rather advanced, with several modes (1 and 2 players, high score list, etc.). See the examples below.
3. Your program has an advanced interaction with sound, for instance by sampling sound as input, manipulating the sound signal, and outputting the sound again.

### Development Environment

The [MCB32 toolchain](https://github.com/is1200-example-projects/mcb32tools/releases/) is developed specifically for this course. All the labs in the course are using this environment. The main benefits with this environment are i) that it is completely command line based, ii) it does not hide hardware details, and iii) it does not need a PICkit programmer, as the MPLAB X IDE requires. This is the environment that most students will use in their projects. 
