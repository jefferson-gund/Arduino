#include "EEPROM.h"
#include "cc1101.h"

CC1101 cc1101;

byte b;
byte syncWord = 199;
boolean packetAvailable = false;
byte chan=0;
byte i;
byte dados[32];
byte resposta[32];
byte comandos[16];
byte tamanho;
void vira_direita();
void vira_esquerda();
void anda(int cm1,int pwm);

int e = 0, d = 0;
int cont_e = 0,cont_d = 0;
int leitura_e,leitura_d;
int encod_e, encod_d;
 byte PWM_D=100;
  byte PWM_E=107;
byte CRCH;
byte CRCL;
struct registro {//Registrador de deslocamentos.
  byte DeslocH;
  byte DeslocL;
 
};
registro Desloc[8];

void cc1101signalsInterrupt(void){
// set the flag that a package is available
packetAvailable = true;
}


void setup()
{
    Serial.begin(38400);
    Serial.println("start");
    pinMode(3, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);

// Inicializa configurações do chip RF
    cc1101.init();

    cc1101.setSyncWord(&syncWord, false);
    cc1101.setCarrierFreq(CFREQ_433);
    cc1101.disableAddressCheck();
    //cc1101.setTxPowerAmp(PA_LowPower);

    delay(100);

  attachInterrupt(0, cc1101signalsInterrupt, FALLING);
}

void executa_rota()
{
     for(i=0;i<9;i++)
     {
          if( Desloc[i].DeslocH==0)
           {
                anda(Desloc[i].DeslocL,PWM_D,PWM_E);
            }
            else if(Desloc[i].DeslocH==208)
            {
                 envia_dado();
                 vira_direita();
             }
               else if(Desloc[i].DeslocH==224)
             {   
                   envia_dado();               
                   vira_esquerda();
             }
      }         
}
void send_data() {
  
    int j;

    
   // data.length=32;
    
    if(dados[0]==1)
    {
          switch(dados[1])
          {       
                  
              case 16:
              {
                    if(dados[2]==1)
                    {
                          if(dados[3]==1)
                          {
                              tamanho=8;
                              
                              PWM_D=dados[27];
                              resposta[0]=dados[0];
                              resposta[1]=dados[1];
                              resposta[2]=dados[2];
                              resposta[3]=dados[3];
                              resposta[4]=0;
                              resposta[5]=dados[5];
                              resposta[6]=CRCH; 
                              resposta[7]=CRCL;
                              envia_dado();
                              executa_rota();
                               break;           
                          }
                          else
                          {
                              PWM_E=dados[26];
                              tamanho=8;
                              resposta[0]=dados[0];
                              resposta[1]=dados[1];
                              resposta[2]=dados[2];
                              resposta[3]=dados[3];
                              resposta[4]=0;
                              resposta[5]=dados[5];
                              resposta[6]=CRCH; 
                              resposta[7]=CRCL; 
                              envia_dado();
                              break;
                          }
                    
                    }
                    else
                    {
                            j=dados[3];
                          for(i=dados[3];i<(dados[5]/2)+dados[3];i++)
                          {
                                Desloc[i].DeslocH=dados[j+10];
                                Desloc[i].DeslocL=dados[j+11];
                                j+=2;
                          }
                          resposta[0]=dados[0];
                          resposta[1]=dados[1];
                          resposta[2]=dados[2];
                          resposta[3]=dados[3];
                          resposta[4]=dados[4];              
                          resposta[5]=dados[5];
                          resposta[6]=CRCH; 
                          resposta[7]=CRCL; 
                          tamanho=8;
                          envia_dado();
                          break;
                    }
              }
              case 3:
              {
                    if(dados[2]==1)
                    {
                          if(dados[3]==1)
                          {
                              tamanho=7;
                              
                              resposta[0]=dados[0];
                              resposta[1]=dados[1];
                              resposta[2]=2;
                              resposta[3]=0;
                              resposta[4]=PWM_D;
                              resposta[5]=CRCH; 
                              resposta[6]=CRCL;
                               envia_dado(); 
                              break;           
                          }
                          else
                          {
                              if(dados[7]==1){
                                  tamanho=7;
                                 
                                  resposta[0]=dados[0];
                                  resposta[1]=dados[1];
                                  resposta[2]=2;
                                  resposta[3]=0;
                                  resposta[4]=PWM_E;
                                  resposta[5]=CRCH; 
                                  resposta[6]=CRCL; 
                                  envia_dado();
                                  break;
                              }
                              else{
                                  tamanho=9;
                                 
                                  resposta[0]=dados[0];
                                  resposta[1]=dados[1];
                                  resposta[2]=4;
                                  resposta[3]=0;
                                  resposta[4]=PWM_D;
                                  resposta[5]=0;
                                  resposta[6]=PWM_E;
                                  resposta[7]=CRCH; 
                                  resposta[8]=CRCL; 
                                  envia_dado();
                                  break;
                              }
                          }
                    
                    }
                    else
                    {
                          resposta[0]=dados[0];
                          resposta[1]=dados[1];
                          resposta[2]=2*dados[7];
                          
                          j=dados[3];
                          if(dados[7]<9-dados[3])
                          {  
                              j=0;
                              for(i=dados[3];i<dados[7];i++)
                              {
                                   resposta[3+j]=Desloc[i].DeslocH;
                                   resposta[4+j]=Desloc[i].DeslocL;
                                   j+=2;
                              }
                              resposta[j+3]=CRCH; 
                              resposta[j+4]=CRCL; 
                              tamanho=j+5;
                              envia_dado();
                              break;
                          }
                          else
                          {
                                CRCH=255;
                                CRCL=5;
                          }
                  }                
              }
            
          }      
    }
    else
    {
        Serial.println("2crc not ok");
    }

}
void envia_dado()
{
  CCPACKET data;
  data.length=tamanho;
   for(i=0;i<data.length;i++){
        data.data[i]=resposta[i];
    }
    //cc1101.flushTxFifo ();
    if(cc1101.sendData(data)){
        Serial.println("Enviado");
    }
    else{
        Serial.println("Falha envio!!");
    } 
}

void ReadLQI(){
    byte lqi=0;
    byte value=0;
    lqi=(cc1101.readReg(CC1101_LQI, CC1101_STATUS_REGISTER));
    value = 0x3F - (lqi & 0x3F);
}

void ReadRSSI(){
    byte rssi=0;
    byte value=0;

    rssi=(cc1101.readReg(CC1101_RSSI, CC1101_STATUS_REGISTER));

    if (rssi >= 128){
        value = 255 - rssi;
        value /= 2;
        value += 74;
    }
    else{
        value = rssi/2;
        value += 74;
    }
}


//Escrita dos registradores
void ESCR_REG(){
    for(i=0; i<dados[5];i++){
      comandos[i]=dados[i+10];    
    }
    tamanho = dados[5];
  
}

void le_reg(){
    if(dados[2]==1)
    {
          if(dados[3]==0)
          {
                resposta[3]=dados[26];
                resposta[4]=dados[27];
                resposta[5]=dados[28];
                resposta[6]=dados[29];
          }
          else
          {
                resposta[5]=dados[28];
                resposta[6]=dados[29];
          }
    
    }
    else
    {
          for(i=0;i<17;i++)
          {
                resposta[i+7]=dados[i+10];   
          }
    } 
}
void vira_direita()
{
    Serial.println("virando Direita ");
     delay(500);
  encod_e = 8;
  digitalWrite(6,LOW);
  analogWrite(9,55);
  
  //if(digitalRead(13)==LOW){encod_e = 9;}
  
  while(1){
    leitura_e = digitalRead(8);
    if((leitura_e == HIGH) && (e == 1)){
      cont_e++;
      e = 0;
    }
    
    if(leitura_e == LOW){e = 1;}
      
    if(cont_e > encod_e){
      digitalWrite(9,LOW);
      e = 0;
      cont_e = 0;
      break;
    }
  }
  
  encod_d = 8;
  analogWrite(3,65);
  digitalWrite(5,LOW);
  
  //if(digitalRead(12)==HIGH){encod_d = 9;}
  
  while(1){
    leitura_d = digitalRead(7);
    if((leitura_d == HIGH) && (d == 1)){
      cont_d++;
      d = 0;
    }
    
    if(leitura_d == LOW){d = 1;}
      
    if(cont_d > encod_d){
      digitalWrite(3,LOW);
      d = 0;
      cont_d = 0;
      break;
    }
  }


  //delay(1000);
}

void vira_esquerda()
{
  Serial.println("Virando Esquerda ");
  delay(500);
  encod_e = 8;
  
  // virar carrinho para a direita
  digitalWrite(3,LOW);
  analogWrite(5,65);
  
  //if(digitalRead(12)==LOW){encod_e = 9;}
  
  while(1){
    leitura_e = digitalRead(7);
    if((leitura_e == HIGH) && (e == 1)){
      cont_e++;
      e = 0;
    }
    
    if(leitura_e == LOW){e = 1;}
      
    if(cont_e > encod_e){
      digitalWrite(5,LOW);
      //digitalWrite(10,LOW);
      e = 0;
      cont_e = 0;
      break;
    }
  }
  
  analogWrite(6, 55);
  digitalWrite(9,LOW);
  encod_d = 8;
  
  //if(digitalRead(13)==LOW){encod_d = 9;}
  
  while(1){
    leitura_d = digitalRead(8);
    if((leitura_d == HIGH) && (d == 1)){
      cont_d++;
      d = 0;
    }
    
    if(leitura_d == LOW){d = 1;}
      
    if(cont_d > encod_d){
      //digitalWrite(6,LOW);
      digitalWrite(6,LOW);
      d = 0;
      cont_d = 0;
      break;
    }
  }
  //delay(1000);
  
  
}
void anda(int cm1, int pwmD, int pwmE)
{
  
  int cm=cm1/10;
  int passo = 0;      // variável que indica quantos passos de 10cm o carro anda
  int pwm_plus = pwmE; // variável que ajusta ganho
  
  analogWrite(3,LOW);
  analogWrite(5,pwm_plus);
  analogWrite(6,LOW);
  analogWrite(9,pwmD);
  
  do{
    
    leitura_e = digitalRead(7);
    if((leitura_e == HIGH) && (e == 1)){
      cont_e++;
      e = 0;
    }
    
    if(leitura_e == LOW){e = 1;}
    
    if(cont_e > 7){
      passo++;
      e = 0;
      cont_e = 0;
    }
     envia_dado();
    //send_data();
  }while(passo < cm);
  Serial.println("Andando ");
  Serial.print(cm);
  Serial.println("  cm");
  delay(1000);
  
}

void Exec_Tra()
{    
     int instrucao=0;
      for(i=10;i<25;i+=2)
      {
            if(dados[i]==208)
            {
                vira_direita();
                instrucao++;
            }
            else if(dados[i]==224)
            {
                  vira_esquerda();
            }
            else if(dados[i]==0)
            {
                  anda(dados[i+1],100);
            }
            else
            {
                  resposta[26]=255;//Código de erro do CRC  CRC_H
                  resposta[27]=3;//Numero do erro referente a leitura = 3; CRC_L
            }
      }
  
}

void loop()
{
send_data();

if(packetAvailable){
//Serial.println("packet received");
// Disable wireless reception interrupt
detachInterrupt(0);

ReadRSSI();
ReadLQI();
// clear the flag
packetAvailable = false;

CCPACKET packet;

if(cc1101.receiveData(&packet) > 0){
if(!packet.crc_ok) {
  Serial.println("crc not ok");
}

for(i=0;i<packet.length;i++)
{
     dados[i]= packet.data[i];
}
//dados[0]=1;
resposta[0]=dados[0];
resposta[1]=dados[1];
resposta[2]=dados[5];
resposta[28]=dados[2];
resposta[29]=dados[3];
resposta[30]=dados[6];
resposta[31]=dados[7];
resposta[24]=dados[4];
resposta[25]=dados[5];
Serial.print("\n  dados[1]:  ");
Serial.print(dados[1]);  
Serial.print("\n  ");  
Serial.print(packet.length);  
Serial.print("\n  ");  




}
else
{
  
    Serial.println("4crc not ok");
}

// Enable wireless reception interrupt
attachInterrupt(0, cc1101signalsInterrupt, FALLING);
}
delay(2000);
}
