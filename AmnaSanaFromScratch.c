#include <xc.h>

// Define clock frequency for __delay_ms
#define _XTAL_FREQ 20000000  // 20 MHz Crystal Oscillator

//TANBK B CONFIGURATIONS
#define TankB_L RC0 
#define TankB_H RC2 
#define B_L RC3
#define B_H RD1

//TANK A CONFIGURATIONS
#define A_E RD2
#define A_H RD3
#define A_L RC4
#define TankA_E RC5
#define TankA_H RC6
#define TankA_L RC7

#define VAL RB7
//PUMP CONFIGURATION
#define P_1 RB5
//#define P_2 RD7

#define BUZB RD5

#define BTN RB0
#define BUZA RB1


//#define timeout0_s 5                //MCU will buzz BuzzerB for 5 seconds after water reaches H of TankB.
//#define timeout0_ms (timeout0_s*1000)
//#define tmr0_target (timeout0_ms/65)//CHANGE TO 262 and check!!!!!!!!!! "Change 131 to 4.1 in the calculation"  //Is Timer0's overflow target.
//                                     //  Dividing  by 65.5ms becuz Timer0 overflows in approx. 65.5ms (becuz pre-scalar is set to 1:256 later).


// Configuration bits
#pragma config FOSC = HS        // High Speed Oscillator
#pragma config WDTE = OFF       // Watchdog Timer Disabled
#pragma config PWRTE = OFF      // Power-up Timer Disabled
#pragma config BOREN = ON       // Brown-out Reset Enabled
#pragma config LVP = OFF        // Low Voltage Programming Disabled
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection Off
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits Off
#pragma config CP = OFF         // Flash Program Memory Code Protection Off

//volatile long tmr0_overflow_count = 0;
//volatile unsigned char tmr0_active = 0;
volatile unsigned char buzB_triggered = 0;

volatile unsigned char buzA_active = 0;
volatile unsigned char emergency_reset_required = 0;

void initializingPorts(void);
void initializingTimer0(void);
void initializeInterrupts(void);
void updatingLEDs(void);
void controllingValve(void);
void controllingPumps(void);
void controllingBuzzerB(void);
void controllingBuzzerA(void);

void initializingPorts(void){
    TRISCbits.TRISC0 = 1; //TankB_L
    TRISCbits.TRISC2 = 1; //TankB_H
    TRISCbits.TRISC3 = 0; //B_L
    TRISDbits.TRISD1 = 0; //B_H
    
    PORTCbits.RC3 = 0; //clearing bit for safety
    PORTDbits.RD0 = 0; //clearing bit for safety
    PORTDbits.RD1 = 0; //clearing bit for safety
    
    TRISDbits.TRISD2 = 0; //A_E
    TRISDbits.TRISD3 = 0; //A_H
    TRISCbits.TRISC4 = 0; //A_L
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
//    TRISDbits.TRISD7 = 0; //P_2
//    PORTDbits.RD7 = 0;
    
    TRISDbits.TRISD5 = 0; //BUZ2
    PORTDbits.RD5 = 0;
    
    TRISBbits.TRISB0 = 1;  //BTN
    TRISBbits.TRISB1 = 0; //BUZ1
    PORTBbits.RB1 = 0;
}
//void initializingTimer0(void) {
//    OPTION_REGbits.T0CS = 0; //TMR0 will use the Internal Instruction Cycle Clock as the clock source 
//    OPTION_REGbits.PSA = 0;  //assigns Pre-Scalar to TMR0
//    OPTION_REGbits.PS2 = 1;  //sets the pre-scalar to 1:256
//    OPTION_REGbits.PS1 = 1;  //sets the pre-scalar to 1:256
//    OPTION_REGbits.PS0 = 1;  //sets the pre-scalar to 1:256
//
//    INTCONbits.TMR0IE = 1; //enables Timer0 interrupt.
//    INTCONbits.TMR0IF = 0; //clears Timer0 interrupt flag.
//    TMR0 = 0; //clears the Timer0 register.
//
//}
void initializeInterrupts(void) {        
    INTCONbits.GIE = 1;   //enables the Global Interrupt.
    INTCONbits.PEIE = 1;  //enables the Peripheral Interrupts.
    
    INTCONbits.INTE = 1; //enables the External Interrupt.
    INTCONbits.INTF = 0; //clears the External Interrupt flag.

    OPTION_REGbits.INTEDG = 1; //clears the INTerrupt EDGge pin (for EXT INT))
                               //  i.e. the Interrupt is triggered on Falling edge
                               //  i.e. the Interrupt will happen when the change is from HIGHT to LOW.
    
}
void __interrupt() ISR(void){
//    // When Timer0 Overflows:
//    if (INTCONbits.TMR0IF ==1){     //checks if the flag of TMR0 is 1 (i.e. has it overflowed?):
//        TMR0 = 0; //clears the Timer0 register.
//        INTCONbits.TMR0IF = 0;  //sets tmr0 flag to 0.
//        tmr0_overflow_count++;
//        
//        if(tmr0_overflow_count >=20){
//            
//            BUZB = 0;
//            tmr0_overflow_count = 0;
//            tmr0_active = 0;
//            INTCONbits.TMR0IE = 0;
//        }
//    }
//    
    if(INTCONbits.INTF ==1){
        INTCONbits.INTF = 0; //resets flag back to 0.
        if (buzA_active ==1){
            BUZA = 0;
            buzA_active = 0;
            emergency_reset_required = 1; //doesn't let the buzzer get activated again until the water drops from E and rises back to it.
        }
    }
}
void updatingLEDs(void){
    if(TankB_L ==1){
        B_L = 1;
    }else{
        B_L = 0;
    }
    
    if(TankB_H == 1){
        B_H = 1;
    }else{
        B_H = 0;
    }
    ////
    ////
    if(TankA_E == 1){
        A_E = 1;
    }
    else{
        A_E = 0;
    }
    
    if(TankA_H == 1){
        A_H = 1;
    }else{
        A_H = 0;
    }
    
    if(TankA_L == 1){
        A_L = 1;
    }else{
        A_L = 0;
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
    if(TankB_H ==1){
        P_1 = 1; //OFF
       // P_2 = 1; //OFF
    }else{
        if(TankA_H ==1 || TankA_E ==1){
            P_1 = 0; //ON
            // P_2 = 0; //ON
         
        }else if(TankA_L ==1){
            P_1 = 0; //ON
           // P_2 = 1; //OFF
        }else if( TankA_L == 0){
            P_1 = 1; //OFF
           // P_2 = 1; //OFF
        }
    }
}
void controllingBuzzerB(void){
    
    if((TankB_H ==1)  && (buzB_triggered ==0)){
        BUZB = 1;
        __delay_ms(100);
        BUZB = 0;
        buzB_triggered = 1;
    }else if( TankB_H ==0 ){
        BUZB = 0;
        buzB_triggered = 0;
    }
    
//    if ( (TankB_H == 1) && (tmr0_active ==0) ) {    //if water is at H in TankB:
//        BUZB = 1;                     //  ON BuzzerB
//        tmr0_active = 1;
//        initializingTimer0();         //  starting Timer0        
//    }else if(TankB_H ==0){
//        BUZB = 0;
//    }
////    } else if ((TankB_H == 0) && (tmr0_active == 1)) {
////        // Water dropped before timeout ? stop buzzer early
////        BUZB = 0;
////        tmr0_active = 0;
////        INTCONbits.TMR0IE = 0;       // Disable Timer0 interrupt
////        tmr0_overflow_count = 0;     // Reset counter
////    }
}
void controllingBuzzerA(void){
    // Checks whether there is a risk of overflow and causes the Emergency State:-
    if ( (TankA_E ==1) && (buzA_active ==0) && (emergency_reset_required ==0)){   //if water is at E in TankA:
        BUZA = 1;
        buzA_active = 1;
    } else if (TankA_E == 0){
        BUZA = 0;
        emergency_reset_required = 0;
    }
}

void main(void) {
    initializingPorts();
    initializeInterrupts();
    while (1) {
        updatingLEDs();
        controllingValve();
        controllingPumps();
        controllingBuzzerB();
        controllingBuzzerA();
    }
}
