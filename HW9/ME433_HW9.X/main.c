#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "ws2812b.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

#define PIC32_SYS_FREQ 48000000ul // 48 million Hz
#define PIC32_DESIRED_BAUD 230400 // Baudrate for RS232

void UART1_Startup(void);
void ReadUART1(char * string, int maxLength);
void WriteUART1(const char * string);
void blink();

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB4 = 1;
    LATAbits.LATA4 = 0;
    
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB4 = 1;
    
    U1RXRbits.U1RXR = 0b0000; // Set A2 to U1RX
    RPB3Rbits.RPB3R = 0b0001; // Set B3 to U1TX
    
    UART1_Startup();
    ws2812b_setup();
    
    __builtin_enable_interrupts();
    
    
    // initialize the hues here
    float hue1 = 0;
    float hue2 = hue1 + 60;
    float hue3 = hue2 + 60;
    float hue4 = hue3 + 60;
    float hue5 = hue4 + 60;
    char m[100];
    // reset the lights before use
    LATBbits.LATB6 = 0;
    TMR2 = 0;

    while (TMR2 < 2400) {
        ;
    } // wait 50uS, reset condition

    while (1) {
        blink();
        wsColor color1 = HSBtoRGB(hue1, 1, 0.1);
        wsColor color2 = HSBtoRGB(hue2, 1, 0.1);
        wsColor color3 = HSBtoRGB(hue3, 1, 0.1);
        wsColor color4 = HSBtoRGB(hue4, 1, 0.1);
        wsColor color5 = HSBtoRGB(hue5, 1, 0.1);
        wsColor color_array[5] = {color1, color2, color3, color4, color5}; // set the structure to hold the RGB arrays

        hue1=hue1+60;
        hue2=hue2+60;
        hue3=hue3+60;
        hue4=hue4+60;
        hue5=hue5+60;
        
        
//        sprintf(m,"%f %f %f %f %f\r\n",hue1,hue2,hue3,hue4,hue5);
//        WriteUART1(m);

        if (hue1 > 360) {
        hue1 = 0;
        }

        if (hue2 > 360) {
        hue2 = 0;
        }

        if (hue3 > 360) {
        hue3 = 0;
        }

        if (hue4 > 360) {
        hue4 = 0;
        }

        if (hue5 > 360) {
        hue5 = 0;
        }

        ws2812b_setColor(color_array, 5);

        TMR2 = 0;
        while (TMR2 < 5000) {
        } // wait to slow down color change
    }
}

// Read from UART1
// block other functions until you get a '\r' or '\n'
// send the pointer to your char array and the number of elements in the array
void ReadUART1(char * message, int maxLength) {
  char data = 0;
  int complete = 0, num_bytes = 0;
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (U1STAbits.URXDA) { // if data is available
      data = U1RXREG;      // read the data
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        message[num_bytes] = data;
        ++num_bytes;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  message[num_bytes] = '\0';
}

// Write a character array using UART1
void WriteUART1(const char * string) {
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U1TXREG = *string;
    ++string;
  }
}


void UART1_Startup() {
  // disable interrupts
  __builtin_disable_interrupts();

  // turn on UART1 without an interrupt
  U1MODEbits.BRGH = 0; // set baud to PIC32_DESIRED_BAUD
  U1BRG = ((PIC32_SYS_FREQ / PIC32_DESIRED_BAUD) / 16) - 1;

  // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
  U1MODEbits.PDSEL = 0;
  U1MODEbits.STSEL = 0;

  // configure TX & RX pins as output & input pins
  U1STAbits.UTXEN = 1;
  U1STAbits.URXEN = 1;

  // enable the uart
  U1MODEbits.ON = 1;

  __builtin_enable_interrupts();
}

void blink(){
    LATAbits.LATA4 = 1;
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<24000000/2/20){}
    LATAbits.LATA4 = 0;
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<24000000/2/20){}
}