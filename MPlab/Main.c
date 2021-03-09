// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#define _XTAL_FREQ   4000000


// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "I2CLIB.h"

char sensor_dir = 0xEC; //ID del sensor

char TEMP_LSB = 0;
char led = 0;       //Variables para almacenar datos de UART y del I2C


void setup(void); //Se crean prototipos
void setup(void) {
    ANSEL = 0;
    ANSELH = 0;         //Puertos en Digital

    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;       //Puertos como inputs
    TRISCbits.TRISC3 = 0; //RC3 salida para SCL
    TRISCbits.TRISC7 = 1; //RC7 como entrada para RX
    TRISD = 0x00;
    TRISE = 0x00;
    PORTE = 0x00;       //Se limpia puerto para leds

    I2C_Master_Init(9); //Se configura direccion e I2C

    TXSTAbits.BRGH = 1; //Baudrate 9600 
    BAUDCTLbits.BRG16 = 0;
    SPBRG = 25;

    //Transmision
    TXSTAbits.TXEN = 1; //Se habilita TX
    TXSTAbits.SYNC = 0; //modo Asíncrono
    RCSTAbits.SPEN = 1; //Se habilita RX
    TXSTAbits.TX9 = 0; //Se transmiten 8 bits

    //Lectura
    RCSTAbits.CREN = 1; //Se habilita recibir datos
    RCSTAbits.RX9 = 0; //Se reciben solo 8 bits

    //ENCENDEMOS INTERRUPCIONES
    PIE1bits.RCIE = 1;
    PIE1bits.TXIE = 0; //No se habilitan interrupciones en el envio
    PIR1bits.RCIF = 0; //Se apaga la interrupcion

    INTCONbits.GIE = 1; //Interrupciones del timer
    return;

}

void __interrupt() ISR(void) {
    if (PIR1bits.RCIF == 1) {
        PIR1bits.RCIF = 0; //Resetea bandera RCIF
        led = RCREG;       //Se lee el registro y se guarda
    }
    if (led == 0X0A) {      //Dependiendo del dato que entra en RX enciende o apaga las leds
        PORTE = 0;          //depende de los botones en el IOT cloud
    } else if (led == 0X0B) {
        PORTE = 1;
    } else if (led == 0X0C) {
        PORTE = 0;
    } else if (led == 0X0D) {
        PORTE = 2;
    }
    return;
}

void main(void) {
    setup(); //Se ejecuta el setup
    while (1) {
        TXREG = TEMP_LSB;   //Enviamos por TX los datos del I2C
        I2C_Master_Start(); //Iniciamos I2C
        I2C_Master_Write(0b11101100); //Escribimos el adress de 7 bits y el bit de escritura
        I2C_Master_Write(0xD0); //Se escribe en el dress
        I2C_Master_Stop(); //Se detiene la condicion
        __delay_ms(200);
        I2C_Master_Start(); //Se inicia el I2C
        I2C_Master_Write(0b11101101); //Escribimos el adress de 7 bits y el bit de lectura
        TEMP_LSB = I2C_Master_Read(0); //Se lee y guarda el dato
        I2C_Master_Stop(); //Se detiene la condicion
        __delay_ms(200);
    }
}

