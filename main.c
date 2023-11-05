#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"

//Reloj a 16MHz
#define XTAL 16000000

//Variables
int bandera = 0;
uint32_t i;

//Prototipos
void setup(void);

//Main
int main(void){
    setup();                                                                                    //Setup

//Loop
    while(1){

    }
}

void setup(void){
    //General
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);     //Reloj a 40MHz
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);                                                //Habilitar el puerto F
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));                                         //Esperar a que se inicialice
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);                                                //Habilitar el puerto A
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);               //Configurar LEDS como salidas
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);                     //Iniciar con leds apagados
    //Timer
    IntMasterEnable();                                                                          //Activar interrupciones
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);                                               //Habilitar Timer0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);                                            //Configurar a 32 bits periódico contando hacia abajo
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);                                         //Configurar pin de toggle
    TimerLoadSet(TIMER0_BASE, TIMER_BOTH, 20000000 - 1);                                        //Cuenta hasta los 0.5s
    IntEnable(INT_TIMER0A);                                                                     //Activar interrupcion TIMER0A
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                                            //Interrupción en timeout (0.5s)
    TimerEnable(TIMER0_BASE, TIMER_A);                                                          //Iniciar Timer0
    //UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);                                                //Habilitar módulo UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);                                                            //RX en A0
    GPIOPinConfigure(GPIO_PA1_U0TX);                                                            //TX en A1
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);                                  //Habilitar los pines
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE)); //UART0 con 115200 de baudrate, de 8 bits, de un dato, sin paridad
    IntEnable(INT_UART0);                                                                       //Activar interrupcion UART0
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);                                       //Activar interrupcion TX, RX
    UARTEnable(UART0_BASE);                                                                     //Iniciar UART0
}

void Timer0IntHandler(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                                                     //Limpiar bandera de interrupción
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5) ^ GPIO_PIN_5);   //Toggle del LED
}

void UARTIntHandler(void){
    char rx;                                                                                    //Variable para guardar dato
    UARTIntClear(UART0_BASE, UART_INT_RX | UART_INT_RT);                                        //Limpiar bandera de interrupción
    while(UARTCharsAvail(UART0_BASE))                                                           //Si hay datos
        {
            rx = UARTCharGetNonBlocking(UART0_BASE);                                            //Guardar dato
            UARTCharPutNonBlocking(UART0_BASE, rx);                                             //Mostrar el dato recibido
        }
    if (rx == 'r'){                                                                             //Si se recibió 'r'
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1); //Toggle del led rojo
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);                                           //Apagar led azul
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);                                           //Apagar led verde
    }

    else if (rx == 'g'){                                                                        //Si se recibió 'g'
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);                                           //Apagar led rojo
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);                                           //Apagar led azul
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3) ^ GPIO_PIN_3); //Toggle del led verde
    }

    else if (rx == 'b'){                                                                        //Si se recibió 'b'
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);                                           //Apagar del led rojo
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) ^ GPIO_PIN_2); //Toggle del led azul
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);                                           //Apagar led verde
    }
}
