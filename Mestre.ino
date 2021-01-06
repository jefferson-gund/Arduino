#include "EEPROM.h"
#include "cc1101.h"

CC1101 cc1101;


// counter to get increment in each loop
byte counter;
byte b;
byte syncWord = 199;
boolean packetAvailable = false;
byte chan=0;
byte i;
byte CRC_H;
byte CRC_L;

int tamanho=16;  //Define o tamanho do número de bytes dados.

byte resposta[32];//Recebe a resposta do escravo.

struct registro {//Registrador de deslocamentos.
  byte DeslocH;
  byte DeslocL;
 
};
registro Desloc[8];
 // byte PWM_D=100;
 //byte PWM_E=107;
byte dados[32];//Pergunta do mestre.

void cc1101signalsInterrupt(void){//Função de interrupção da biblioteca do módulo RF1100SE.
// set the flag that a package is available
packetAvailable = true;
}


void setup(){
    Serial.begin(38400);
    Serial.println("start");
// --------------------------CONFIGURAÇÃO DA TRAGETÓRIA-----------------------------------
   tamanho=16; //tamanho de bytes está definido no início do código!
    
    Desloc[0].DeslocH=0;
    Desloc[0].DeslocL=100;
    Desloc[1].DeslocH=208;
    Desloc[1].DeslocL=0;
    Desloc[2].DeslocH=0;
    Desloc[2].DeslocL=100;
    Desloc[3].DeslocH=208;
    Desloc[3].DeslocL=0;
    Desloc[4].DeslocH=0;
    Desloc[4].DeslocL=100;
    Desloc[5].DeslocH=224;
    Desloc[5].DeslocL=0;
    Desloc[6].DeslocH=0;
    Desloc[6].DeslocL=100;
    Desloc[7].DeslocH=224;
    Desloc[7].DeslocL=0;
    Desloc[8].DeslocH=0;
    Desloc[8].DeslocL=100;

// ------------------------FIM DA CONFIGURAÇÃO DE TRAGETÓRIA-------------------------------
    Serial.println("inicializando...");
    // initialize the RF Chip
    cc1101.init();

    cc1101.setSyncWord(&syncWord, false);
    cc1101.setCarrierFreq(CFREQ_433);
    cc1101.disableAddressCheck();
    //cc1101.setTxPowerAmp(PA_LowPower);

    delay(1000);
    /*
    Serial.print("CC1101_PARTNUM "); //cc1101=0
    Serial.println(cc1101.readReg(CC1101_PARTNUM, CC1101_STATUS_REGISTER));
    Serial.print("CC1101_VERSION "); //cc1101=4
    Serial.println(cc1101.readReg(CC1101_VERSION, CC1101_STATUS_REGISTER));
    Serial.print("CC1101_MARCSTATE ");
    Serial.println(cc1101.readReg(CC1101_MARCSTATE, CC1101_STATUS_REGISTER) & 0x1f);
    */
    attachInterrupt(0, cc1101signalsInterrupt, FALLING);
    Serial.println("Inicializado! ");
    //Serial.println("done");
}

void send_data(int escravo,int funcao, int endereco_inicial_MSB, int endereco_inicial_LSB,int num_registradores_pwm, int PWM_, int numero_pontos_H, int numero_pontos_L){
   
    switch(funcao){
        case 16:{
              if(endereco_inicial_MSB==0){//escrita da trajetória->ele executa a trajetória descrita no início do código.
                  CCPACKET data;

                  data.length=22;
    
                  data.data[0]=escravo;
                  data.data[1]=funcao;//16 = escrita registradores   1 = executa trajetória   3 = leitura de registradores 
                  data.data[2]=endereco_inicial_MSB;
                  data.data[3]=endereco_inicial_LSB;
                  data.data[4]=tamanho;//número debytes de dados a serem enviados. O vetor de dados é de tamanho máximo dados[32].
                  data.data[5]=Desloc[0].DeslocH;
                  data.data[6]=Desloc[0].DeslocL;
                  data.data[7]=Desloc[1].DeslocH;
                  data.data[8]=Desloc[1].DeslocL;
                  data.data[9]=Desloc[2].DeslocH;
                  data.data[10]=Desloc[2].DeslocL;
                  data.data[11]=Desloc[3].DeslocH;
                  data.data[12]=Desloc[3].DeslocL;
                  data.data[13]=Desloc[4].DeslocH;
                  data.data[14]=Desloc[4].DeslocL;
                  data.data[15]=Desloc[5].DeslocH;
                  data.data[16]=Desloc[5].DeslocL;
                  data.data[17]=Desloc[6].DeslocH;
                  data.data[18]=Desloc[6].DeslocL;
                  data.data[19]=Desloc[7].DeslocH;
                  data.data[20]=Desloc[7].DeslocL;
                  data.data[21]=CRC_H;//CRC_H
                  data.data[21]=CRC_L;//CRC_L
                  
                  //cc1101.flushTxFifo ();

                  for(i=0;i<data.length;i++){//Copia os dados enviados pelo data.data[i] para o vetor dados[]. Isto é necessário, pois no momento em que algum dado
                  //é atribuído à data.data[i], imediatamente o dado é enviado pelo módulo RF.
                      dados[i] = data.data[i];
                  }

                  if(cc1101.sendData(data)){
                      //Serial.println(" Enviado! ");
                                            
                      Serial.print("TX: PERGUNTA DE ESCRITA DE TRAJETORIA: \n");
                      for(i=0;i<data.length;i++){//Imprime no monitor serial.
                            Serial.print(dados[i],HEX);
                            Serial.print(" ");                            
                      }
                      Serial.print(" \n");
                }
              }
           
              
              if(endereco_inicial_MSB==1){//escrita do ganho pwm
                  CCPACKET data;

                  data.length=8;
    
                  data.data[0]=escravo;
                  data.data[1]=funcao;//16 = escrita registradores   1 = executa trajetória   3 = leitura de registradores 
                  data.data[2]=endereco_inicial_MSB;
                  data.data[3]=endereco_inicial_LSB;
                  data.data[4]=num_registradores_pwm;//número de valores de PWM's a serem escritos no escravo.
                  data.data[5]=PWM_;//Ganho pwm
                  data.data[6]=CRC_H;//CRC_H
                  data.data[7]=CRC_L;//CRC_L
                  
                  for(i=0;i<data.length;i++){
                      dados[i] = data.data[i];
                  }
                  
                  Serial.print("TX: PERGUNTA DE ESCRITA: \n");
                  for(i=0;i<data.length;i++){//Imprime no monitor serial.
                      Serial.print(dados[i],HEX);
                      Serial.print(" ");                            
                  }
                  Serial.print("\n");   
              }
            break;
        }
                
                
        case 3:{//PERGUNTA PARA LEITURA 
            CCPACKET data;

            data.length=8;
    
            data.data[0]=escravo;
            data.data[1]=funcao;//16 = escrita registradores   1 = executa trajetória   3 = leitura de registradores 
            data.data[2]=endereco_inicial_MSB;
            data.data[3]=endereco_inicial_LSB;
            data.data[4]=numero_pontos_H;//
            data.data[5]=numero_pontos_L;//
            data.data[6]=CRC_H;//CRC_H
            data.data[7]=CRC_L;//CRC_L     
 
            for(i=0;i<data.length;i++){
                  dados[i] = data.data[i];
            }
            
            Serial.print("TX: PERGUNTA DE LEITURA: \n");
            for(i=0;i<data.length;i++){//Imprime no monitor serial.
                  Serial.print(dados[i],HEX);
                  Serial.print(" ");                            
            }
            Serial.print(" \n");    
            break;
        }
    }
}
  
  
void ReadLQI()
{
    byte lqi=0;
    byte value=0;
    lqi=(cc1101.readReg(CC1101_LQI, CC1101_STATUS_REGISTER));
    value = 0x3F - (lqi & 0x3F);
}

void ReadRSSI()
{
    byte rssi=0;
    byte value=0;

    rssi=(cc1101.readReg(CC1101_RSSI, CC1101_STATUS_REGISTER));

    if (rssi >= 128)
    {
        value = 255 - rssi;
        value /= 2;
        value += 74;
    }
    else
    {
        value = rssi/2;
        value += 74;
    }

}

//-----------------------MAIN-----------------------------------------

void loop(){
  
    //void send_data(int escravo,int funcao, int endereco_inicial_MSB, int endereco_inicial_LSB,int num_registradores_pwm, int PWM, int numero_pontos_H, int numero_pontos_L)
    send_data(1,3,0,0,0,0,0,0);//num_registradores_pwm=>numero de registradores de pwm a serem setados ou lidos. Não deve ser usado para setar bytes da trajetória. Para tal,
    //deve-se alterar no início do programa na definição da trajetória, na variável "tamanho".
    
    //funcao-> código da função desejada: 3=escrita; 16=leitura;

    //Caso a função de escrita não seja para a configuração dos valores do PWM dos motores serão ignorados. Porém deve ser 
    //repassado algum valor no camp respectivo a estes parametros quando chamar a rotina send_data().

    //Caso deseja-se fazer a leitura do ganho PWM ou da trajetória, deve-se passar o número de pontos (HIGH) e (LOW).
    //Caso contrário, estes dados devem ser repassador na chamada da função, porém não serão usados dentro da rotina.
    
    if(packetAvailable){
        //Serial.println("Pacote recebido! ");
        // Disable wireless reception interrupt
        detachInterrupt(0);

        //ReadRSSI();
        //ReadLQI();
        // clear the flag
        packetAvailable = false;

        CCPACKET packet;

        if(cc1101.receiveData(&packet) > 0){
            if(!packet.crc_ok) {
                Serial.println("ERRO DE CRC!");
            }

            if(packet.length > 0){
                for(i=0;i<packet.length;i++){
                    resposta[i]=packet.data[i];
                }
                if(packet.data[0]==1){
                    switch(resposta[1])
                    {
                        case 16:
                        {
                            Serial.print("RX: ");
                            Serial.print(resposta[0]);
                            Serial.print(" ");
                            Serial.print(resposta[1]);
                            Serial.print(" ");
                            Serial.print(resposta[28]);
                            Serial.print(" ");
                            Serial.print(resposta[29]);
                            Serial.print(" ");
                            Serial.print(resposta[24]);
                            Serial.print(" ");
                            Serial.print(resposta[25]);
                            Serial.print(" ");
                            Serial.print(resposta[26]);
                            Serial.print(" ");
                            Serial.print(resposta[27]);
                            Serial.print(" \n");
                            break;
                        }
                        case 3:
                        {  
                            Serial.print("RX: ");
                            Serial.print(resposta[0],HEX);
                            Serial.print(" ");
                            Serial.print(resposta[1],HEX);
                            Serial.print(" ");
                           
                            if(resposta[28]==1)
                            {
                                  if(resposta[29]==0)
                                  {
                                      if(resposta[31]==2)
                                        {
                                            Serial.print(4,HEX);
                                            Serial.print(" ");
                                            Serial.print(resposta[3],HEX);
                                            Serial.print(" ");
                                            Serial.print(resposta[4],HEX);
                                            Serial.print(" ");
                                            Serial.print(resposta[5],HEX);
                                            Serial.print(" ");
                                            Serial.print(resposta[6],HEX);
                                            Serial.print(" "); 
                                      }
                              
                                      else
                                      {
                                            Serial.print(2,HEX);
                                            Serial.print(" ");
                                            Serial.print(resposta[3],HEX);
                                            Serial.print(" ");
                                            Serial.print(resposta[4],HEX);
                                     }
                                  }
                                  else
                                  {
                                      if(resposta[31]==1)
                                        {
                                             Serial.print(2,HEX);
                                             Serial.print(" ");
                                             Serial.print(resposta[3],HEX);
                                             Serial.print(" ");
                                             Serial.print(resposta[4],HEX);
                                        }
                                      else
                                      {
                                            Serial.print("ERRO DE CRC!");
                                            Serial.print(" ");
                                      }
                                  }
                            }
                            else if(resposta[28]==0)
                            {
                                  
                                     Serial.print(resposta[31],HEX);
                                     Serial.print(" ");
                                     if(resposta[31]<9-resposta[29])
                                     {
                                           for(i=0;i<2*resposta[31];i++)
                                           {
                                               Serial.print(resposta[i+7+2*resposta[29]],HEX);
                                               Serial.print(" ");
                                           }
                                     }
                                     else
                                     {
                                           Serial.print("Registradores insuficientes");
                                     }
                                     break;
                                 
                             }
                            else
                            {
                                Serial.print("  ");
                            }
                            break;
                        }
                        case 1:
                        {
                            Serial.print("RX: ");
                            Serial.print(resposta[0]);
                            Serial.print(" ");
                            Serial.print(resposta[1]);
                            Serial.print(" ");
                        }
                        default:
                        {
                            
                        }
                        
                    } 
            }
            
            
            
            
            
            else
            {
                Serial.println("ERRO DE CRC! ");
            }
        }
        else
        {
                Serial.println("ERRO DE CRC! ");
            }
        }
        }
    

        // Habilita interrupção do receptor RF.
        attachInterrupt(0, cc1101signalsInterrupt, FALLING);
    
    
    delay(2000);
}
