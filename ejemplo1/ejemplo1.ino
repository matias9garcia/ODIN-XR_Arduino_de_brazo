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
 PosicionInicial();
 Serial.begin(9600);
 delay(500);  // evita perder el primer dígito
 BTSerial.begin(9600);
 Serial.println("READY");
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
  Braccio.ServoMovement(80,0,15, 180, 0, 0,  10);  
  delay(500);
}

void PosicionSecundaria(){
   //(step delay, M1, M2, M3, M4, M5, M6);
  Braccio.ServoMovement(80,90,15, 180, 0, 0,  10);  
  delay(500);
}

void PosicionTerciaria(){
   //(step delay, M1, M2, M3, M4, M5, M6);
  Braccio.ServoMovement(80,90,110, 160, 110, 0,70);  
  delay(500);
}

void PosicionSegura(){
   //(step delay, M1, M2, M3, M4, M5, M6);
  m1=90;
  m2=45;
  m3=180;
  m4=180;
  m5=70;
  m6=10;
  Braccio.ServoMovement(30,m1,m2, m3, m4, m5, m6);  
}

void CambioDePosicion(int m1, int m2, int m3, int m4, int m5, int m6) {
    Braccio.ServoMovement(10,m1,m2, m3, m4, m5, m6);
    // delay(5000);
    // if (m1 >= 0 && m1 < 60) {
    //   PosicionInicial();
    // }
    // else if (m1 >= 60 && m1 < 120) {
    //   PosicionTerciaria();
    // }
    // else if (m1 >= 120 && m1 < 180) {
    //   PosicionSecundaria();
    // }
    // else {
    //   Serial.println("Comando inválido.");
    // }
}

void TesteoAbrirEfector(){
  Braccio.ServoMovement(1,0,15, 180, 0, 0,  1);
  delay(2500);
  Braccio.ServoMovement(1,0,15, 180, 0, 0,  70);
  delay(2500);
}


// =============================
// CINEMÁTICA INVERSA Braccio
// =============================

// Medidas aproximadas del Braccio en cm
const float d1 = 10.5;  
const float L2 = 12;
const float L3 = 14.5;
const float L4 = 22;  // muñeca → pinza

// Convierte radianes → grados
float rad2deg(float r){
  return r * 180.0 / PI;
}

// =============================
//  IK: ingresa (x,y,z) deseado
// genera m1..m4
// =============================


String inputString = "";
bool stringComplete = false;

void loop() {

  // TesteoAbrirEfector();

  // Lectura de caracteres del puerto serial
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      stringComplete = true;
    } else {
      inputString += c;
    }
  }

  // Si llegó una línea completa
  if (stringComplete) {

    int m1, m2, m3, m4, m5, m6;

    // Parseo limpio sin breaks ni loops complejos
    int parsed = sscanf(
      inputString.c_str(),
      "%d,%d,%d,%d,%d,%d",
      &m1, &m2, &m3, &m4, &m5, &m6
    );

    if (parsed == 6) {
      // Llamar a la función con las 6 variables por nombre
      CambioDePosicion(m1, m2, m3, m4, m5, m6);
      Serial.println(
        String("Arduino recibió: ") +
        m1 + "," +
        m2 + "," +
        m3 + "," +
        m4 + "," +
        m5 + "," +
        m6
      );
    } else {
      Serial.println("ERROR_FORMATO");
    }

    // Reset del buffer
    inputString = "";
    stringComplete = false;
  }
}
