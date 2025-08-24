#include <xc.h>

// ================= CONFIG =================
#pragma config FOSC = HS  // HS oscillator
#pragma config WDTE = OFF // Watchdog Timer Disable
#pragma config PWRTE = ON // Power-up Timer Enable
#pragma config BOREN = ON // Brown-out Reset Enable
#pragma config LVP = OFF  // Low-Voltage Programming Disable
#pragma config CPD = OFF  // Data EEPROM Code Protection off
#pragma config WRT = OFF  // Flash Program Memory Write Enable off
#pragma config CP = OFF   // Flash Program Memory Code Protection off

#define _XTAL_FREQ 20000000 // 20 MHz crystal

// ================= LCD PINS =================
#define RS RD0
#define EN RD1
#define D4 RD2
#define D5 RD3
#define D6 RD4
#define D7 RD5

// ================= I2C =================
void I2C_Init(void)
{
    SSPCON = 0x28;
    SSPADD = ((_XTAL_FREQ / 4) / 100000) - 1;
    SSPSTAT = 0;
}

void I2C_Start(void)
{
    SEN = 1;
    while (SEN)
        ;
}

void I2C_Stop(void)
{
    PEN = 1;
    while (PEN)
        ;
}

void I2C_Write(unsigned char data)
{
    SSPBUF = data;
    while (BF)
        ;
    while (SSPSTATbits.R_nW)
        ;
}

unsigned char I2C_Read(unsigned char ack)
{
    unsigned char data;
    RCEN = 1;
    while (!BF)
        ;
    data = SSPBUF;
    ACKDT = (ack) ? 0 : 1;
    ACKEN = 1;
    while (ACKEN)
        ;
    return data;
}

// ================= DS1307 =================
#define DS1307_ADDRESS 0xD0

void DS1307_Write(unsigned char addr, unsigned char data)
{
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(addr);
    I2C_Write(data);
    I2C_Stop();
}

unsigned char DS1307_Read(unsigned char addr)
{
    unsigned char data;
    I2C_Start();
    I2C_Write(DS1307_ADDRESS);
    I2C_Write(addr);
    I2C_Start();
    I2C_Write(DS1307_ADDRESS | 1);
    data = I2C_Read(0);
    I2C_Stop();
    return data;
}

// ================= LCD =================
void LCD_Command(unsigned char cmnd)
{
    RS = 0;
    D4 = (cmnd & 0x10) >> 4;
    D5 = (cmnd & 0x20) >> 5;
    D6 = (cmnd & 0x40) >> 6;
    D7 = (cmnd & 0x80) >> 7;
    EN = 1;
    __delay_us(10);
    EN = 0;

    D4 = (cmnd & 0x01);
    D5 = (cmnd & 0x02) >> 1;
    D6 = (cmnd & 0x04) >> 2;
    D7 = (cmnd & 0x08) >> 3;
    EN = 1;
    __delay_us(10);
    EN = 0;
    __delay_ms(2);
}

void LCD_Char(unsigned char data)
{
    RS = 1;
    D4 = (data & 0x10) >> 4;
    D5 = (data & 0x20) >> 5;
    D6 = (data & 0x40) >> 6;
    D7 = (data & 0x80) >> 7;
    EN = 1;
    __delay_us(10);
    EN = 0;

    D4 = (data & 0x01);
    D5 = (data & 0x02) >> 1;
    D6 = (data & 0x04) >> 2;
    D7 = (data & 0x08) >> 3;
    EN = 1;
    __delay_us(10);
    EN = 0;
    __delay_ms(2);
}

void LCD_Init()
{
    TRISD = 0x00;
    __delay_ms(20);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
}

void LCD_Clear()
{
    LCD_Command(0x01);
    __delay_ms(2);
}

void LCD_Set_Cursor(unsigned char row, unsigned char column)
{
    unsigned char pos;
    if (row == 1)
        pos = 0x80 + column - 1;
    else if (row == 2)
        pos = 0xC0 + column - 1;
    LCD_Command(pos);
}

void LCD_Write_String(char *str)
{
    while (*str)
        LCD_Char(*str++);
}

// ================= MAIN =================
void main(void)
{
    TRISD = 0x00; // LCD output
    I2C_Init();
    LCD_Init();

    LCD_Set_Cursor(1, 1);
    LCD_Write_String("Automotive Relay");
    __delay_ms(2000);

    while (1)
    {
        unsigned char sec = DS1307_Read(0x00); // Read seconds register
        unsigned char min = DS1307_Read(0x01); // Read minutes
        unsigned char hr = DS1307_Read(0x02);  // Read hours

        LCD_Clear();
        LCD_Set_Cursor(1, 1);
        LCD_Write_String("Time:");
        LCD_Char((hr / 10) + '0');
        LCD_Char((hr % 10) + '0');
        LCD_Char(':');
        LCD_Char((min / 10) + '0');
        LCD_Char((min % 10) + '0');
        LCD_Char(':');
        LCD_Char((sec / 10) + '0');
        LCD_Char((sec % 10) + '0');
        __delay_ms(1000);
    }
}
