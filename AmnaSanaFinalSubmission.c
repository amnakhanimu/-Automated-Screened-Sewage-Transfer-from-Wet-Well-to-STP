#include <xc.h>
#define _XTAL_FREQ 20000000  // Defining Clock Frequency to 20 MHz Crystal Oscillator

// Configuring TankB:-
#define TankB_L RC0 
#define TankB_H RC2 
#define B_L RC3
#define B_H RD1

//Configuring TankA:-
#define A_E RD2
#define A_H RD3
#define A_L RC4
#define TankA_E RC5
#define TankA_H RC6
#define TankA_L RC7

//Configuring Valve:-
#define VAL RB7

// Configuring Pump:-
#define P_1 RB5

// Configuring Buzzers and Button:-
#define BUZB RD5
#define BTN RB0
#define BUZA RB1



// Basic Configuration bits:-
#pragma config FOSC = HS        // High Speed Oscillator
#pragma config WDTE = OFF       // Watchdog Timer Disabled
#pragma config PWRTE = OFF      // Power-up Timer Disabled
#pragma config BOREN = ON       // Brown-out Reset Enabled
#pragma config LVP = OFF        // Low Voltage Programming Disabled
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection Off
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits Off
#pragma config CP = OFF         // Flash Program Memory Code Protection Off


// Defining Global Variables:-
volatile unsigned char buzB_triggered = 0;             //to see if Buzzer B has already been activated when TankB's water lvl goes above lvl H.
volatile unsigned char buzA_active = 0;                //checks to see whether Buzzer A is on when TankA is at emergency lvl.
volatile unsigned char emergency_reset_required = 0;   //stops Buzzer A from turning on again unless the water lvl falls below E and then increases again.


// Declaring Functions:-
void initializingPorts(void);
void initializingTimer0(void);
void initializeInterrupts(void);
void updatingLEDs(void);
void controllingValve(void);
void controllingPumps(void);
void controllingBuzzerB(void);
void controllingBuzzerA(void);


// Initializing Ports:-
void initializingPorts(void){
    TRISCbits.TRISC0 = 1;  //TankB_L
    TRISCbits.TRISC2 = 1;  //TankB_H
    TRISCbits.TRISC3 = 0;  //B_L
    TRISDbits.TRISD1 = 0;  //B_H
    
    PORTCbits.RC3 = 0;     //clears bit for safety
    PORTDbits.RD0 = 0;     //clears bit for safety
    PORTDbits.RD1 = 0;     //clears bit for safety
    
    TRISDbits.TRISD2 = 0;  //A_E
    TRISDbits.TRISD3 = 0;  //A_H
    TRISCbits.TRISC4 = 0;  //A_L
    TRISCbits.TRISC5 = 1;  //TankA_E
    TRISCbits.TRISC6 = 1;  //TankA_H
    TRISCbits.TRISC7 = 1;  //TankA_L
    PORTDbits.RD2 = 0;     //clears bit for safety
    PORTDbits.RD3 = 0;     //clears bit for safety
    PORTCbits.RC4 = 0;     //clears bit for safety
    
    TRISBbits.TRISB7 = 0;  //VALVE
    PORTBbits.RB7 = 0;     //clears bit for safety
    
    TRISBbits.TRISB5 = 0;  //P_1 
    PORTBbits.RB5 = 0;     //clears bit for safety

    
    TRISDbits.TRISD5 = 0;  //BUZB
    PORTDbits.RD5 = 0;     //clears bit for safety.
    
    TRISBbits.TRISB0 = 1;  //BTN
    TRISBbits.TRISB1 = 0;  //BUZA
    PORTBbits.RB1 = 0;     //clears bit for safety.
}

// Initializing Interrupts:-
void initializeInterrupts(void) {        
    INTCONbits.GIE = 1;    //enables the Global Interrupt.
    INTCONbits.PEIE = 1;  //enables the Peripheral Interrupts.
    
    INTCONbits.INTE = 1; //enables the External Interrupt.
    INTCONbits.INTF = 0; //clears the External Interrupt flag.

    OPTION_REGbits.INTEDG = 1; //clears the INTerrupt EDGge pin (for EXT INT))
                               //  i.e. the Interrupt is triggered on Falling edge
                               //  i.e. the Interrupt will happen when the change is from HIGHT to LOW.
    
}

// Running the Interrupt Service Routine (when needed):-
void __interrupt() ISR(void){   
    if(INTCONbits.INTF ==1){
        INTCONbits.INTF = 0;    //resets flag back to 0.
        if (buzA_active ==1){
            BUZA = 0;                       //turns BuzzerA off.
            buzA_active = 0;                //shows that BuzzerA is inactive.
            emergency_reset_required = 1;   //shows that BuzzerA has been active Once, since the water hit E this time.
        }
    }
}

// Updating LEDS based on water lvls:-
void updatingLEDs(void){
   //Updating the Low lvl L LED for TankB:
    if(TankB_L ==1){
        B_L = 1;
    }else{
        B_L = 0;
    }

   //Updating the High lvl LED H for TankB:   
    if(TankB_H == 1){
        B_H = 1;
    }else{
        B_H = 0;
    }
  
   //Updating the Emergency lvl E LED for TankA:
    if(TankA_E == 1){
        A_E = 1;
    }
    else{
        A_E = 0;
    }
    
   //Updating the High lvlH LED for TankA:
    if(TankA_H == 1){
        A_H = 1;
    }else{
        A_H = 0;
    }
    
   //Updating the Low lvl L LED for TankA:
    if(TankA_L == 1){
        A_L = 1;
    }else{
        A_L = 0;
    }
}

// Controlling the Valve based on TankA's water lvls:-
void controllingValve(void){
    //Closes the Valve when TankA is full or at emergency lvl:
    if(TankA_H ==1 || TankA_E ==1){
        VAL = 1; //Valve OFF
    }else{
        VAL = 0; //Valve ON
    }
}

// Controlling the Pump for water flow from TankA to TankB:-
void controllingPumps(void){ 
    //Turns Pump off when TankB is at H:
    if(TankB_H ==1){
        P_1 = 1; //OFF
    
    //TankA's lvls are checked when TankB is below H:
    }else{
        //Keeps Pump On when TankA is at H or E:
        if(TankA_H ==1 || TankA_E ==1){
            P_1 = 0; //ON
         
        //Turns Pump On when TankA is at L:
        }else if(TankA_L ==1){
            P_1 = 0; //ON

        //Turns Pump Off when TankA is below L:
        }else if( TankA_L == 0){
            P_1 = 1; //OFF

        }
    }
}

// Controlling BuzzerB as TankB's "TankB is full" notification:-
void controllingBuzzerB(void){
    //Buzzer B briefly turns on when TankB reaches H and dose not turn on again unless the water lvl falls below H and then rises again:
    if((TankB_H ==1)  && (buzB_triggered ==0)){
        BUZB = 1;              //turns BuzzerB ON.
        __delay_ms(100);       //causes a delay to keep the buzzer on for 0.1s.
        BUZB = 0;              //turns BuzzerB OFF.
        buzB_triggered = 1;    //prevents BuzzerB from turning on again until the water lvl falls from H and rises to H again.
    }else if( TankB_H ==0 ){
        BUZB = 0;              //ensure buzzer is OFF.
        buzB_triggered = 0;    //resets the trigger when water lvl drops from H.
    }
    
}

//Controlling BuzzerA for TankA's Emergency State:-
void controllingBuzzerA(void){
    // Checks whether there is a risk of overflow and causes the Emergency State:-
    if ( (TankA_E ==1) && (buzA_active ==0) && (emergency_reset_required ==0)){ 
        BUZA = 1;                        //turns BuzzerA ON.
        buzA_active = 1;                 //shows that BuzzerA is inactive.
    } else if (TankA_E == 0){ 
        BUZA = 0;                        //BuzzerA is OFF when water drops.
        emergency_reset_required = 0;    //stops Buzzer A from turning on again unless the water lvl falls below E and then increases again.
    }
}

void main(void) {
    initializingPorts();
    initializeInterrupts();
    
    //Looping all functions to update every component of the system frequently:-
    while (1) {
        updatingLEDs();
        controllingValve();
        controllingPumps();
        controllingBuzzerB();
        controllingBuzzerA();
    }
}
