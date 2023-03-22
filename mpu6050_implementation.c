///Projeto final - MICROPROCESSADORES II///

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "LCD20x4.h"
#include "tecladoM.h"

#define GPIO_PA0_U0RX  0x00000001 //UART
#define GPIO_PA1_U0TX  0x00000401

#define GPIO_PA6_I2C1SCL 0x00001803 //I2C
#define GPIO_PA7_I2C1SDA 0x00001C03


#define SLAVE_ADDRESS_MPU 0x69 //SENSOR_MPU

#define SLAVE_ADDR 0x68  //endereços do DS1307 
#define SEC 0x00    
#define MIN 0x01
#define HRS 0x02
#define DAY 0x03
#define DATE 0x04
#define MONTH 0x05
#define YEAR 0x06
#define CNTRL 0x07

#define END_EEPROM 0x50 //EEPROM
#define High_Addr  0x00

void Config_UART0(void);
void monitoramento(void);
void MENU(void);
void print_float( float var);
void I2C_PORTA(void);
void Configi2c(void);
void recebe_MPU6050(void);
void ends_MPU6050(void);
void WriteEeprom(unsigned char Low_Addr, uint8_t Dado);
uint8_t ReadEeprom(unsigned char Low_Addr);
void LimpaEEPROM(void);
signed char teclado(void);
unsigned char dec2bcd(unsigned char valor);
unsigned char bcd2dec(unsigned char valor);
void SetTime(char hora, char minuto, char segundo);
void SetData(char dia, char mes, char ano);
unsigned char GetData(unsigned char end);
unsigned char x;
unsigned char y1,y2,y3,y4,y5,y6, l_x1,l_x2,l_y1,l_y2,l_z1,l_z2, l_g=0;
int16_t acc_x, acc_y, acc_z;
int16_t temp;
int Clock;
float Y,X,a, Z=0.0;
int X1;
char chato[10];
unsigned char sec, min, hour, date, month, year, sec_a=0;
int flag=0;
int op,voltar;
char al_min_uni,al_min_dez,al_hora_uni,al_hora_dez,Hora,Minutos=0;
char dia_dez,dia_uni,Dia,mes_dez,mes_uni,ano_dez,ano_uni,Mes,Ano=0;
int coor_X,coor_Y,coor_Z;
int choque=0;
int teste=0;
int g=0;
int moni=0;


void main(void)
{
  inicializa();
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN );
  Config_UART0();
  I2C_PORTA();
  
  uint8_t i=7;
  // SetData(26,11,19); // dia,mes,ano ? comentar depois
  // SetTime(20,04,00); // hora,minuto,segundo ? comentar depois
  //UARTprintf("\n\nMódulo MPU - 6050\n");//print teste
  
  ends_MPU6050();
  
  while(1)
  {
    MENU();  
    
    monitoramento();
    
    SysCtlDelay(100000000);
    limpa();
    
    while(choque==0)
    {
      UARTprintf("\x1B[2J\x1B[0;0H");
      if(sec!=sec_a)
      {
        UARTprintf("\r%02d/%02d/%02d - %02d:%02d:%02d\r",date,month,year,hour,min,sec);
        sec_a=sec;
        
      }
      
      recebe_MPU6050();//le dados
      UARTprintf("\n\rCoordenadas: %d,  %d,  %d    Temperatura:%dºC\n ",acc_x,acc_y,acc_z,(temp/340+36));
      
      UARTprintf("\nCoor. X:");
      print_float(X);
      UARTprintf("\nCoor. Y:");         
      print_float(Y);
      UARTprintf("\nCoor. Z:");
      print_float(Z-1);
      
      year = GetData(YEAR);
      month = GetData(MONTH);
      date = GetData(DATE);
      hour = GetData(HRS);
      min = GetData(MIN);
      sec = GetData(SEC);
      
      if(X>=coor_X)
      {
        UARTprintf("\nBATEEEU!"); 
        choque=1;
        teste=1;
      }
      else if(X<=(-coor_X))
      {
        UARTprintf("\nBATEEEU!"); 
        choque=1;
        teste=2;
      }
      
      else if(Y>=coor_Y)
      {
        UARTprintf("\nBATEEEU!"); 
        choque=1;
        teste=3;
      }
      else if(Y<=(-coor_Y))
      {
        UARTprintf("\nBATEEEU!"); 
        choque=1;
        teste=4;
      }
      
      else if((Z-1)>=coor_Z)
      {
        UARTprintf("\nBATEEEU!"); 
        choque=1;
        teste=5;
      }
      else if((Z-1)<=(-coor_Z))
      {
        UARTprintf("\nBATEEEU!"); 
        choque=1;
        teste=6;
      }
      if(choque==1)
      {
        WriteEeprom(0,hour);
        WriteEeprom(1,min);
        WriteEeprom(2,sec);
        WriteEeprom(3,date);
        SysCtlDelay(10000000); 
        WriteEeprom(4,month);
        SysCtlDelay(10000000); 
        WriteEeprom(5,year);
        SysCtlDelay(10000000); 
       
      UARTprintf("\n\n-----gravou-----" );  
      }
      escreve (0x80, "MONITORANDO...");
      SysCtlDelay(5000000); 
    }    
    limpa();
    choque=0;
    
    UARTprintf("\nTARDE DE MAIS"); 
    
    if(teste==1)
    {
      g=coor_X;
      UARTprintf("\n A colisão ocorreu com forca de %d g!",g);
      UARTprintf("\n EIXO X!");
      escreve (0x80, "Colisao no eixo X!");
      escreve_char (0xC4,  (g+0x30));
    }
    else if(teste==2)
    {
      g=(-coor_X);
      UARTprintf("\n A colisão ocorreu com forca de %d g!",g);
      UARTprintf("\n EIXO X!");
      escreve (0x80, "Colisao no eixo X!");
      escreve (0xC3, "-");
      escreve_char (0xC4, (fabs(g)+0x30)); 
    }
    else if(teste==3)
    {
      g=(coor_Y);
      UARTprintf("\n A colisão ocorreu com forca de %d g!",g);
      UARTprintf("\n EIXO Y!");
      escreve (0x80, "Colisao no eixo Y!");
      escreve_char (0xC4,  (g+0x30)); 
    }
    else if(teste==4)
    {
      g=(-coor_Y);
      UARTprintf("\n A colisão ocorreu com forca de %d g!",g);
      UARTprintf("\n EIXO Y!");
      escreve (0x80, "Colisao no eixo Y!");
      escreve (0xC3, "-");
      escreve_char (0xC4,  (fabs(g)+0x30));
    }
    else if(teste==5)
    {
      g=(coor_Z);
      UARTprintf("\n A colisão ocorreu com forca de %d g!",g);
      UARTprintf("\n EIXO Z!");
      escreve (0x80, "Colisao no eixo Z!");
      escreve_char (0xC4,  (g+0x30));
    } 
    else if(teste==6)
    {
      g=(-coor_Z);
      UARTprintf("\n A colisão ocorreu com forca de %d g!",g);
      UARTprintf("\n EIXO Z!"); 
      escreve (0x80, "Colisao no eixo Z!");
      escreve (0xC3, "-");
      escreve_char (0xC4,  (fabs(g)+0x30)); 
    } 
    WriteEeprom(6,fabs(g));
    SysCtlDelay(100000000);
  }  
}

void Config_UART0(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
  UARTStdioConfig(0, 115200, 16000000);
}

void ends_MPU6050(void)
{
  //ativação MPU6050
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_MPU, false); //escrita
  I2CMasterDataPut(I2C1_BASE, 0x6B); //end. inicial
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
  while(I2CMasterBusy(I2C1_BASE));
  I2CMasterDataPut(I2C1_BASE, 0x00);//Reset MPU
  
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
  while(I2CMasterBusy(I2C1_BASE));
  
  //config acelerometro: sensibilidade p/(+-4g)
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_MPU, false); 
  I2CMasterDataPut(I2C1_BASE, 0x1C); //end. inicial//registro correspondente ao acelerometro
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
  while(I2CMasterBusy(I2C1_BASE));
  I2CMasterDataPut(I2C1_BASE, 0x08);//valor calculado para os 4g
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); //stop
  while(I2CMasterBusy(I2C1_BASE));
}

void recebe_MPU6050(void) 
{
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_MPU, false);
  I2CMasterDataPut(I2C1_BASE, 0x3B);//parte alta coor.X
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
  while(I2CMasterBusy(I2C1_BASE));
  
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_MPU, true); //LEITURA
  
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
  while(I2CMasterBusy(I2C1_BASE));
  //CAPTURA 16 BITS
  ///coordenada X///
  acc_x = I2CMasterDataGet(I2C1_BASE);//VALOR BRUTO
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
  while(I2CMasterBusy(I2C1_BASE));
  acc_x = (acc_x<<8)|I2CMasterDataGet(I2C1_BASE);//PARTE BAIXA DOS VALORES
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
  
  X=(float)acc_x;
  X=X/8192;            //calculo para transformar em g/ sens. em 4g
  
  while(I2CMasterBusy(I2C1_BASE));
  
   ///coordenada Y///
  acc_y = I2CMasterDataGet(I2C1_BASE);
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
  while(I2CMasterBusy(I2C1_BASE));
  acc_y = (acc_y<<8)|I2CMasterDataGet(I2C1_BASE);
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
  
  Y=(float)acc_y;
  Y=Y/8192;
  
  while(I2CMasterBusy(I2C1_BASE));
  
   ///coordenada Z///
  acc_z = I2CMasterDataGet(I2C1_BASE);
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
  while(I2CMasterBusy(I2C1_BASE));
  acc_z = (acc_z<<8)|I2CMasterDataGet(I2C1_BASE);
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
  
  Z=(float)acc_z;
  Z=Z/8192;
  
  while(I2CMasterBusy(I2C1_BASE)); 
  
  ///CONFIG. TEMPERATURA///
  temp = I2CMasterDataGet(I2C1_BASE);
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
  while(I2CMasterBusy(I2C1_BASE));
  temp = (temp<<8)|I2CMasterDataGet(I2C1_BASE);
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
  while(I2CMasterBusy(I2C1_BASE));
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH); //stop
  while(I2CMasterBusy(I2C1_BASE));
}

void I2C_PORTA(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA6_I2C1SCL);
  GPIOPinConfigure(GPIO_PA7_I2C1SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
  GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
  I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);
}
//transforma um número do formato decimal para BCD
unsigned char dec2bcd(unsigned char valor)
{
  return (((valor/10)<<4) | (valor%10));
}
//transforma um número do formato BCD para decimal
unsigned char bcd2dec(unsigned char valor)
{
  return (((valor&0xF0)>>4)*10) + (valor&0x0F);
}

void SetTime(char hora, char minuto, char segundo)
{
  uint8_t vet[3];
  vet[0]=dec2bcd(segundo);
  vet[1]=dec2bcd(minuto);
  vet[2]=dec2bcd(hora);
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, false); //end. do dispositivo, R/S=0
  I2CMasterDataPut(I2C1_BASE, SEC); //end. inicial de gravação SEC=0x00
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START); //start
  while(I2CMasterBusy(I2C1_BASE));
  for(char i=0; i<3; i++)
  {
    I2CMasterDataPut(I2C1_BASE, vet[i]);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); //envia
    while(I2CMasterBusy(I2C1_BASE));
  }
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); //stop
  while(I2CMasterBusy(I2C1_BASE));
}

void SetData(char dia, char mes, char ano)
{
  uint8_t vet[3];
  vet[0]=dec2bcd(dia);
  vet[1]=dec2bcd(mes);
  vet[2]=dec2bcd(ano);
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, false); //end. do dispositivo, R/S=0
  I2CMasterDataPut(I2C1_BASE, DATE); //end. inicial de gravação DATE=0x04
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START); //start
  while(I2CMasterBusy(I2C1_BASE));
  for(char i=0; i<3; i++)
  {
    I2CMasterDataPut(I2C1_BASE, vet[i]);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); //envia
    while(I2CMasterBusy(I2C1_BASE));
  }
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); //stop
  while(I2CMasterBusy(I2C1_BASE));
}

unsigned char GetData(unsigned char end)
{
  unsigned char dado;
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, false); //R/S=0 (Send)
  I2CMasterDataPut(I2C1_BASE, end); //end. Memória RAM do RTC
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
  while(I2CMasterBusy(I2C1_BASE));
  I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, true); //R/S=1 (Read)
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE); //sel. recepção
  while(I2CMasterBusy(I2C1_BASE));
  dado=I2CMasterDataGet(I2C1_BASE); //leitura
  return bcd2dec(dado);
}
void WriteEeprom(unsigned char Low_Addr, unsigned char Dado)// Escrita EEPROM 
{
  for(uint32_t i=0;i<=50000;i++);
  
  I2CMasterSlaveAddrSet(I2C1_BASE, END_EEPROM, false);//escrita
  I2CMasterDataPut(I2C1_BASE, High_Addr);//parte alta em 0
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
  while(I2CMasterBusy(I2C1_BASE));
  
  I2CMasterDataPut(I2C1_BASE, Low_Addr); //end. desejado
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);  
  while(I2CMasterBusy(I2C1_BASE));
  
  I2CMasterDataPut(I2C1_BASE, Dado); //valor desejado
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
  while(I2CMasterBusy(I2C1_BASE));
}

uint8_t ReadEeprom(unsigned char Low_Addr)// Leitura EEPROM
{
  for(uint32_t i=0;i<=50000;i++);
  unsigned char Dado; 
  
  I2CMasterSlaveAddrSet(I2C1_BASE, END_EEPROM, false);// End. do dispositivo, R/S=0
  
  I2CMasterDataPut(I2C1_BASE, High_Addr); 
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  while(I2CMasterBusy(I2C1_BASE));
  I2CMasterDataPut(I2C1_BASE, Low_Addr); 
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(I2CMasterBusy(I2C1_BASE));
  
  I2CMasterSlaveAddrSet(I2C1_BASE, END_EEPROM, true);        
  I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE); 
  while(I2CMasterBusy(I2C1_BASE));
  Dado=I2CMasterDataGet(I2C1_BASE);                          
  
  return Dado;
}

uint32_t UARTDecGet(uint32_t ui32Base)
{
  uint32_t numero=0,tamanho=0;
  char caractere;
  
  caractere= UARTCharGet(ui32Base);
  while(caractere!= 0x0D)//CR - ENTER
  {
    if((caractere>='0')&&(caractere<='9'))
    {
      numero=10*numero+(caractere-0x30);
      tamanho++;
      UARTCharPut(ui32Base, caractere);
    }
    else if((caractere==0x08)&&(tamanho!=0))//backspace
    {
      numero/=10;
      tamanho--;
      UARTCharPut(ui32Base, caractere);
    }
    caractere= UARTCharGet(ui32Base);
  }
  return numero;
}
void print_float( float var) //para valores float - MPU
{
  int x1=0,x2=0,x3=0;
  x1=(int)var;
  if(x1!= 0)
  {
    x2=(var*10)-((int)var)*10;
    x2=fabs(x2);
    x3=(var*100)-(int)var*100;
    x3=fabs(x3);
    UARTprintf("%d.%d%d\n",x1,x2,x3);
    
  }
  else
  {
    x1=0;
    x2=(var*10);
    x2=fabs(x2);
    x3=(var*100)-(int)var*100;
    x3=fabs(x3);
    if(var<0)
    {
      UARTprintf("-%d.%d%d\n",x1,x2,x3);
    }
    else
    {
      UARTprintf("%d.%d%d\n",x1,x2,x3);
    }
    
  } 
}

void monitoramento(void)
{
  moni=0;
  do
  {
    escreve (0x80, "Digite os pontos da colisao:");
    SysCtlDelay(10000000);
    limpa();
    escreve (0x80, "Digite o valor para a coord. X(1~4):");
    coor_X=teclado();
    limpa();
    switch (coor_X)
    {
    case 1:
      escreve (0x80, "Voce escolheu 1 g!");
      break;
    case 2:
      escreve (0x80, "Voce escolheu 2 g!");
      break;
    case 3:
      escreve (0x80, "Voce escolheu 3 g!");
      break;
    case 4:
      escreve (0x80, "Voce escolheu 4 g!");
      break;
      SysCtlDelay(10000000);
    }
    SysCtlDelay(10000000);
    limpa();
    escreve (0x80, "Digite o valor para a coord. Y(1~4):");
    coor_Y=teclado();
    SysCtlDelay(10000000);
    limpa();
    switch (coor_Y)
    {
      limpa();
    case 1:
      escreve (0x80, "Voce escolheu 1 g!");
      break;
    case 2:
      escreve (0x80, "Voce escolheu 2 g!");
      break;
    case 3:
      escreve (0x80, "Voce escolheu 3 g!");
      break;
    case 4:
      escreve (0x80, "Voce escolheu 4 g!");
      break;
      SysCtlDelay(10000000);
    }
    SysCtlDelay(10000000);
    limpa();
    escreve (0x80, "Digite o valor para a coord. Z(1~4):");
    coor_Z=teclado();
    SysCtlDelay(10000000);
    limpa();
    switch (coor_Z)
    {
      limpa();
    case 1:
      escreve (0x80, "Voce escolheu 1 g!");
      break;
    case 2:
      escreve (0x80, "Voce escolheu 2 g!");
      break;
    case 3:
      escreve (0x80, "Voce escolheu 3 g!");
      break;
    case 4:
      escreve (0x80, "Voce escolheu 4 g!");
      break;
      
    }
    
    UARTprintf("\n%d,%d,%d",coor_X,coor_Y,coor_Z);
  }
  while((coor_X<1 || coor_X>4)||(coor_Y<1 || coor_Y>4)||(coor_X<1 || coor_Z>4));
}
void MENU(void)
{
  
  while(moni==0)
  {
    limpa();
    escreve (0x80, "Digite:");
    escreve (0xC0, "1 - Ler EEPROM");
    escreve (0x94, "2 - Monitoramento");
    escreve(0xD4,"3 - Alt. data/hora");
    op=teclado();
    SysCtlDelay(10000000);
    switch (op)
    {
      limpa();
    case 1:
      limpa();
      escreve (0x80, "1 - Ler EEPROM");
      y1=ReadEeprom(0);  
      y2=ReadEeprom(1);  
      y3=ReadEeprom(2);
      SysCtlDelay(10000000);  
      UARTprintf("\nHora e dia da última batida:");
      UARTprintf("\n%d:%d:%d",y1,y2,y3);
      y4=ReadEeprom(3);  
      y5=ReadEeprom(4);  
      y6=ReadEeprom(5);
      SysCtlDelay(10000000); 
      
      
      UARTprintf("---------%d/%d/%d",y4,y5,y6);
      l_g=(ReadEeprom(6)); 
      SysCtlDelay(10000000); 
      if(teste==1)
      {
        
        UARTprintf("\nBatida de %d g",l_g);
        UARTprintf("\n EIXO X!"); 
        escreve (0x80, "Colisao no eixo X!");
        
        escreve(0xC0,"Batida de    g");
        escreve_char (0xCB,  (l_g+0x30));
      } 
      else if(teste==2)
      { 
        UARTprintf("\nBatida de -%d g",l_g);     
        UARTprintf("\n EIXO X!"); 
        escreve (0x80, "Colisao no eixo X!");
        escreve(0xC0,"Batida de -   g");
        escreve_char (0xCB,  (l_g+0x30));
      }
      else if(teste==3)
      {
        
        UARTprintf("\nBatida de %d g",l_g);
        UARTprintf("\n EIXO Y!"); 
        escreve (0x80, "Colisao no eixo Y!");
        
        escreve(0xC0,"Batida de    g");
        escreve_char (0xCB,  (l_g+0x30));
      } 
      else if(teste==4)
      { 
        UARTprintf("\nBatida de -%d g",l_g);     
        UARTprintf("\n EIXO Y!"); 
        escreve (0x80, "Colisao no eixo Y!");
        escreve(0xC0,"Batida de -   g");
        escreve_char (0xCB,  (l_g+0x30));
      }
      else if(teste==5)
      {
        
        UARTprintf("\nBatida de %d g",l_g);
        UARTprintf("\n EIXO Z!"); 
        escreve (0x80, "Colisao no eixo Z!");
        escreve(0xC0,"Batida de    g");
        escreve_char (0xCB,  (l_g+0x30));
      } 
      else if(teste==6)
      { 
        UARTprintf("\nBatida de -%d g",l_g);     
        UARTprintf("\n EIXO Z!"); 
        escreve (0x80, "Colisao no eixo Z!");
        escreve(0xC0,"Batida de -   g");
        escreve_char (0xCB,  (l_g+0x30));
      }
      escreve_char (0x94,((y1/10)+0x30));
      escreve_char (0x95,((y1%10)+0x30));
      escreve (0x96, ":");
      escreve_char (0x97,((y2/10)+0x30));
      escreve_char (0x98,((y2%10)+0x30));
      escreve (0x99, ":");
      escreve_char (0x9A,((y3/10)+0x30));
      escreve_char (0x9B,((y3%10)+0x30));
      
      
      escreve_char (0xD4,((y4/10)+0x30));
      escreve_char (0xD5,((y4%10)+0x30));
      escreve (0xD6, "/");
      escreve_char (0xD7,((y5/10)+0x30));
      escreve_char (0xD8,((y5%10)+0x30));
      escreve (0xD9, "/");
      escreve_char (0xDA,((y6/10)+0x30));
      escreve_char (0xDB,((y6%10)+0x30));
      SysCtlDelay(200000000);   
      
      break;
    case 2:
      escreve (0x80, "2- Monitoramento de novas colisoes");
      moni=1;
      break;
    case 3:
      escreve(0x80,"3 - Alterar data/hora");
      SysCtlDelay(10000000); 
      limpa();
      
      escreve (0x80, "Horas:");     
    //HORA - 24 hrs
      al_hora_dez=teclado();
      SysCtlDelay(10000000); 
      al_hora_dez=al_hora_dez+0x30;
      while( al_hora_dez>2+0x30)
      {
        limpa();
        escreve (0x80, "Horas:"); 
        SysCtlDelay(10000000);
        al_hora_dez=teclado();
        SysCtlDelay(10000000);
        
        al_hora_dez= al_hora_dez+0x30;
      }
      
      escreve_char (0x86,  al_hora_dez);
      al_hora_uni=teclado();
      SysCtlDelay(10000000);
      al_hora_uni=al_hora_uni+0x30;
      while( al_hora_uni>3+0x30 &&al_hora_dez==2+0x30) //23 hrs
      {
        al_hora_uni=teclado();
        SysCtlDelay(10000000);
        al_hora_uni= al_hora_uni+0x30;
      }
      escreve_char (0x87,  al_hora_uni);
      
      SysCtlDelay(10000000);
      if(al_hora_dez==0+0x30)
      {
        al_hora_uni=al_hora_uni-0x30;
        Hora=al_hora_uni;
      }
      if(al_hora_dez==1+0x30)                                     
      {
        al_hora_dez=al_hora_dez-0x30;
        al_hora_uni=al_hora_uni-0x30;
        Hora=al_hora_dez+9+al_hora_uni;
      }
      else if(al_hora_dez==2+0x30) 
      {
        al_hora_dez=al_hora_dez-0x30;
        al_hora_uni=al_hora_uni-0x30;
        Hora=al_hora_dez+18+al_hora_uni;
      }
      
      // MIN
      
      escreve (0xC0, "Minutos:");
      al_min_dez=teclado();
      SysCtlDelay(10000000);
      al_min_dez=al_min_dez+0x30;
      while(al_min_dez>=6+0x30) //60 min
      {
        al_min_dez=teclado();
        SysCtlDelay(10000000);
        al_min_dez=al_min_dez+0x30;
      }
      
      escreve_char (0xC8, al_min_dez);
      al_min_uni=teclado();
      SysCtlDelay(10000000);
      al_min_uni=al_min_uni+0x30;
      escreve_char (0xC9, al_min_uni);
      SysCtlDelay(10000000);
      if(al_min_dez==0+0x30)
      {
        al_min_uni=al_min_uni-0x30;
        Minutos=al_min_uni;
      }                 
      if(al_min_dez==1+0x30)
      {
        al_min_dez=al_min_dez-0x30;
        al_min_uni=al_min_uni-0x30;
        Minutos=al_min_dez+9+al_min_uni;
      }
      else if(al_min_dez==2+0x30){al_min_dez=al_min_dez-0x30;al_min_uni=al_min_uni-0x30;Minutos=al_min_dez+18+al_min_uni;}     
      else if(al_min_dez==3+0x30){al_min_dez=al_min_dez-0x30;al_min_uni=al_min_uni-0x30;Minutos=al_min_dez+27+al_min_uni;}  
      else if(al_min_dez==4+0x30){al_min_dez=al_min_dez-0x30;al_min_uni=al_min_uni-0x30;Minutos=al_min_dez+36+al_min_uni;}   
      else if(al_min_dez==5+0x30){al_min_dez=al_min_dez-0x30;al_min_uni=al_min_uni-0x30;Minutos=al_min_dez+45+al_min_uni;}
      
      SetTime(Hora,Minutos,0); 
      
      limpa();
      escreve (0x80, "Dia:");  
      // Aguarda usuário digitar valor do Dia
      dia_dez=teclado();
      SysCtlDelay(10000000);               
      dia_dez=dia_dez+0x30;
      while(dia_dez>3+0x30) //limite para 31 dias
      {
        dia_dez=teclado();
        SysCtlDelay(10000000); 
        dia_dez=dia_dez+0x30;
      }  
      
      escreve_char (0x84, dia_dez);
      SysCtlDelay(10000000); 
      if(dia_dez==3+0x30)                                        
      {
        dia_uni=teclado();
        SysCtlDelay(10000000); 
        dia_uni=dia_uni+0x30;
        while(dia_uni>1+0x30)
        {
          dia_uni=teclado();
          SysCtlDelay(10000000); 
          dia_uni=dia_uni+0x30;
        }
        escreve_char (0x85, dia_uni);
        SysCtlDelay(10000000); 
        dia_dez=dia_dez-0x30;
        dia_uni=dia_uni-0x30;
        Dia=(dia_dez*10)+dia_uni;
      }
      else
      {
        dia_uni=teclado();
        SysCtlDelay(10000000); 
        dia_uni=dia_uni+0x30;
        escreve_char (0x85, dia_uni);
        SysCtlDelay(10000000);
        dia_dez=dia_dez-0x30;
        dia_uni=dia_uni-0x30;
        Dia=(dia_dez*10)+dia_uni;
      }
      
      // MES
      
      escreve (0xC0, "Mes:");                               
      mes_dez=teclado();
      SysCtlDelay(10000000);
      mes_dez= mes_dez+0x30;
      while( mes_dez>1+0x30) //12 meses
      {
        mes_dez=teclado();
        SysCtlDelay(10000000);
        mes_dez= mes_dez+0x30;
      }
      escreve_char (0xC4,  mes_dez);
      SysCtlDelay(10000000);
      mes_uni=teclado();
      SysCtlDelay(10000000);
      mes_uni= mes_uni+0x30;
      while( mes_uni>2+0x30&&mes_dez==1+0x30)
      { 
        mes_uni=teclado();
        SysCtlDelay(10000000);
        mes_uni= mes_uni+0x30;
      }   
     
      escreve_char (0xC5,  mes_uni); 
      SysCtlDelay(10000000);
      mes_dez=mes_dez-0x30;
      mes_uni= mes_uni-0x30;
      Mes=(mes_dez*10)+ mes_uni;                                         
      
      //ANO
      
      escreve (0x94, "Ano:");
      ano_dez=teclado();
      SysCtlDelay(10000000);
      ano_dez= ano_dez+0x30;
      escreve_char (0x98,  ano_dez);
      SysCtlDelay(10000000);
      ano_uni=teclado();
      SysCtlDelay(10000000);
      ano_uni=ano_uni+0x30;
      escreve_char (0x99, ano_uni);
      SysCtlDelay(10000000);
      ano_dez= ano_dez-0x30;
      ano_uni=ano_uni-0x30;
      Ano=( ano_dez*10)+ano_uni;
     
      SetData(Dia,Mes,Ano);
      break; 
    }
  }
}  