#include <msp430.h>
#include "HAL_Dogs102x6.h"
#include "HAL_Buttons.h"
#include <stdio.h>
#include <intrinsics.h>
#include <string.h>
#include <stdint.h>

#define BUTTON_PORT_IN    PAIN

void startup_function();
void options_menu();
void option_1_led();
void option_2_ios();
void option_3_adc();
void option_4_timer();

int status = 0;
char p_str[200] ={0};

unsigned int volt_hex;
float volt_float;
char pi_str1[30] ={0};
char pi_str2[3] ={0};

int main(void)
{
  volatile unsigned int i;

  uint8_t contrast, brightness;
  contrast = 11;
  brightness = 11;

  WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT

    // Configure LEDs
    P1DIR |= BIT0;
    P8DIR |= BIT1;
    P8DIR |= BIT2;
    P8OUT &= (~BIT2);

    P1DIR |= BIT1;
    P1DIR |= BIT2;
    P1DIR |= BIT3;
    P1DIR |= BIT4;
    P1DIR |= BIT5;


    // ADC Initialization
    ADC12CTL0 =  ADC12ON;         // Sampling time, ADC12 on
    ADC12CTL1 = 0x5000|ADC12SHP;  // Sample and Hold Mode
    ADC12IE = 0x20;               // Enable interrupt for channel 5
    ADC12MCTL5 |= 0x05;

    P6SEL |= 0x20;                // P6.5 ADC option select
    P6DIR &= 0xDF;

    ADC12CTL0 |= ADC12ENC;  //Enable conversion


  // Configure button ports
  PADIR &= ~0x0480;               // Buttons on P1.7/P2.2 are inputs
  P2DIR &= 0xfb;

  // Configure button 1 and 2
  P1DIR |= BIT7;
  P2DIR |= BIT2;

  // Configure CapTouch ports
  P1OUT &= ~0x7E;
  P1DIR |= 0x7E;
  P6OUT = 0x00;
  P6DIR = 0x00;

  // Configure Dogs102x6 ports
  P5OUT &= ~(BIT6 + BIT7);        // LCD_C/D, LCD_RST
  P5DIR |= BIT6 + BIT7;
  P7OUT &= ~(BIT4 + BIT6);        // LCD_CS, LCD_BL_EN
  P7DIR |= BIT4 + BIT6;
  P4OUT &= ~(BIT1 + BIT3);        // SIMO, SCK
  P4DIR &= ~BIT2;                 // SOMI pin is input
  P4DIR |= BIT1 + BIT3;

  __enable_interrupt();

  // Buttons Initialization -> S1 - Interrupt based ; S2 - Polling based
  Buttons_init(BUTTON_ALL);
  Buttons_interruptEnable(BUTTON_S1);
  buttonsPressed = 0;

  // LCD initialization
  Dogs102x6_init();
  Dogs102x6_backlightInit();

  // Set Default Contrast
	  contrast = 11;

  // Set Default Brightness
	  brightness = 11;

  Dogs102x6_setBacklight(brightness);
  Dogs102x6_setContrast(contrast);
  Dogs102x6_clearScreen();
  Dogs102x6_stringDraw(1,1,"Welcome",DOGS102x6_DRAW_NORMAL);
  for(i = 1;i<=5;i++)
  __delay_cycles(200000);
  //startup_function();
  options_menu();

  //UART Init
   P4SEL = BIT4+BIT5;                        // P3.4,5 = USCI_A0 TXD/RXD
   UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
   UCA1CTL1 |= UCSSEL_2;                     // SMCLK
   UCA1BR0 = 6;                              // Lower Byte = Integer part of Prescaler
   UCA1BR1 = 0;                              // Higher Byte = 0
   UCA1MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Modln UCBRSx=0, UCBRFx=13 (Fractional input)
                                             // over sampling = 1048576/(16*(prescaler))
   UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
   UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt


   // Timer A init
//   TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
//   TA0CCR0 = 100000;
//   TA0CTL = TASSEL_2 + ID_3 + MC_1;                  // SMCLK, upmode



   __enable_interrupt();

   while(1){
       __bis_SR_register(LPM0_bits | GIE); // Enter Low Power Mode 0 + Global Interrupt Enable
   }
}

void startup_function() {
    Dogs102x6_stringDraw(1,1,"Welcome",DOGS102x6_DRAW_NORMAL);
    int i;
    for (i = 0; i < 5; i++) {
        if (i % 2) {
            P1OUT |= BIT0;
            P8OUT |= BIT1;
            P8OUT |= BIT2;
            _delay_cycles(1000000);
        } else {
            P1OUT &= (~BIT0);
            P8OUT &= (~BIT1);
            P8OUT &= (~BIT2);
            _delay_cycles(1000000);
        }
    }
        P1OUT |= BIT1;
        _delay_cycles(500000);
        P1OUT |= BIT2;
        _delay_cycles(500000);
        P1OUT |= BIT3;
        _delay_cycles(500000);
        P1OUT |= BIT4;
        _delay_cycles(500000);
        P1OUT |= BIT5;
        _delay_cycles(500000);

        P1OUT &= (~BIT1);
        P1OUT &= (~BIT2);
        P1OUT &= (~BIT3);
        P1OUT &= (~BIT4);
        P1OUT &= (~BIT5);
}

void options_menu() {
    __enable_interrupt();
    Dogs102x6_clearScreen();
    Dogs102x6_stringDraw(1,1,"Diagnostics tool",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(2,1,"v.1.0 UTD",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(3,1,"1 LEDs",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(4,1,"2 IOs",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(5,1,"3 ADC",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(6,1,"4 Timers",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(7,1,"5 LCD",DOGS102x6_DRAW_NORMAL);
    UCA1TXBUF = 0x0A;
    sprintf(p_str, "\n \rDiagnostics tool v.1.0 UTD\n \r"
            "1 LEDs\n \r"
            "2 IOs\n \r"
            "3 ADC\n \r"
            "4 Timers\n \r"
            "5 LCD\n \r");
    int y;
    while (!(UCA1IFG&UCTXIFG));
    for (y = 0; y < 74; y++) {
         while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
         UCA1TXBUF = p_str[y];
         _delay_cycles(50);
    }

}

   // UART Setup
   #if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)    //UART ISR
   #pragma vector=USCI_A1_VECTOR
   __interrupt void USCI_A1_ISR(void)
   #elif defined(__GNUC__)
   void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
   #else
   #error Compiler not supported!
   #endif
   {
   Dogs102x6_init();
     switch(UCA1RXBUF) {
         case '1':{
             if (status == 0) {
                 sprintf(p_str, "\n \rLED 1 - Press 1\n \rLED 2 - Press 2\n \rLED 3 - Press 3\n \rPAD 1 - Press 4\n \rPAD 2 - Press 5\n \r"
                         "PAD 3 - Press 6\n \rPAD 4 - Press 7\n \rPAD 5 - Press 8\n \r");
                 int y;
                 while (!(UCA1IFG&UCTXIFG));
                 for (y = 0; y < 200; y++) {
                      while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                      UCA1TXBUF = p_str[y];
                      _delay_cycles(50);
                 }
                 option_1_led();
             } else if (status == 1) {
                 P1OUT ^= BIT0;
             }
          } break;
          case '2': {
              if (status == 1) {
                  P8OUT ^= BIT1;
              } else {
                  option_2_ios();
              }
          } break;
          case '3': {
              if (status == 1) {
                  P8OUT ^= BIT2;
              } else {
                  option_3_adc();
              }
          } break;
          case '4': {
              if (status == 1) {
                  P1OUT ^= BIT1;
              } else {
                  option_4_timer();
              }
          } break;
          case '5': {
              if (status == 1) {
                  P1OUT ^= BIT2;
              } else {
                  int r, c;
                  Dogs102x6_clearScreen();
                  for (r = 0; r < 102; r++) {
                      for (c = 0; c < 102; c++) {
                          Dogs102x6_pixelDraw(r, c, 0);
                      }
                  }
              }
          } break;
          case '6': {
              if (status == 1) {
                  P1OUT ^= BIT3;
              }
          } break;
          case '7': {
              if (status == 1) {
                  P1OUT ^= BIT4;
              }
          } break;
          case '8': {
              if (status == 1) {
                  P1OUT ^= BIT5;
              }
          } break;
          case '9': {
              status = 0;
              Dogs102x6_clearScreen();
              options_menu();
          } break;
          default:
              break;
     }
   }

void option_1_led() {
    Dogs102x6_init();
    Dogs102x6_clearScreen();
    Dogs102x6_stringDraw(0,1,"LED 1 - Press 1",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(1,1,"LED 2 - Press 2",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(2,1,"LED 3 - Press 3",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(3,1,"PAD 1 - Press 4",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(4,1,"PAD 2 - Press 5",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(5,1,"PAD 3 - Press 6",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(6,1,"PAD 4 - Press 7",DOGS102x6_DRAW_NORMAL);
    Dogs102x6_stringDraw(7,1,"PAD 5 - Press 8",DOGS102x6_DRAW_NORMAL);
    status = 1;
}

void option_2_ios() {
    Dogs102x6_init();
    Dogs102x6_clearScreen();
    __enable_interrupt();

    while (1) {
        if ((!(P1IN & BIT7)) && (!(P2IN & BIT2))) {
            Dogs102x6_stringDraw(1,1,"Button N:  ON",DOGS102x6_DRAW_NORMAL);
            Dogs102x6_stringDraw(2,1,"Button S:  ON",DOGS102x6_DRAW_NORMAL);

            sprintf(p_str, "\n \rButton N:  ON\n \rButton S:  ON ");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 35; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            _delay_cycles(1000000);

        } else if (!(P1IN & BIT7)) {
            Dogs102x6_stringDraw(1,1,"Button N: OFF",DOGS102x6_DRAW_NORMAL);
            Dogs102x6_stringDraw(2,1,"Button S:  ON",DOGS102x6_DRAW_NORMAL);

            sprintf(p_str, "\n \rButton N:  OFF\n \rButton S:  ON ");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 35; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            _delay_cycles(1000000);

        } else if (!(P2IN & BIT2)) {
            Dogs102x6_stringDraw(1,1,"Button N:  ON",DOGS102x6_DRAW_NORMAL);
            Dogs102x6_stringDraw(2,1,"Button S: OFF",DOGS102x6_DRAW_NORMAL);

            sprintf(p_str, "\n \rButton N:  ON\n \rButton S:  OFF ");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 35; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            _delay_cycles(1000000);

        } else if ((P1IN & BIT7) && (P2IN & BIT2)) {
            Dogs102x6_stringDraw(1,1,"Button N: OFF",DOGS102x6_DRAW_NORMAL);
            Dogs102x6_stringDraw(2,1,"Button S: OFF",DOGS102x6_DRAW_NORMAL);

            sprintf(p_str, "\n \rButton N:  OFF\n \rButton S:  OFF ");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 35; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            _delay_cycles(1000000);
        }

        if (UCA1RXBUF == '9') {
            break;
        }
    }
}


void option_3_adc() {
    Dogs102x6_init();
    Dogs102x6_clearScreen();

    while(1){
        ADC12CTL0 |= ADC12SC;   // start conversion

        while(ADC12IV != 0x10);

        volt_hex = ADC12MEM5;   // Read converted value for channel 5. This clears the interrupt flag
        volt_float = volt_hex * 0.0008789;
        sprintf(pi_str1, "\n \r%f V \n \r", volt_float);
        Dogs102x6_stringDraw(1,0,pi_str1,DOGS102x6_DRAW_NORMAL);

        int i;
        for (i = 0; i < 30; i++) {
            while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
            UCA1TXBUF = pi_str1[i];
            _delay_cycles(50);
        }

        __enable_interrupt();
        _delay_cycles(1000000);
        __disable_interrupt();

        if (UCA1RXBUF == '9') {
            return;
        }
    }
}


void option_4_timer() {
    Dogs102x6_init();
    Dogs102x6_clearScreen();

    while(1){
        ADC12CTL0 |= ADC12SC;   // start conversion

        while(ADC12IV != 0x10);

        volt_hex = ADC12MEM5;   // Read converted value for channel 5. This clears the interrupt flag
        volt_float = volt_hex * 0.0008789;

        // 0.08 increments
        if ((volt_float >= 0.00) && (volt_float < 0.08)) {
            Dogs102x6_stringDraw(1,0,"2 seconds ",DOGS102x6_DRAW_NORMAL);
            sprintf(p_str, "\n \r2 seconds \n \r");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 17; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            if (!(P2IN & BIT2)) {
                Dogs102x6_stringDraw(1,0,"Safe open     ",DOGS102x6_DRAW_NORMAL);
                P1OUT |= BIT0;
                _delay_cycles(2000000);
                P1OUT &= ~BIT0;
                Dogs102x6_stringDraw(1,0,"Safe closed   ",DOGS102x6_DRAW_NORMAL);
                return;
            }
        } else if ((volt_float >= 0.08) && (volt_float < 0.16)) {
            Dogs102x6_stringDraw(1,0,"4 seconds ",DOGS102x6_DRAW_NORMAL);
            sprintf(p_str, "\n \r4 seconds \n \r");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 17; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            if (!(P2IN & BIT2)) {
                Dogs102x6_stringDraw(1,0,"Safe open     ",DOGS102x6_DRAW_NORMAL);
                P1OUT |= BIT0;
                _delay_cycles(4000000);
                P1OUT &= ~BIT0;
                Dogs102x6_stringDraw(1,0,"Safe closed   ",DOGS102x6_DRAW_NORMAL);
                return;
            }
        } else if ((volt_float >= 0.16) && (volt_float < 0.24)) {
            Dogs102x6_stringDraw(1,0,"6 seconds ",DOGS102x6_DRAW_NORMAL);
            sprintf(p_str, "\n \r6 seconds \n \r");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 17; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            if (!(P2IN & BIT2)) {
                Dogs102x6_stringDraw(1,0,"Safe open     ",DOGS102x6_DRAW_NORMAL);
                P1OUT |= BIT0;
                _delay_cycles(6000000);
                P1OUT &= ~BIT0;
                Dogs102x6_stringDraw(1,0,"Safe closed   ",DOGS102x6_DRAW_NORMAL);
                return;
            }
        } else if ((volt_float >= 0.24) && (volt_float < 0.32)) {
            Dogs102x6_stringDraw(1,0,"8 seconds ",DOGS102x6_DRAW_NORMAL);
            sprintf(p_str, "\n \r8 seconds \n \r");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 17; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            if (!(P2IN & BIT2)) {
                Dogs102x6_stringDraw(1,0,"Safe open     ",DOGS102x6_DRAW_NORMAL);
                P1OUT |= BIT0;
                _delay_cycles(8000000);
                P1OUT &= ~BIT0;
                Dogs102x6_stringDraw(1,0,"Safe closed   ",DOGS102x6_DRAW_NORMAL);
                return;
            }
        } else if (volt_float >= 0.32) {
            Dogs102x6_stringDraw(1,0,"10 seconds",DOGS102x6_DRAW_NORMAL);
            sprintf(p_str, "\n \r10 seconds \n \r");
            int y;
            while (!(UCA1IFG&UCTXIFG));
            for (y = 0; y < 17; y++) {
                 while (!(UCA1IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
                 UCA1TXBUF = p_str[y];
                 _delay_cycles(100);
            }
            if (!(P2IN & BIT2)) {
                Dogs102x6_stringDraw(1,0,"Safe open     ",DOGS102x6_DRAW_NORMAL);
                P1OUT |= BIT0;
                _delay_cycles(10000000);
                P1OUT &= ~BIT0;
                Dogs102x6_stringDraw(1,0,"Safe closed   ",DOGS102x6_DRAW_NORMAL);
                return;
            }
        }

        __enable_interrupt();
        _delay_cycles(1000000);
        __disable_interrupt();

        if (UCA1RXBUF == '9') {
            return;
        }
    }
}

//// Timer A0 interrupt service routine
//#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=TIMER0_A0_VECTOR
//__interrupt void Timer_A (void)
//#elif defined(__GNUC__)
//void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
//#else
//#error Compiler not supported!
//#endif
//{
//  P1OUT |= BIT0;                            // Toggle P1.0
//  _bis_SR_register_on_exit(LPM3_bits + GIE);
//}
