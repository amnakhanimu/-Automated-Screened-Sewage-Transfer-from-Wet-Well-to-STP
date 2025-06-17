#include <xc.h>

// Define clock frequency for __delay_ms
#define _XTAL_FREQ 4000000  // 20 MHz Crystal Oscillator


#define TankB_L RC0
//#define TankB_M RC1
#define TankB_H RC2 
#define Y_L RC3
#define Y_M RD0
#define Y_H RD1

#define R_E RD2
#define R_H RD3
#define R_L RC4
#define TankA_E RC5
#define TankA_H RC6
#define TankA_L RC7

#define VAL RB7

#define P_1 RB5
#define P_2 RD7

#define BUZB RD5


//#define timeout0_s 5                //MCU will buzz BuzzerB for 5 seconds after water reaches H of TankB.
//#define timeout0_ms (timeout0_s*1000)
//#define tmr0_target (timeout0_ms/65)//CHANGE TO 262 and check!!!!!!!!!! "Change 131 to 4.1 in the calculation"  //Is Timer0's overflow target.
//                                     //  Dividing  by 65.5ms becuz Timer0 overflows in approx. 65.5ms (becuz pre-scalar is set to 1:256 later).


// Configuration bits
#pragma config FOSC = HS     // High-Speed Oscillator
#pragma config WDTE = OFF    // Watchdog Timer Disabled
#pragma config PWRTE = ON    // Power-up Timer Enabled
#pragma config BOREN = ON    // Brown-out Reset Enabled
#pragma config LVP = OFF     // Low Voltage Programming Disabled

volatile long tmr0_overflow_count = 0;
//volatile unsigned char tmr0_target_reached = 0;
volatile unsigned char tmr0_active = 0;

void initializingPorts(void);
void initializingTimer0(void);
void initializeInterrupts(void);
void updatingLEDs(void);
void controllingValve(void);
void controllingPumps(void);
void controllingBuzzerB(void);

void initializingPorts(void){
    TRISCbits.TRISC0 = 1; //TankB_L
//    TRISCbits.TRISC1 = 1; //TankB_M
    TRISCbits.TRISC2 = 1; //TankB_H
    TRISCbits.TRISC3 = 0; //Y_L
    TRISDbits.TRISD0 = 0; //Y_M
    TRISDbits.TRISD1 = 0; //Y_H
    PORTCbits.RC3 = 0; //clearing bit for safety
    PORTDbits.RD0 = 0; //clearing bit for safety
    PORTDbits.RD1 = 0; //clearing bit for safety
    
    TRISDbits.TRISD2 = 0; //R_E
    TRISDbits.TRISD3 = 0; //R_M
    TRISCbits.TRISC4 = 0; //R_L
    TRISCbits.TRISC5 = 1; //TankA_E
    TRISCbits.TRISC6 = 1; //TankA_H
    TRISCbits.TRISC7 = 1; //TankA_L
    PORTDbits.RD2 = 0; //clearing bit for safety
    PORTDbits.RD3 = 0; //clearing bit for safety
    PORTCbits.RC4 = 0; //clearing bit for safety
    
    TRISBbits.TRISB7 = 0; //VAL
    PORTBbits.RB7 = 0;    //  clearing bit for safety
    
    TRISBbits.TRISB5 = 0; //P_1
    PORTBbits.RB5 = 0;
    TRISDbits.TRISD7 = 0; //P_2
    PORTDbits.RD7 = 0;
    
    TRISDbits.TRISD5 = 0; //BUZ2
    PORTDbits.RD5 = 0;
}
void initializingTimer0(void) {
    OPTION_REGbits.T0CS = 0; //TMR0 will use the Internal Instruction Cycle Clock as the clock source 
    OPTION_REGbits.PSA = 0;  //assigns Pre-Scalar to TMR0
    OPTION_REGbits.PS2 = 1;  //sets the pre-scalar to 1:256
    OPTION_REGbits.PS1 = 1;  //sets the pre-scalar to 1:256
    OPTION_REGbits.PS0 = 1;  //sets the pre-scalar to 1:256

    INTCONbits.TMR0IE = 1; //enables Timer0 interrupt.
    INTCONbits.TMR0IF = 0; //clears Timer0 interrupt flag.
    TMR0 = 0; //clears the Timer0 register.

}
void __interrupt() ISR(void){
    // When Timer0 Overflows:
    if (INTCONbits.TMR0IF){     //checks if the flag of TMR0 is 1 (i.e. has it overflowed?):
        INTCONbits.TMR0IF = 0;  //sets tmr0 flag to 0.
        tmr0_overflow_count++;
        
        if(tmr0_overflow_count >=76){
            BUZB = 0;
//            tmr0_target_reached = 1;
            tmr0_overflow_count = 0;
            tmr0_active = 0;
            INTCONbits.TMR0IE = 0;
        }
    }
}
void initializeInterrupts(void) {        
    INTCONbits.GIE = 1;   //enables the Global Interrupt.
    INTCONbits.PEIE = 1;  //enables the Peripheral Interrupts.
    
    INTCONbits.INTF = 0; //clears the External Interrupt flag.
    INTCONbits.INTE = 1; //enables the External Interrupt.

    OPTION_REGbits.INTEDG = 0; //clears the INTerrupt EDGge pin (for EXT INT))
                               //  i.e. the Interrupt is triggered on Falling edge
                               //  i.e. the Interrupt will happen when the change is from HIGHT to LOW.
    
}
void updatingLEDs(void){
    if(TankB_L ==1){
        Y_L = 1;
    }else{
        Y_L = 0;
    }
    
//    if(TankB_M == 1){
//        Y_M = 1;
//    }else{
//        Y_M = 0;
//    }
    
    if(TankB_H == 1){
        Y_H = 1;
    }else{
        Y_H = 0;
    }
    ////
    ////
    if(TankA_E == 1){
        R_E = 1;
    }
    else{
        R_E = 0;
    }
    
    if(TankA_H == 1){
        R_H = 1;
    }else{
        R_H = 0;
    }
    
    if(TankA_L == 1){
        R_L = 1;
    }else{
        R_L = 0;
    }
}
void controllingValve(void){
    if(TankA_H ==1 || TankA_E ==1){
        VAL = 1; //Valve OFF
    }else{
        VAL = 0; //Valve ON
    }
}
void controllingPumps(void){
//    if (TankB_H ==1 && TankB_M ==1 && TankB_L ==1){
//        P_1 = 1; //OFF
//        P_2 = 1; //OFF
//    } else if (TankB_H == 0 && TankB_M == 1 && TankB_L == 1){
//            P_1 = 0; //ON
//            P_2 = 1; //OFF
//    } else if (TankB_H == 0 && TankB_M ==0 && TankB_L ==1){
//        P_1 = 0; //OFF
//        P_2 = 0; //OFF
//    } else if (TankB_H == 0 && TankB_M ==0 && TankB_L ==0){
//        P_1 = 0; //OFF
//        P_2 = 0; //OFF
//        }
//    }
    
//    //SIR'S CODE:
//    if (TankB_H ==1){
//        P_1 = 1; //OFF
//        P_2 = 1; //OFF
//    }
//    else{
//        if (TankB_H == 0 && TankB_M == 1 && TankB_L == 1){
//            P_1 = 0; //ON
//            P_2 = 1; //OFF
//        }
//        else if (TankB_H == 0 && TankB_M ==0 && TankB_L ==1){
//            P_1 = 0; //ON
//            P_2 = 0; //ON
//            
//        }
//        else if (TankB_H == 0 && TankB_M ==0 && TankB_L ==0){
//            P_1 = 0; //ON
//            P_2 = 0; //ON
//            
//        }
//    }
   
//    if(TankB_H == 1){
//        P_1 = 1;
//        P_2 = 1;   
//    }else{
//        if(TankB_H == 0 && TankA_H == 1){
//            P_1 = 0;
//            P_2 = 1;
//        } else if()
//            
//    }
    
    if(TankB_H ==1){
        P_1 = 1; //OFF
        P_2 = 1; //OFF
    }else{
        if(TankA_H ==1){
            P_1 = 0; //ON
            P_2 = 0; //ON
        }else if(TankA_L ==1){
            P_1 = 0; //ON
            P_2 = 1; //OFF
        }else{
            P_1 = 1; //OFF
            P_2 = 1; //OFF
        }
    }
}
void controllingBuzzerB(void){
    if (TankB_H == 1 && tmr0_active ==0) {    //if water is at H in TankB:
        BUZB = 1;                     //  ON BuzzerB
        initializingTimer0();         //  starting Timer0
        tmr0_active = 1;
    }
//    if(TankB_H ==1 && tmr0_active ==0){
//        BUZB = 1;
//        __delay_ms(1000);
//        BUZB = 0;
//        tmr0_active = 1;
//    }
}

void main(void) {
    initializingPorts();
    initializeInterrupts();
    while (1) {
        updatingLEDs();
        controllingValve();
        controllingPumps();
        controllingBuzzerB();
    }
}
