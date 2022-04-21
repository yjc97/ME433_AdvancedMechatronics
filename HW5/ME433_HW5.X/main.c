#include <xc.h>          // processor SFR definitions
#include <sys/attribs.h> // __ISR macro
#include <stdio.h>
#include <math.h>
#include "spi.h"

#define PI 3.14159265
#define NU32_SYS_FREQ 48000000
#define NU32_DESIRED_BAUD 230400

// DEVCFG0
#pragma config DEBUG = OFF       // disable debugging
#pragma config JTAGEN = OFF      // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF         // disable flash write protect
#pragma config BWP = OFF         // disable boot write protect
#pragma config CP = OFF          // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL //  use fast frc oscillator with pll
#pragma config FSOSCEN = OFF  // disable secondary oscillator
#pragma config IESO = OFF     // disable switching clocks
#pragma config POSCMOD = OFF   // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD       // disable clock switch and FSCM
#pragma config WDTPS = PS1048576    // use largest wdt value
#pragma config WINDIS = OFF         // use non-window mode wdt
#pragma config FWDTEN = OFF         // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0     // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF  // allow multiple reconfigurations

void ReadUART1(char *string, int maxLength);
void WriteUART1(const char *string);
void blink();

unsigned short genSine(unsigned int i, unsigned short v);
unsigned short genTriangle(unsigned int i, unsigned short v);


char m[100];

int main() {

  __builtin_disable_interrupts(); // disable interrupts while initializing
                                  // things

  // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
  __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

  // 0 data RAM access wait states
  BMXCONbits.BMXWSDRM = 0x0;

  // enable multi vector interrupts
  INTCONbits.MVEC = 0x1;

  // disable JTAG to get pins back
  DDPCONbits.JTAGEN = 0;

  // B4 is input
  TRISBbits.TRISB4 = 1;

  // A4 is output
  TRISAbits.TRISA4 = 0;

  // A4 is low
  LATAbits.LATA4 = 0;
  
  initSPI();
  
  U1RXRbits.U1RXR = 0b0000; // Set A2 to U1RX
  RPB3Rbits.RPB3R = 0b0001; // Set B3 to U1TX
   // turn on UART3 without an interrupt
  U1MODEbits.BRGH = 0; // set baud to NU32_DESIRED_BAUD
  U1BRG = ((NU32_SYS_FREQ / NU32_DESIRED_BAUD) / 16) - 1;

  // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
  U1MODEbits.PDSEL = 0;
  U1MODEbits.STSEL = 0;

  // configure TX & RX pins as output & input pins
  U1STAbits.UTXEN = 1;
  U1STAbits.URXEN = 1;

  // enable the uart
  U1MODEbits.ON = 1;
  
  __builtin_enable_interrupts();

  int i = 1;
  
  while (1) {
    unsigned short v = 511;
    unsigned char c, c1;
    unsigned short p, p1;
    unsigned short vf, vf1;
    for (i = 0; i < 100; i = i + 1) {
      c = 1;
      vf = genSine(i, v);
      p = (c << 15);
      p = p | (0b111 << 12);
      p = p | (vf << 2);

      c1 = 0;
      vf1 = genTriangle(i, v);
      p1 = (c1 << 15);
      p1 = p1 | (0b111 << 12);
      p1 = p1 | (vf1 << 2);

      LATAbits.LATA0 = 0; // bring CS low
      spi_io(p >> 8);     // write the byte
      spi_io(p);
      LATAbits.LATA0 = 1; // bring CS high */

      LATAbits.LATA0 = 0; // bring CS low
      spi_io(p1 >> 8);    // write the byte
      spi_io(p1);
      LATAbits.LATA0 = 1; // bring CS high

      _CP0_SET_COUNT(0);                            // delay here
      while (_CP0_GET_COUNT() < 48000000 / (100)) { // 1kHz
        ;
      }
    }

  }
  
}

void blink(){
    
     LATAbits.LATA4 = 1;  
     _CP0_SET_COUNT(0);
     while(_CP0_GET_COUNT()<24000000/2){}
     LATAbits.LATA4 = 0;
     _CP0_SET_COUNT(0);
     while(_CP0_GET_COUNT()<24000000/2){}
     LATAbits.LATA4 = 1;
     _CP0_SET_COUNT(0);
     while(_CP0_GET_COUNT()<24000000/2){}
     LATAbits.LATA4 = 0;
     _CP0_SET_COUNT(0);
     while(_CP0_GET_COUNT()<24000000/2){}
}

unsigned short genSine(unsigned int i, unsigned short v) {
  double ret, val;
  val = PI / 180;
  // ret = sin(8*i*val); //2Hz frequency
  ret = 512.0 * (sin((4 * PI * (i / (100 / 2.0))))) + 512.0;

  return (ret);
}

double tri = 0;
unsigned short genTriangle(unsigned int i, unsigned short v) {
  double tri;
  if (i < 50) {
    tri = abs((1024 / 25) * i - 1023);
  } else {
    tri = abs((1024 / 25) * (i - 50) - 1023);
  }

  return (tri);
}

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

void WriteUART1(const char * string) {
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U1TXREG = *string;
    ++string;
  }
}