#include <SoftwareSerial.h>
#include <Braccio.h>
#include <Servo.h>

Servo base;
Servo shoulder;
Servo elbow;
Servo wrist_rot;
Servo wrist_ver;
Servo gripper;

SoftwareSerial BTSerial(2, 3); // RX | TX

char opcion;
int m1=0,m2=0,m3=0,m4=0,m5=0,m6=0;
int signo=0;

void setup() {
 Braccio.begin();
 Serial.begin(9600);
 PosicionSegura();
 BTSerial.begin(9600); 
  
}

 /*
   Step Delay: a milliseconds delay between the movement of each servo.  Allowed values from 10 to 30 msec.
   M1=base degrees. Allowed values from 0 to 180 degrees
   M2=shoulder degrees. Allowed values from 15 to 165 degrees
   M3=elbow degrees. Allowed values from 0 to 180 degrees
   M4=wrist vertical degrees. Allowed values from 0 to 180 degrees
   M5=wrist rotation degrees. Allowed values from 0 to 180 degrees
   M6=gripper degrees. Allowed values from 10 to 73 degrees. 10: the toungue is open, 73: the gripper is closed.
  */
void PosicionInicial(){
   //(step delay, M1, M2, M3, M4, M5, M6);
  Braccio.ServoMovement(20,0,15, 180, 0, 0,  10);  
  delay(500);
}

void PosicionSegura(){
   //(step delay, M1, M2, M3, M4, M5, M6);
  m1=90;
  m2=45;
  m3=180;
  m4=180;
  m5=90;
  m6=10;
  Braccio.ServoMovement(30,m1,m2, m3, m4, m5, m6);  
}

void CambioDePosicion(){
  if (Serial.available() > 0) {
    char input = Serial.read();  // lee un solo carácter
    if (input == '1') {
      PosicionInicial();
    }
    else if (input == '2') {
      PosicionSegura();
    }
    else {
      Serial.println("Comando inválido. Ingresa 1 o 2.");
    }
  }
}


void loop() {
  //Serial.println(BTSerial.available());
  
  CambioDePosicion();

  // if (BTSerial.available()){   // read from HC-05 and send to Arduino Serial Monitor
  //     opcion=BTSerial.read();
  //     Serial.println(opcion);
  // switch(opcion){
  //   case '0': 
  //      PosicionSegura();
  //   break;
    
  //   case '1': //M1
  //     if(signo==1)m1=m1+10;else m1=m1-10;
  //     Braccio.ServoMovement(30,m1,m2, m3, m4, m5,  m6);  
  //     delay(500);
  //   break;

  //   case '2': //M2
  //     if(signo==1)m2=m2+10; else m2=m2-10;
  //     Braccio.ServoMovement(30,m1,m2, m3, m4, m5,  m6);  
  //     delay(500);
  //   break;

  //   case '3': // M3
  //     if(signo==1)m3=m3+10;else m3=m3-10;
  //     Braccio.ServoMovement(30,m1,m2, m3, m4, m5,  m6);  
  //     delay(500);
  //    break;
  //   case '4': //M4
  //      if(signo==1)m4=m4+10;else m4=m4-10;
  //      Braccio.ServoMovement(30,m1,m2, m3, m4, m5,  m6);  
  //      delay(500);
  //    break;

  //    case '5': //M5
  //      if(signo==1)m5=m5+10;else m5=m5-10;
  //      Braccio.ServoMovement(30,m1,m2, m3, m4, m5,  m6);  
  //      delay(500);
  //    break;

  //    case '6': //M6
  //      if(signo==1)m6=m6+10;else m6=m6-10;
  //      Braccio.ServoMovement(30,m1,m2, m3, m4, m5,  m6);  
  //      delay(500);
  //    break;

  //    case '7': //signo ++
  //      signo=1;
  //    break;

  //    case '8': //signo --
  //       signo=0;
  //    break;

  //  }
     
  // }
 
}
