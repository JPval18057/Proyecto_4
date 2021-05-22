//**************************************************************************************************************
/**
 * LABORATORIO 7 ELECTRÓNICA DIGITAL II
 * SECCION: 31
 * JUAN PABLO VALENZUELA, CARNET: 18057
 * DESCRIPCION:
 * USO Y CONFIGURACIÓN DE LAS INTERRUPCIONES Y EL MODULO UART
 */
//**************************************************************************************************************
// Librerías necesarias
//**************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"

#include "inc/hw_gpio.h"



#define XTAL 16000000
// No estoy seguro por qué pero necesita esto

#define GPIO_PD6_U2RX           0x00031801
#define GPIO_PD7_U2TX           0x00031C01


//**************************************************************************************************************
// Variables Globales
//**************************************************************************************************************
uint32_t        i = 0;
uint32_t        ui32Period;
uint32_t        receive = 114;
unsigned char   send_data=0;
uint32_t        boton1 = 0;
uint32_t        boton2 = 0;
uint32_t        boton3 = 0;
uint32_t        boton4 = 0;
uint32_t        estado_botones = 0;
unsigned char   espacio_disponible = 0;
uint32_t        estado_boton1 = 0;
uint32_t        estado_boton2 = 0;
uint32_t        estado_boton3 = 0;
uint32_t        estado_boton4 = 0;

//**************************************************************************************************************
// Prototipos de Funciones
//**************************************************************************************************************
void uart_test(void);
void InitUART(void);
void setup(void);

//Procesamos la entrada acá
//Convierto de entero 32bits a entero 8 bits para enviar datos
unsigned char buttons_math(uint32_t S);

//Manejamos las luces acá
void luces(void);

//Enviamos el estado del parqueo acá
void parking_update(void);

//Funciones para el display
void display(unsigned char F);
void cero(void);
void uno(void);
void dos(void);
void tres(void);
void cuatro(void);
void cinco(void);
//A PARTIR DE ACÁ NO SON NECESARIAS
void seis(void);
void siete(void);
void ocho(void);
void nueve(void);
//Función para transformar a caracteres
char num2string(uint32_t k);



















//**************************************************************************************************************
// Función Principal
//**************************************************************************************************************
int main(void)
{
    setup(); //llamamos a la configuración

    //**********************************************************************************************************
    // Loop Principal
    //**********************************************************************************************************
   while (1)
    {
       //Leemos los botones
       boton1 = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0);
       boton2 = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1);
       boton3 = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4);
       boton4 = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5);
       //Calculamos el estado de cada boton individualmente
       estado_boton1 = (boton1==GPIO_PIN_0);
       estado_boton2 = (boton2==GPIO_PIN_1);
       estado_boton3 = (boton3==GPIO_PIN_4);
       estado_boton4 = (boton4==GPIO_PIN_5);
       //Calculamos el estado general de todos los botones juntos
       estado_botones = (boton1==GPIO_PIN_0) + (boton2==GPIO_PIN_1) + (boton3==GPIO_PIN_4) + (boton4==GPIO_PIN_5);
       //Calculamos cuantos espacios están disponibles
       espacio_disponible = 4-buttons_math(estado_botones);

       //Detectamos si ya es hora de actualizar el parqueo
       if (send_data==1){
           /*
           UARTCharPut(UART0_BASE, 'A'); //caracter de inicio
           UARTCharPut(UART0_BASE, 10);
           UARTCharPut(UART0_BASE, 13);
           */
           send_data=0; //se reinicia el envio de datos
           //poner acá la función para enviar el estado actual del parqueo que incluye lo de arriba
           parking_update();

       }

       //La función buttons_math solo convierte el número de una variable de 32 bits a una de 8 bits para que se más fácil enviarla por serial
       display(espacio_disponible);
       //Indicamos por medio de luces si está disponible el lugar o esta ocupado
       luces();


    }
}
























//MENSAJE DE PRUEBA UART

void uart_test(void)
{
    // Se manda mensajes por UART
    UARTCharPut(UART0_BASE, 'H');
    UARTCharPut(UART0_BASE, 'o');
    UARTCharPut(UART0_BASE, 'l');
    UARTCharPut(UART0_BASE, 'a');
    UARTCharPut(UART0_BASE, ' ');
    UARTCharPut(UART0_BASE, 'M');
    UARTCharPut(UART0_BASE, 'u');
    UARTCharPut(UART0_BASE, 'n');
    UARTCharPut(UART0_BASE, 'd');
    UARTCharPut(UART0_BASE, 'o');
    UARTCharPut(UART0_BASE, 10);
    UARTCharPut(UART0_BASE, 13);
}


//**************************************************************************************************************
// Inicialización de UART
//**************************************************************************************************************
void InitUART(void)
{
    /*Enable the GPIO Port A*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /*Enable the peripheral UART Module 0*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Make the UART pins be peripheral controlled. */
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Sets the configuration of a UART. */
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    //Borramos todas las banderas principales antes de inicializar la interrupción
    UARTIntClear(UART0_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX | UART_INT_FE | UART_INT_PE | UART_INT_BE | UART_INT_OE | UART_INT_RI | UART_INT_CTS | UART_INT_DCD | UART_INT_DSR);
    //Inicializamos la interrupción del uart0
    IntEnable(INT_UART0);
    //Inicializamos las interrupciones del módulo UART
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    //**************************************************************************************************************************************************************************************
    //Inicializar el UART2
    //**************************************************************************************************************************************************************************************
    HWREG(GPIO_PORTD_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;

    HWREG(GPIO_PORTD_BASE+GPIO_O_CR) |= GPIO_PIN_7;

    //Habilitamos el modulo UART2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    //HWREG(GPIO_PORTD_BASE,);

    //Configuramos los puertos D pines 6 y 7 para que los use el uart
    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    /* Sets the configuration of a UART. */
    UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //Borramos todas las banderas principales antes de inicializar la interrupción
    UARTIntClear(UART2_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX | UART_INT_FE | UART_INT_PE | UART_INT_BE | UART_INT_OE | UART_INT_RI | UART_INT_CTS | UART_INT_DCD | UART_INT_DSR);

    //Habilitamos el uart en los pines 6 y 7
    GPIOPinConfigure(GPIO_PD6_U2RX);
    GPIOPinConfigure(GPIO_PD7_U2TX);


}













//**************************************************************************************************************
// Handler de la interrupción del TIMER 0 - Recordar modificar el archivo tm4c123ght6pm_startup_css.c
//**************************************************************************************************************
void Timer0IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // send data every timer interrupt
    send_data=1;

}
















//**************************************************************************************************************
// Handler de la interrupción del UART0 - Recordar modificar el archivo tm4c123ght6pm_startup_css.c
//**************************************************************************************************************
void UARTIntHandler(void){
//acá va la rutina que hace toggle a los leds de la tiva según el caracter que reciba
    //Clear the UART interrupt
    UARTIntClear(UART0_BASE, UART_INT_RX | UART_INT_RT);
    //hacer una serie de ifs que detecten el caracter que se recibió
    receive = UARTCharGet(UART0_BASE);
    UARTCharPut(UART0_BASE, receive); //enviamos el caracter como prueba

}












void setup(void){
    // Se setea oscilador externo de 16MHz
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);  //Reloj configurado a 40MHz
    //*************************************************************************************************************************
    //configuración Dislplay

    // Se asigna reloj a puerto E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Se asigna reloj al puerto D
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);


    // Se establecen como salidas los pines 1, 2 y 3 del puerto E (DISPLAY)
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // Se establecen como salidas los pines 0, 1, 2 y 3 del puerto D (DISPLAY)
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    //*************************************************************************************************************************
    //Configuración Botones
    // Se asigna reloj a puerto B
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);


    //Se configuran los pines 0 y 1 como entradas digitales
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //Configuramos el weak pulldown a 12mA máximo
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPD);


    //Se configuran los pines 4 y 5 del puerto E como entradas
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    //Configuramos el weak pulldown a 12mA máximo
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPD);
    /*
     * BOTON1   PB_0
     * BOTON2   PB_1
     * BOTON3   PE_4
     * BOTON4   PE_5
     */

    //*************************************************************************************************************************
    //Configuración LEDs

    //*************************************************************************************************************************
    // Se asigna reloj a puerto C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    //Se configuran los pines 4, 5, 6 y 7
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    //Se configuran los pines 4, 5, 6 y 7 del puerto B como salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    //El puerto se activó anteriormente para los botones


    //*************************************************************************************************************************

    //Configuración UART

    //*************************************************************************************************************************
    // Se inicializa la comunicación UART
    InitUART();
    //Se envía un mensaje de prueba
    uart_test();


    //*************************************************************************************************************************
    //Configuración del timer 0
    //*************************************************************************************************************************s

    // Se habilita el reloj para el timer 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Configuración del Timer 0 como temporizador períodico
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // Se calcula el período para el temporizador (1 seg) este período es el que se utiliza para actualizar los datos en el servidor web
    ui32Period = (SysCtlClockGet()/4);
    // Establecer el periodo del temporizador
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period - 1);

    // Se habilita la interrupción por el TIMER0A
    IntEnable(INT_TIMER0A );
    // Se establece que exista la interrupción por Timeout
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // Se habilitan las interrupciones Globales
    IntMasterEnable();
    // Se habilita el Timer
    TimerEnable(TIMER0_BASE, TIMER_A);
}




//********************************************************************************************************************************************
//********************************************************************************************************************************************
//              FUNCIONES PARA DIBUJAR NÚMEROS
//********************************************************************************************************************************************
//********************************************************************************************************************************************
void cero(void){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0);
}

void uno(void){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
}

void dos(void){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_1, GPIO_PIN_2 | GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3, 0);
}

void tres(void){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_3, GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_1 | GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_0, 0);
}

void cuatro(void){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_2, GPIO_PIN_3 | GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1 | GPIO_PIN_0, 0);
}


//********************************************************************************************************************************************
//              FUNCION GENERAL DEL DISPLAY
//********************************************************************************************************************************************
void display(unsigned char F){
    switch(F) {

       case 0  :
          cero();
          break;

       case 1  :
          uno();
          break;

       case 2  :
          dos();
          break;

       case 3  :
          tres();
          break;

       case 4  :
          cuatro();
          break;
       //En caso que obtenga un valor que no sea ninguno de los que buscamos
       default :
       cero();
       break;
    }
}











//Procesamos las entradas de los botones
//Convertimos de 32bits a 8 bits
unsigned char buttons_math(uint32_t S){

    switch (S){
    case 0  :
       return 0;

    case 1  :
       return 1;

    case 2  :
       return 2;

    case 3  :
       return 3;

    case 4  :
       return 4;
    //En caso que haya algún error retorna 10
    default :
       return 10;
    }
}








//Se encarga de manejar las luces indicadoras del parqueo
//Luces azules puerto c y luces verdes puertos ____
void luces(void){
    if (boton1==GPIO_PIN_0){
        //encendemos luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 , GPIO_PIN_4);
        //apagamos luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0);

    } else {
        //apagamos luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
        //encendemos luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4);

    }


    if (boton2==GPIO_PIN_1){
        //encendemos la luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);
        //apagamos la luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);

    } else {
        //apagamos la luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
        //encendemos la luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);

    }


    if (boton3==GPIO_PIN_4) {
        //encendemos la luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, GPIO_PIN_6);
        //apgamos la luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0);

    } else {
        //apgamos la luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0);
        //encendemos la luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_PIN_6);

    }


    if (boton4==GPIO_PIN_5) {
        //encendemos la luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);
        //apagamos la luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0);

    } else {
        //apagamos la luz azul
        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);
        //encendemos la luz verde
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_PIN_7);

    }

}








//Función para convertir de numero a caracter
char num2string(uint32_t k){
    switch (k){
    case 0  :
       return '0';

    case 1  :
       return '1';

    case 2  :
       return '2';

    case 3  :
       return '3';

    case 4  :
       return '4';

    default :
       return 'N';
    }
}












//Función de comunicación con el ESP32
void parking_update(void){
    UARTCharPut(UART2_BASE, 'A'); //caracter de incio
    UARTCharPut(UART2_BASE, num2string(estado_boton1)); //1
    UARTCharPut(UART2_BASE, num2string(estado_boton2)); //2
    UARTCharPut(UART2_BASE, num2string(estado_boton3)); //3
    UARTCharPut(UART2_BASE, num2string(estado_boton4)); //4
    UARTCharPut(UART2_BASE, num2string(espacio_disponible)); //5
    UARTCharPut(UART2_BASE, 'F'); //caracter de fin
    //Estos son solamente por estética
    //UARTCharPut(UART2_BASE, 10);
    //UARTCharPut(UART2_BASE, 13); //caracter de fin
    /*
     * Mandamos la A
     * Mandamos 5 datos
     * Mandamos la F
     * Recepción:
     * Se recibe una A
     * Se inicia la comunicación
     * Se reciben 5 datos
     * Se recibe una F
     * Se terminó la comunicación
     */
    //solo para debugear
    /*
    UARTCharPut(UART0_BASE, 'A'); //caracter de incio
    UARTCharPut(UART0_BASE, num2string(estado_boton1)); //1
    UARTCharPut(UART0_BASE, num2string(estado_boton2)); //2
    UARTCharPut(UART0_BASE, num2string(estado_boton3)); //3
    UARTCharPut(UART0_BASE, num2string(estado_boton4)); //4
    UARTCharPut(UART0_BASE, num2string(espacio_disponible)); //5
    UARTCharPut(UART0_BASE, 'F'); //caracter de fin
    //Estos son solamente por estética
    UARTCharPut(UART0_BASE, 10);
    UARTCharPut(UART0_BASE, 13); //caracter de fin
    */
}





/*
 * Notas
 * PD_0, 1, 2 y 3 son salidas del display
 * PE_1, 2 y 3 son salidas del display
 * PC_4, 5, 6 y 7; y PB_ 4, 5, 6 y 7 son las salidas de las LEDs
 * PB_0 y 1; PE_4 y 5 son entradas de botones
 *
 * Por hacer:
 * Configurar UART2
 * Mandar datos por UART2
 * Hacer una rutina que mande un dato de inicio y un dato de final
 * Se va a mandar:
 * Character de inicio
 * Estado boton1
 * Estado boton2
 * Estado boton3
 * Estado boton4
 *
 */
