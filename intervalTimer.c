/*
 * intervalTimer.c
 *
 *  Created on: Sep 15, 2016
 *      Author: hornej2
 */

#include <stdio.h>
#include "intervalTimer.h"
#include "xparameters.h"
#include "xil_io.h"
#include <inttypes.h>

// this control/status register is used to
//control the cascaded 64-bit counter and to load values into the 32-bit counter.
#define TCSR0 0x00 //control status register 0
//you will need to set both 32-bit counters to zero in order to reset the timer.
//You do this for counter 0 by loading a 0 into this register and loading the
//contents of TLR0 into the counter.
#define TLR0 0x04 //load register 0
//you read this register to find out the current value of counter 0.
#define TCR0 0x08 //timer/counter register 0
//you will use this control/status register only to load a 0 into counter 1 to reset it.
#define TCSR1 0x10 //control/status register 1
// you load a 0 into this and load the contents of TLR1 into counter 1 to reset it
//and set it to 0 (just like counter 0).
#define TLR1 0x14 //load register 1
//you read this register to find out the current value of counter 1.
#define TCR1 0x18 //timer/counter register 1
#define SET 1//ONE
#define CLEAR 0 //ZERO
#define ENT0 0x80 //ENT0
#define ENT1 0x80 //ENT1
#define CASC 0x800 //CASC
#define LOAD0 0x20 //LOAD0
#define LOAD1 0x20 //LOAD1
#define UDT0 0x2 //UDT0
#define UDT1 0x2 //UDT1
/* Definitions for peripheral AXI_TIMER_0 */
#define TIMER0_BASEADDR XPAR_AXI_TIMER_0_BASEADDR
/* Definitions for peripheral AXI_TIMER_1 */
#define TIMER1_BASEADDR XPAR_AXI_TIMER_1_BASEADDR
/* Definitions for peripheral AXI_TIMER_2 */
#define TIMER2_BASEADDR XPAR_AXI_TIMER_2_BASEADDR
// Definition for Timers 0, 1, and 2 frequency
#define TIMER_CLOCK_PERIOD 1/XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ
#define THIRTY_TWO 32
#define FORCE_DOUBLE 1.0

//read from the General Purpose I/O register
uint32_t timer_readGpioRegister(int32_t baseaddr,int32_t offset) {
    return Xil_In32(baseaddr + offset);
}

//write to the General Purpose I/O register
void timer_writeGpioRegister(int32_t baseaddr,int32_t offset,int32_t value){
    return Xil_Out32(baseaddr + offset,value);
}

//this function will take the timer number and return the respective base address
uint32_t whatTimer(uint32_t timerNumber){
    if (timerNumber == INTERVAL_TIMER_TIMER_0){
        return TIMER0_BASEADDR;
    }
    if (timerNumber == INTERVAL_TIMER_TIMER_1){
        return TIMER1_BASEADDR;
    }
    if (timerNumber == INTERVAL_TIMER_TIMER_2){
        return TIMER2_BASEADDR;
    }
}

// You must initialize the timers before you use them.
// timerNumber indicates which timer should be initialized.
// returns INTERVAL_TIMER_STATUS_OK if successful, some other value otherwise.
intervalTimer_status_t intervalTimer_init(uint32_t timerNumber){
    timer_writeGpioRegister(whatTimer(timerNumber),TCSR0,CLEAR);
    timer_writeGpioRegister(whatTimer(timerNumber),TCSR1,CLEAR);
    timer_writeGpioRegister(whatTimer(timerNumber),TCSR0,CASC);
    return INTERVAL_TIMER_STATUS_OK;
}

// This is a convenience function that initializes all interval timers.
// Simply calls intervalTimer_init() on all timers.
// returns INTERVAL_TIMER_STATUS_OK if successful, some other value otherwise.
intervalTimer_status_t intervalTimer_initAll(){
    intervalTimer_init(INTERVAL_TIMER_TIMER_0);
    intervalTimer_init(INTERVAL_TIMER_TIMER_1);
    intervalTimer_init(INTERVAL_TIMER_TIMER_2);
    return INTERVAL_TIMER_STATUS_OK;
}

// This function starts the interval timer running.
// timerNumber indicates which timer should start running.
void intervalTimer_start(uint32_t timerNumber){
    timer_writeGpioRegister(whatTimer(timerNumber),TCSR0,ENT0|timer_readGpioRegister(whatTimer(timerNumber),TCSR0));
}

// This function stops the interval timer running.
// timerNumber indicates which timer should stop running.
void intervalTimer_stop(uint32_t timerNumber){
    timer_writeGpioRegister(whatTimer(timerNumber),TCSR0,~ENT0&timer_readGpioRegister(whatTimer(timerNumber),TCSR0));
}

// This function resets the interval timer.
// timerNumber indicates which timer should reset.
void intervalTimer_reset(uint32_t timerNumber){
    timer_writeGpioRegister(whatTimer(timerNumber),TLR0,CLEAR); //write a 0 into the TLR0 register.
    //write a 1 into the LOAD0 bit in the TCSR0.
    timer_writeGpioRegister(whatTimer(timerNumber),TCSR0,LOAD0|timer_readGpioRegister(whatTimer(timerNumber),TCSR0));
    timer_writeGpioRegister(whatTimer(timerNumber),TLR1,CLEAR); //write a 0 into the TLR1 register.
    //write a 1 into the LOAD1 bit of the TCSR1 register.
    timer_writeGpioRegister(whatTimer(timerNumber),TCSR1,LOAD1|timer_readGpioRegister(whatTimer(timerNumber),TCSR1));
    intervalTimer_init(timerNumber); //initialize the timer
}

// Convenience function for intervalTimer_reset().
// Simply calls intervalTimer_reset() on all timers.
void intervalTimer_resetAll(){
    intervalTimer_reset(INTERVAL_TIMER_TIMER_0); //reset timer 0
    intervalTimer_reset(INTERVAL_TIMER_TIMER_1); //reset timer 1
    intervalTimer_reset(INTERVAL_TIMER_TIMER_2); //reset timer 2
    intervalTimer_initAll(); //initialize all the timers
}

// Once the interval timer has stopped running, use this function to
// ascertain how long the timer was running.
// The timerNumber argument determines which timer is read.
double intervalTimer_getTotalDurationInSeconds(uint32_t timerNumber){
    /*
            The following are the steps for reading the 64-bit counter/timer:
        1. Read the upper 32-bit timer/counter register (TCR1).
        2. Read the lower 32-bit timer/counter register (TCR0).
        3. Read the upper 32-bit timer/counter register (TCR1) again. If the value is different from the 32-bit upper value
        read previously, go back to previous step (reading TCR0). Otherwise 64-bit timer counter value is correct.
     */
    uint64_t totalcnt; //the 64 bit number that holds the total count
    double duration; //the number in seconds we will be returning
    uint32_t Upper1 = timer_readGpioRegister(whatTimer(timerNumber),TCR1); //step 1.
    uint32_t Lower = timer_readGpioRegister(whatTimer(timerNumber),TCR0); //step 2.
    uint32_t Upper2 = timer_readGpioRegister(whatTimer(timerNumber),TCR1); //step 3.
    if(Upper1==Upper2){ //if first TCR1 reading equals second TCR1 reading
        totalcnt = Upper1; //read the first 32 bits in
        totalcnt = (totalcnt<<THIRTY_TWO); //move upper1 to the upper half of the 64 bit number
        totalcnt = totalcnt|Lower; //add the lower 32 bits
        duration = FORCE_DOUBLE*totalcnt*TIMER_CLOCK_PERIOD; //turn the totalcnt to seconds
        return duration;
    }
    else{ //if first TCR1 reading doesn't equal second TCR1 reading then reread TCR0
        Lower = timer_readGpioRegister(whatTimer(timerNumber),TCR0);
        totalcnt = Upper2; //read the first 32 bits in
        totalcnt = (totalcnt<<THIRTY_TWO); //move upper2 to the upper half of the 64 bit number
        totalcnt = totalcnt|Lower; //add the lower 32 bits
        duration = FORCE_DOUBLE*totalcnt*TIMER_CLOCK_PERIOD; //turn the totalcnt to seconds
        return duration;
    }
}


