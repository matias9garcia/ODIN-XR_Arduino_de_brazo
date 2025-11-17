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
 PosicionInicial();
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
bool IK_Braccio(float x, float y, float z, int &m1, int &m2, int &m3, int &m4) {
  // 1) Ángulo base
  m1 = rad2deg(atan2(y, x));
  if(m1 < 0) m1 += 360;
  if(m1 > 180) return false;

  // 2) Centro muñeca
  float R = sqrt(x*x + y*y);
  float Z = z - d1;

  // Distancia hombro–muñeca
  float D = sqrt(R*R + Z*Z);

  // 3) Ley del coseno: codo
  float cos_angle3 = (D*D - L2*L2 - L3*L3) / (2 * L2 * L3);

  if(cos_angle3 > 1 || cos_angle3 < -1) return false;

  float angle3 = acos(cos_angle3);  // radianes
  m3 = 180 - rad2deg(angle3);       // Braccio invertido

  // 4) Hombro
  float angle2 = atan2(Z, R) - atan2(L3 * sin(angle3),
                                     L2 + L3 * cos(angle3));
  m2 = rad2deg(angle2) + 90;  // compensación mecánica Braccio

  // 5) Muñeca vertical
  m4 = 180 - (m2 + m3);

  // Validación rangos mecánicos
  if(m2 < 15 || m2 > 165) return false;
  if(m3 < 0  || m3 > 180) return false;
  if(m4 < 0  || m4 > 180) return false;

  return true;
}

void MoverConIK(float x, float y, float z){
  int ik1, ik2, ik3, ik4;

  // Calcular valores dinámicos
  float R_min, R_max, Z_min, Z_max;
  CalcularRangos(R_min, R_max, Z_min, Z_max);

  float R = sqrt(x*x + y*y);

  // Mostrar rangos
  ImprimirRangos();

  // Mostrar el punto solicitado
  Serial.print("Intentando mover a → X="); Serial.print(x);
  Serial.print("  Y="); Serial.print(y);
  Serial.print("  Z="); Serial.println(z);

  // ---------- VALIDACIÓN ANTES DE LA IK ----------
  bool fuera = false;

  if (R < R_min) {
    Serial.print("❌ ERROR: Radio demasiado pequeño (R=");
    Serial.print(R); Serial.println(" cm)");
    fuera = true;
  }
  if (R > R_max) {
    Serial.print("❌ ERROR: Radio demasiado grande (R=");
    Serial.print(R); Serial.println(" cm)");
    fuera = true;
  }
  if (z < Z_min) {
    Serial.print("❌ ERROR: Z demasiado bajo (Z=");
    Serial.print(z); Serial.println(" cm)");
    fuera = true;
  }
  if (z > Z_max) {
    Serial.print("❌ ERROR: Z demasiado alto (Z=");
    Serial.print(z); Serial.println(" cm)");
    fuera = true;
  }

  if (fuera) {
    Serial.println("👉 Coordenadas fuera de los límites permitidos.");
    return;
  }

  // ---------- Intento IK ----------
  if(IK_Braccio(x, y, z, ik1, ik2, ik3, ik4)){
    Serial.print("IK OK → ");
    Serial.print(ik1); Serial.print(", ");
    Serial.print(ik2); Serial.print(", ");
    Serial.print(ik3); Serial.print(", ");
    Serial.println(ik4);

    Braccio.ServoMovement(20, ik1, ik2, ik3, ik4, 90, m6);
  } else {
    Serial.println("IK falló: punto NO se puede alcanzar por ángulos.");
  }
}

// =============================
// RANGO DINÁMICO DEL BRACCIO
// =============================
void CalcularRangos(float &R_min, float &R_max, float &Z_min, float &Z_max) {
  R_min = abs(L2 - L3);      // mínimo radial (brazo doblado sobre sí mismo)
  R_max = L2 + L3;           // máximo radial (brazo extendido)
  Z_min = 0;                 // asumiendo mesa en z=0
  Z_max = d1 + (L2 + L3);    // máximo vertical
}

void ImprimirRangos() {
  float R_min, R_max, Z_min, Z_max;
  CalcularRangos(R_min, R_max, Z_min, Z_max);

  Serial.println("=== RANGO PERMITIDO DEL BRACCIO ===");
  Serial.print("Radio mínimo (R_min): "); Serial.print(R_min); Serial.println(" cm");
  Serial.print("Radio máximo (R_max): "); Serial.print(R_max); Serial.println(" cm");
  Serial.print("Altura mínima (Z_min): "); Serial.print(Z_min); Serial.println(" cm");
  Serial.print("Altura máxima (Z_max): "); Serial.print(Z_max); Serial.println(" cm");
  Serial.println("===================================");
}


void loop() {
  //Serial.println(BTSerial.available());
  if (Serial.available()){
      String cmd = Serial.readStringUntil('\n');

      if(cmd.startsWith("IK")){
        float x = cmd.substring(3, cmd.indexOf(',')).toFloat();
        float y = cmd.substring(cmd.indexOf(',')+1, cmd.lastIndexOf(',')).toFloat();
        float z = cmd.substring(cmd.lastIndexOf(',')+1).toFloat();
        MoverConIK(x,y,z);
      }
  }

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
