/*
 * buttons.c
 *
 *  Created on: Sep 7, 2016
 *      Author: hornej2
 */

#include "buttons.h"
#include "xil_io.h"
#include "xparameters.h"
#include <stdio.h>
#include "supportFiles/display.h"

#define GPIO_DATA_OFFSET 0x0
#define GPIO_TRI_OFFSET 0x4
#define BUTTONS_GPIO_DEVICE_BASE_ADDRESS XPAR_PUSH_BUTTONS_BASEADDR
#define GPIO_VALUE 0xf
#define FONT_SIZE 2 //set font size
//these values are used to set the btn box displays
#define QUARTER_WIDTH DISPLAY_WIDTH/4
#define HALF_WIDTH DISPLAY_WIDTH/2
#define THREEQUARTER_WIDTH DISPLAY_WIDTH*3/4
#define SIXTEENTH_WIDTH DISPLAY_WIDTH/16
#define HALF_HEIGHT DISPLAY_HEIGHT/2
#define QUARTER_HEIGHT DISPLAY_HEIGHT/4-5 //almost one fourth from the top of screen

//read from the register
int32_t buttons_readGpioRegister(int32_t offset) {
//   Just use the low-level Xilinx call.
   return Xil_In32(BUTTONS_GPIO_DEVICE_BASE_ADDRESS + offset);
}

//write to the register
void buttons_writeGpioRegister(int32_t offset){
    return Xil_Out32(BUTTONS_GPIO_DEVICE_BASE_ADDRESS + offset,GPIO_VALUE);
}

// Initializes the button driver software and hardware. Returns one of the defined status values (above).

int32_t buttons_init(){
    buttons_writeGpioRegister(GPIO_TRI_OFFSET);
    display_init(); //initialize display
    return BUTTONS_INIT_STATUS_OK;
}

// Returns the current value of all 4 buttons as the lower 4 bits of the returned value.
// bit3 = BTN3, bit2 = BTN2, bit1 = BTN1, bit0 = BTN0.
int32_t buttons_read(){
   return buttons_readGpioRegister(GPIO_DATA_OFFSET);
}

// Runs a test of the buttons. As you push the buttons, graphics and messages will be written to the LCD
// panel. The test will until all 4 pushbuttons are simultaneously pressed.
void buttons_runTest(){\
    buttons_init();
    display_fillScreen(DISPLAY_BLACK);
    bool btn0_flag = false; // initialize btn flags.
    bool btn1_flag = false;   //these allow for the btn box display to turn off after btn is released
    bool btn2_flag = false;
    bool btn3_flag = false;
    //This while loop will continue to run until all buttons are pressed at once
    //It will display the btns on the screen when pressed
    while(buttons_read()!=GPIO_VALUE){
                //********************************
                //*********BUTTON 0 LOGIC*********
                //********************************
       if(buttons_read()&BUTTONS_BTN0_MASK&&!btn0_flag){ //turn on btn0 box display
           //this will create a yellow box in the far right hand side with text "BTN0"
           display_fillRect(THREEQUARTER_WIDTH,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_YELLOW);
           display_setCursor(SIXTEENTH_WIDTH+THREEQUARTER_WIDTH,QUARTER_HEIGHT);
           display_setTextColor(DISPLAY_BLACK); //this btn has black font
           display_setTextSize(FONT_SIZE);
           display_println("BTN0");//btn0 text
           btn0_flag=true; //raise btn0 flag
       }
       else if(btn0_flag&&!(buttons_read()&BUTTONS_BTN0_MASK)){ //black off btn0 box display
           display_fillRect(THREEQUARTER_WIDTH,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_BLACK);
           btn0_flag=false; //lower btn0 flag
       }
               //********************************
               //*********BUTTON 1 LOGIC*********
               //********************************
       if(buttons_read()&BUTTONS_BTN1_MASK&&!btn1_flag){ //turn on btn1 box display
           //this will create a cyan box in the right middle with text "BTN1"
           display_fillRect(HALF_WIDTH,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_CYAN);
           display_setCursor(SIXTEENTH_WIDTH+HALF_WIDTH,QUARTER_HEIGHT);
           display_setTextColor(DISPLAY_BLACK); //this btn has black font
           display_setTextSize(FONT_SIZE);
           display_println("BTN1");//btn1 text
           btn1_flag=true; //raise btn1 flag
       }
       else if(btn1_flag&&!(buttons_read()&BUTTONS_BTN1_MASK)){ //black off btn1 box display
           display_fillRect(HALF_WIDTH,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_BLACK);
           btn1_flag=false; //lower btn1 flag
       }
               //********************************
               //*********BUTTON 2 LOGIC*********
               //********************************
       if(buttons_read()&BUTTONS_BTN2_MASK&&!btn2_flag){ //turn on btn2 box display
           //this will create a red box in the left middle with text "BTN2"
           display_fillRect(QUARTER_WIDTH,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_RED);
           display_setCursor(SIXTEENTH_WIDTH+QUARTER_WIDTH,QUARTER_HEIGHT);
           display_setTextColor(DISPLAY_WHITE); //this btn has white font
           display_setTextSize(FONT_SIZE);
           display_println("BTN2");//btn2 text
           btn2_flag=true; //raise btn2 flag
       }
       else if(btn2_flag&&!(buttons_read()&BUTTONS_BTN2_MASK)){ //black off btn2 box display
           display_fillRect(QUARTER_WIDTH,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_BLACK);
           btn2_flag=false; //lower btn2 flag
       }
               //********************************
               //*********BUTTON 3 LOGIC*********
               //********************************
       if(buttons_read()&BUTTONS_BTN3_MASK&&!btn3_flag){ //turn on btn3 box display
           //this will create a blue box in the far left side with text "BTN3"
           display_fillRect(0,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_BLUE);
           display_setCursor(SIXTEENTH_WIDTH,QUARTER_HEIGHT);
           display_setTextColor(DISPLAY_WHITE); //this btn has white font
           display_setTextSize(FONT_SIZE);
           display_println("BTN3");//btn3 text
           btn3_flag=true; //raise btn3 flag
       }
       else if(btn3_flag&&!(buttons_read()&BUTTONS_BTN3_MASK)){ //black off btn3 box display
           display_fillRect(0,0,QUARTER_WIDTH,HALF_HEIGHT,DISPLAY_BLACK);
           btn3_flag=false; //lower btn3 flag
       }
    }
    display_fillScreen(DISPLAY_BLACK); //show screen black at end of test
}
