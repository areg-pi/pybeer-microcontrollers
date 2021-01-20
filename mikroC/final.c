/*
        Universidad de Sonora
        Ingenieria Mecatrónica
        Microcontroladores: Práctica 2
        Equipo 6:
               Cisneros Padilla Andrés
               López Contreras Eduardo
               Navalles Colores Moisés Gerardo
               Rubio Garcia Francisco
        Modelo uC: PIC16F877a
        Descripción: proyecto final
*/

// LCD module connections
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RB0_bit;
sbit LCD_D5 at RB1_bit;
sbit LCD_D6 at RB2_bit;
sbit LCD_D7 at RB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;

//  Set temperature resolution to the corresponding resolution of used DS18x20 sensor:
//  18S20: 9  (default setting; can be 9,10,11,or 12)
//  18B20: 12
const unsigned short TEMP_RESOLUTION = 12;

// Variables
char *text = "000.0000";       // Para temperatura
unsigned temp;
char uart_rd, temperature;     // Para UART y temperatura
unsigned int adc_rd1;          // Guarda lectura de canales
//char *pH = "00.00";
char pH[7];                    // Para la escala en volts
float x, y, f_temp;            // Para la ecuación de pH
float m = -5.544, b = 21.66;

// Function to convert each value of temperature
char Convert_Temperature(unsigned int temp2write);


char Convert_Temperature(unsigned int temp2write) {
    const unsigned short RES_SHIFT = TEMP_RESOLUTION - 8;
    char temp_whole;
    unsigned int temp_fraction;

    // Check if temperature is negative
    if (temp2write & 0x8000) {
        text[0] = '-';
        temp2write = ~temp2write + 1;
    }

    // Extract temp_whole
    temp_whole = temp2write >> RES_SHIFT ;

    // Convert temp_whole to characters
    if (temp_whole/100)
        text[0] = temp_whole/100  + 48;
    else
        text[0] = '0';

    text[1] = (temp_whole/10)%10 + 48;             // Extract tens digit
    text[2] =  temp_whole%10     + 48;             // Extract ones digit

    // Extract temp_fraction and convert it to unsigned int
    temp_fraction  = temp2write << (4-RES_SHIFT);
    temp_fraction &= 0x000F;
    temp_fraction *= 625;

    // Convert temp_fraction to characters
    text[4] =  temp_fraction/1000    + 48;         // Extract thousands digit
    text[5] = (temp_fraction/100)%10 + 48;         // Extract hundreds digit
    text[6] = (temp_fraction/10)%10  + 48;         // Extract tens digit
    text[7] =  temp_fraction%10      + 48;         // Extract ones digit

    temperature = text;              // Gets temperature as str
    Lcd_out(2,1,temperature);        // Prints on LCD
    f_temp = atof(temperature);      // Converts from str to float
    return temperature, f_temp;
  }

void main() {

    ADCON1 = 0b0100;                // 3 análogos en el 16f87x
    UART1_Init(9600);               // Initialize UART module at 9600 bps
    Delay_ms(100);                  // Wait for UART module to stabilize

    // Entradas análogas
    TRISA = 0xFF;                   // PORT A as input
    TRISB = 0;                      // PORT B as output
    TRISC = 0;                      // PORT C as output
    PORTB = 0;
    PORTC = 0;
    TRISC6_bit = 0;                 // Salida TX
    TRISC7_bit = 1;                 // Entrada RX
    Lcd_Init();                     // Inicializar LCD
    Lcd_Cmd(_LCD_CLEAR);            // Limpiar LCD
    Lcd_Cmd(_LCD_CURSOR_OFF);       // Apagar cursor
    Lcd_Out(1, 1, " Temp:      pH:");

    Lcd_Chr(2,9,223);           // Mostrar caracter de grados °.
    Lcd_Chr(2,10,'C');


    while (1) {
        
        //Lectura de pH y conversión ADC
        adc_rd1 = ADC_Read(2);
        x = (float)adc_rd1*5/1023;
        
        // Almacenamiento de datos en ecuación de pH con offset
        y = m * x + b;
        y = y - 1.2;
        FloatToStr(y, pH);         // Conversión de str a float
        Lcd_out(2,12,pH);          // Print pH on LCD

// Perform temperature reading with One Wire library
        Ow_Reset(&PORTA, 4);                  // Onewire reset signal
        Ow_Write(&PORTA, 4, 0xCC);            // Issue command SKIP_ROM
        Ow_Write(&PORTA, 4, 0x44);            // Issue command CONVERT_T
        Delay_us(120);

        Ow_Reset(&PORTA, 4);
        Ow_Write(&PORTA, 4, 0xCC);            // Issue command SKIP_ROM
        Ow_Write(&PORTA, 4, 0xBE);            // Issue command READ_SCRATCHPAD

        // Read from PORT A and store it
        temp =  Ow_Read(&PORTA, 4);
        temp = (Ow_Read(&PORTA, 4) << 8) + temp;
        
        // Give parameter/value of temperature to function
        temp = temp - 13.944;
        Convert_Temperature(temp);

        // Buzzer condition
        if(f_temp <= 15){
            PORTC.F5 = 1;
        }
        else{
            PORTC.F5 = 0;
        }
        
        Delay_ms(500);

        // Envio de datos por UART y printear en terminal
        UART1_Write_Text("Temperature: ");
        UART1_Write_Text(temperature);
        UART1_Write(temperature);
        UART1_Write(10);
        UART1_Write(13);
        UART1_Write_Text("pH: ");
        UART1_Write_Text(pH);
        UART1_Write(pH);
        UART1_Write(10);
        UART1_Write(13);
        
        UART1_Write(10);
        UART1_Write(13);
    }
}