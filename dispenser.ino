
// ############ Libraries necessarias #################################################################################################################
#include "RTClib.h" //https://github.com/adafruit/RTClib
#include "sha1.h" //https://github.com/lucadentella/TOTP-Arduino
#include "TOTP.h" //https://github.com/lucadentella/TOTP-Arduino
#include "Keypad.h" //http://www.arduino.cc/playground/uploads/Code/Keypad.zip
#include "U8g2lib.h" //https://www.arduinolibraries.info/libraries/u8g2
#include "Password.h" //https://playground.arduino.cc/Code/Password/
// ####################################################################################################################################################

RTC_DS3231 rtc; //Define Relogio para DS3231 (https://create.arduino.cc/projecthub/MisterBotBreak/how-to-use-a-real-time-clock-module-ds3231-bc90fe )

uint8_t hmacKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //gerar senha HEX em: http://www.lucadentella.it/OTP/
TOTP totp = TOTP(hmacKey, 10);
//##############################################################Definição de Variaveis ################################################################
int tmp; //Define variavel temporaria
int T[2][8]; //Define matriz de variaveis para configurar rtc
int senha = 0; //Define variaveis de flag para saber se senha ja foi digitada
int counter = 1; //variavel de suporte para rotinas que utilizem contador 
int Fuso=-3; // variavel para definição de fuso horario No setup: 03=3 horas a mais no horario UTM , 93 =3 a menos no horario UTM
int flagSec; //variavel para sinalizar se os segundos devem ser exibidos 
int local = 20; //variavel para setar posição onde numeros digitados devem aparecer
int Setup = 1; // variavel de inicialização 1 = ao ligar entra e configuração do relogio , 0 = não pede configuração
int Conf = 1; // variavel para definer etapas da configuração
char code[7]; //Devine numero de caracteres do password
Password password = Password( "123456" ); //Inicializa e define password inicial

//############################################################## Configurar teclado/keypad ############################################################
const byte ROWS = 4; //Define que teclado tera 4 linhas
const byte COLS = 3; //Define que teclado tera 3 colunas
// Define disposição das teclas ( http://blogmasterwalkershop.com.br/arduino/como-usar-com-arduino-teclado-matricial-de-membrana-4x3-com-12-teclas/)
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = { 4, 5, 6, 7 };//Define conexões das linhas do keypad aos pinos do arduino
byte colPins[COLS] = { 8, 9, 10 };//Define conexões das colunas do keypad aos pinos do arduino
//Inicializa teclado/keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//#####################################################################################################################################################

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); //Inicializa e define display

//## Função dispensar produto ##
void entregar(){
      while(digitalRead(A0) != LOW){
        digitalWrite(A3,HIGH);
        digitalWrite(A2,LOW);
      }
       while(digitalRead(A1) != LOW){
        digitalWrite(A2,HIGH);
        digitalWrite(A3,LOW);
      }
      //recua um pouco para poupar sensor fim de curso
      //while(digitalRead(A1) == LOW){
       // digitalWrite(A3,HIGH);
       // digitalWrite(A2,LOW);
      //}
        digitalWrite(A2,LOW);
        digitalWrite(A3,LOW);
}



//## Função para ler teclas digitadas
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()){
    case PRESSED:
    if (local < 24) {
      u8g2.clearBuffer();
      flagSec =1;
    }
    if ((Setup != 0) and ( local < 70 )){
      local =70;     
      u8g2.clearBuffer();
      flagSec =1;
    }
    u8g2.setFont(u8g2_font_osb18_tn);
    u8g2.setCursor(local, 32);
    u8g2.print(eKey); 
    u8g2.sendBuffer();
    local = local+14; 
 	  switch (eKey){
    	    case '#': {
    	      if ( Setup == 1){
              u8g2.clearBuffer();
              u8g2.setFont( u8g2_font_ncenB12_tf);
              u8g2.setCursor(10, 14);
              u8g2.print("OK ,");
              u8g2.setCursor(10, 30);
              u8g2.print("AGUARDE !");
              u8g2.sendBuffer();
              Setup = 0;
              break;
    	      }
     	    }
          
          default: {
    	      if ( Setup == 0 ) {
    	       password.append(eKey);
             if ( counter == 6 ){
                //flag=1;
                local=20;
                counter=0;
                checkPassword();break;
              }
    	      }else{
              Setup=2;
              if ( counter == 2 ) {
                    T[counter][Conf] = ((eKey)-48)+T[counter-1][Conf];
                    counter = 0;
                    Conf++;
                    delay(1000);
                    u8g2.clearBuffer();
                    local = 70;
                    break;
              }else{
                    T[counter][Conf] = ((eKey)-48)*10;
              }

    	      }
    	    }
    }
    counter++;
  }
}

//## Função para checar senhas
void checkPassword(){
  local = 22;
  if (password.evaluate()){
     if ( senha == 0 ){ 
      u8g2.clearBuffer();
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(10, 14);
      u8g2.print("RETIRE"); 
      u8g2.setCursor(10, 30);
      u8g2.print("SEU BRINDE"); 
      u8g2.sendBuffer();
      password.reset();
      entregar();
      senha = 1;
     }else{
      u8g2.clearBuffer();
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(10, 14);
      u8g2.print("CODIGO JA"); 
      u8g2.setCursor(10, 30);
      u8g2.print("DIGITADO!"); 
      u8g2.sendBuffer();
     }
  }else{
   if ( senha == 0 ){ 
    u8g2.clearBuffer();
    u8g2.setFont( u8g2_font_ncenB12_tf);
    u8g2.setCursor(10, 14);
    u8g2.print("TENTE");
    u8g2.setCursor(10, 30);
    u8g2.print("NOVAMENTE"); 
    u8g2.sendBuffer();
    password.reset();
   }else{
    u8g2.clearBuffer();
    u8g2.setFont( u8g2_font_ncenB12_tf);
    u8g2.setCursor(20, 30);
    u8g2.print("AGUARDE !");
    u8g2.sendBuffer();
    
   }
  }
}
//## Configurações iniciais
void setup(){
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A1, INPUT_PULLUP); //sem necessidade de colocar resistor push pull
  pinMode(A0, INPUT_PULLUP); //sem necessidade de colocar resistor push pull
  rtc.begin();
  Serial.begin(9600);
  keypad.addEventListener(keypadEvent); 
  u8g2.begin();
}

//## loop de execução
void loop(){
  DateTime now = rtc.now();
  DateTime utm (now + TimeSpan(0,Fuso,0,0));
  long GMT = utm.unixtime();
  char* newCode = totp.getCode(GMT);
  if (Setup < 2){
    if(strcmp(code, newCode) != 0) {
      strcpy(code, newCode);
      password.reset();
      password.set(code);
      local = 20;
      flagSec = 0;
      senha = 0;
      u8g2.clearBuffer();   
      u8g2.setFont(u8g2_font_pressstart2p_8r);
      tmp=(now.hour());
      if ( tmp < 10 ){
        u8g2.setCursor(5, 8);   
        u8g2.print("0");
        u8g2.setCursor(13, 8);
        u8g2.print(now.hour());
      }else{
        u8g2.setCursor(5, 8);
        u8g2.print(now.hour());
      }       
      u8g2.setCursor(20, 8);
      u8g2.print(":"); 
      tmp=(now.minute());
      if ( tmp < 10 ){
        u8g2.setCursor(26, 8);   
        u8g2.print("0");
        u8g2.setCursor(34, 8);   
        u8g2.print(now.minute());
      }else{
        u8g2.setCursor(26, 8);   
        u8g2.print(now.minute());
      } 
      u8g2.setCursor(41, 8);
      u8g2.print(":"); 
      tmp=(now.day());
      if (tmp < 10 ){
        u8g2.setCursor(94, 8);
      }else{
        u8g2.setCursor(86, 8);
      }
      u8g2.print(now.day()); 
      u8g2.setCursor(104, 8);
      u8g2.print("/"); 
      u8g2.setCursor(112, 8);
      u8g2.print(now.month());
      Serial.println(code);
    } 
  } 

  if ((Setup == 0) and (local == 20)) { 
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(10, 30);
      u8g2.print("DIGITE :"); 
      u8g2.sendBuffer();         
  }else if (Setup == 1){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(10, 30);
      u8g2.print("CONFIG ?"); 
      u8g2.sendBuffer();
  }
 
  if ((flagSec == 0 ) and (Setup < 2)){
      u8g2.setFont(u8g2_font_pressstart2p_8r);
      tmp=(now.second());
      if ( tmp < 10 ){
         u8g2.setCursor(46, 8);
         u8g2.print("0");
         u8g2.setCursor(54, 8);
         u8g2.print(now.second());         
      }else{
         u8g2.setCursor(46, 8);
         u8g2.print(now.second());
      }
      u8g2.sendBuffer();
  }
  
  if (Setup == 2){
   if ( Conf == 1){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(22, 30);
      u8g2.print("Fuso:"); 
      u8g2.sendBuffer();
      
    }else if ( Conf == 2){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(30, 30);
      u8g2.print("Dia:"); 
      u8g2.sendBuffer();
    
    }else if ( Conf == 3){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(30, 30);
      u8g2.print("Mes:"); 
      u8g2.sendBuffer();
    
    }else if ( Conf == 4){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(30, 30);
      u8g2.print("Ano:"); 
      u8g2.sendBuffer();
    
    }else if ( Conf == 5){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(30, 30);
      u8g2.print("Hrs:"); 
      u8g2.sendBuffer();
    
    }else if ( Conf == 6){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(30, 30);
      u8g2.print("Min:"); 
      u8g2.sendBuffer(); 
   
    }else if ( Conf == 7){
      u8g2.setFont( u8g2_font_ncenB12_tf);
      u8g2.setCursor(10, 26);
      u8g2.print("Configurado "); 
      u8g2.sendBuffer();
      Setup = 0;  
      delay(1000);
     if ( T[2][1] >= 90 ){
        Fuso= (T[2][1]-90);
      }else{
        Fuso= -(T[2][1]);
      }
      //##Configurar rtc
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
     rtc.adjust(DateTime((T[2][4]+2000), (T[2][3]), (T[2][2]), (T[2][5]), (T[2][6]), 0));
     Serial.println("Configurado:");
     Serial.print("Hora:");
     Serial.print((T[2][5]));
     Serial.print(" Minutos:");
     Serial.print((T[2][6]));
     Serial.print(" Dia:");
     Serial.print((T[2][2]));
     Serial.print(" Mes:");
     Serial.print((T[2][3]));
     Serial.print(" Ano:");
     Serial.println((T[2][4]+2000));
     local = 20;
  }
 }
    keypad.getKey();
}
