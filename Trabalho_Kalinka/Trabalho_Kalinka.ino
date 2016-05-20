/**
 *  PROGRAMA ESCRITO POR
 *  Juliano D. Negri (Gonzales)
 *  João Pedro B. Ishida (Narcus)
 *  Carlos Eduardo A. Martins (Colgate)
 *  Bruno de Oliveira Lima (Pig)
 *  
 *  Este programa controlará um carrinho utilizando um arduino UNO , um LCD, dois Servos Motores, um USB HOST, um sensor Infravermelho e um controle de PS3
 *  
 * O controle do PS3 servirá para : 
 * R1 : Aumenta a Marcha
 * L1 : Diminuie a Marcha
 * LeftHat  x : define o  W  
 * RightHat Y : define a velocidade
 * Quadrado : zera a velocidade
 * Start : Abre um menu de Músicas
 * Setas : controlam o menu ?
 * X : Caso bata :" aperte X para continuar "
 * 
 */
#include <LiquidCrystal.h>  //Biblioteca do LCD
#include <PS3BT.h>          //Biblioteca do bluetooth LCD 
#include <usbhub.h>         // PADRAO USB-HOST2.0

#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

//********** PINS ************
#define LMOTOR1 10    // Driver 1 do LMOTOR
#define LMOTOR2 11    // Driver 2 do LMOTOR
#define LMOTORE 12    // PWM do LMOTOR
#define RMOTOR1 7     // Driver 1 do RMOTOR
#define RMOTOR2 8     // Driver 2 do RMOTOR
#define RMOTORE 9     // PWM do RMOTOR
#define SENSOR A0     // Sensor

// ********* PARAMETROS FÍSICOS *************
#define REIXO 50    // Raio do eixo (mm)
#define RRODA 10    // Raio da roda (mm)

//********** PARÂMETROS DE CONTROLE ***********
#define VMAX 10     // Velocidade limite permitida
#define DMIN 20     // Distância de BATIDA
#define WTOTAL 100  // Velocidade Máxima alcançada (teoricamente) pelo motor

//USBHub Hub1(&Usb); // Some dongles have a hub inside
USB Usb;

// You have to create the Bluetooth Dongle instance like so
BTD Btd(&Usb); 
/* You can create the instance of the class in two ways */
PS3BT PS3(&Btd); // This will just create the instance
//PS3BT PS3(&Btd, 0x00, 0x15, 0x83, 0x3D, 0x0A, 0x57); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch

// Cria a classe LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Marcha é a marcha atual , v é a velocidade linear desejada, w a velocidade angular desejada
int marcha = 0, v= 0 , w = 0;
// Armazena o valor do sensor
int sensor;

void setup() {
  Serial.begin(115200);
  
  pinMode( RMOTOR1, OUTPUT);
  pinMode( RMOTOR2, OUTPUT);
  pinMode( LMOTOR1, OUTPUT);
  pinMode( LMOTOR2, OUTPUT);

  #if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif
    if (Usb.Init() == -1) {
      Serial.print(F("\r\nOSC did not start"));
      while (1); //halt
  }
  Serial.print(F("\r\nPS3 Bluetooth Library Started"));
}

void loop() {
  Usb.Task();
  // Lê o sensor
  sensor = analogRead( SENSOR );
  
  if (PS3.PS3Connected) {
    if (PS3.getAnalogHat(LeftHatX) > 137 || PS3.getAnalogHat(LeftHatX) < 117 || PS3.getAnalogHat(LeftHatY) > 137 || PS3.getAnalogHat(LeftHatY) < 117 || PS3.getAnalogHat(RightHatX) > 137 || PS3.getAnalogHat(RightHatX) < 117 || PS3.getAnalogHat(RightHatY) > 137 || PS3.getAnalogHat(RightHatY) < 117) {
      // ENTRADA DO W -- FALTA O MAP
      w = PS3.getAnalogHat(LeftHatX); 
      if (PS3.PS3Connected) {
        // ENTRADA DO V -- FALTA O MAP
        v = PS3.getAnalogHat(RightHatY);
      }
    }
        
      if (PS3.getButtonClick(SQUARE)){
        // Zera as velocidades
        v = 0 ;
        w = 0 ;
      }
          
      if (PS3.getButtonClick(L1)){
       marcha > 1 ? -- marcha : marcha+=0 ;
             
      }
      if (PS3.getButtonClick(R1))
       marcha < 3 ? ++ marcha : marcha += 0 ;
      
      if (PS3.getButtonClick(START)) {
        menu();
      }
    
    velocidade( v, w , marcha);

    if ( sensor < DMIN && sensor != 0) {
      batida();
    }
}

}

// ************ FUNÇÕES ******************

//Função que transforma V e W desejados em saidas para o motor
void velocidade( int V , int W , int marcha){
  int WL , WR ;
  
// Controle de WR e WL a partir de V e W desejados, além do controle da velocidade pela marcha
  WR =( ( V - (REIXO * W)/ 2  ) / RRODA ) * marcha ;
  WL =( ( V + (REIXO * W)/ 2  ) / RRODA ) * marcha ;

// Muda a polarização do motor para dar marcha-ré
  if ( WR >= 0 ){
    digitalWrite( RMOTOR1, HIGH);
    digitalWrite( RMOTOR2, LOW );
  } else{
    
    digitalWrite( RMOTOR1, LOW);
    digitalWrite( RMOTOR2, HIGH);
  }
  
  if ( WL >= 0 ){
    digitalWrite( LMOTOR1, HIGH);
    digitalWrite( LMOTOR2, LOW );
  } else{
    
    digitalWrite( LMOTOR1, LOW);
    digitalWrite( LMOTOR2, HIGH);
  }
 // Testa o limite máximo da velocidade
  WR = abs(WR) > VMAX ? VMAX : WR;
  WL = abs(WL) > VMAX ? VMAX : WL;
  analogWrite( RMOTORE, map( abs(WR), 0, WTOTAL, 0, 255));
  analogWrite( LMOTORE, map( abs(WL), 0, WTOTAL, 0, 255));
}

//Função que escreve o menu 
void menu(){
  velocidade( 0 , 0, 1);
  lcd.clear();
  lcd.setCursor(0 , 0);
  lcd.print("MUSICA A");  
  lcd.setCursor(0, 1);
  lcd.print("MUSICA B");
  int BPRESSED= 0;
  while( BPRESSED != 1 || BPRESSED != 2){
      Usb.Task();
    if (PS3.getButtonClick(CIRCLE)) 
        BPRESSED = 1;       
    if (PS3.getButtonClick(CROSS))
        BPRESSED = 2;   
  }

  // PRECISA DA FUNÇÂOO TOCAR MUSICA ...
}

void batida(){

  velocidade( 0, 0, 1);
  lcd.clear();
  lcd.setCursor(0 , 0);
  lcd.print("VOCE BATEU !");  
  lcd.setCursor(0, 1);
  lcd.print("PRESSIONE X PARA CONTINUAR");

  PS3.setRumbleOn(RumbleHigh);
  delay(1000);
  PS3.setRumbleOn(RumbleLow);
  int flag = 0;
  while( flag == 0 ){
      Usb.Task();
    if (PS3.getButtonClick(CROSS))
        flag = 1;    
  }
 
  vira180();

}

void vira180(){
  
}

