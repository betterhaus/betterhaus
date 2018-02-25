#include <RCSwitch.h>

#define ROT   11//5
#define GRUEN 3//11
#define BLAU  5//3
#define SYSTEMCODE 0x1B

int wert_rot, wert_blau, wert_gruen;
void setLight(int farbe, int wert);

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  Serial.println("Start Setup");

  wert_rot = wert_blau = wert_gruen = 100;
  analogWrite(ROT, wert_rot);
  analogWrite(GRUEN, wert_gruen);
  analogWrite(BLAU, wert_blau);

  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  setLight(ROT, 0);
}

//Empfange Nachricht: [0-4] Systemcode (11011), [0-1] Farbe (Rot = 01, Gruen = 10, Blau = 11), [0-7] Wert
//also: [0-4] [5-6] [7-14]

void loop() {
  if (mySwitch.available()) {

    Serial.println("Message received");
    unsigned long msg = mySwitch.getReceivedValue();
    int code  = (msg >> 10);
    int farbe = (msg >> 8) & 0x3;
    int wert  = msg & 0xFF;

    if(code == SYSTEMCODE){
      /*Serial.print("code: ");
      Serial.print(code);
      Serial.print("   farbe: ");
      Serial.print(farbe);
      Serial.print("  wert: ");
      Serial.println(wert);
    */switch (farbe){
        case 0x1: wert_rot = wert; break;
        case 0x2: wert_gruen = wert; break;
        case 0x3: wert_blau = wert; break;
        case 0x0: wert_rot = wert_blau = wert_gruen = wert;
      }
      setLight(farbe, wert);
      mySwitch.resetAvailable();
    }
  }
}

 void setLight(int farbe, int wert)
 {  
  /*  Serial.print("Setze Farbe: ");
    Serial.println(farbe);
    Serial.print("Werte: Rot");
    Serial.print(wert_rot);
    Serial.print("    Gruen: ");
    Serial.print(wert_gruen);
    Serial.print("    Blau:  ");
    Serial.println(wert_blau);
  */analogWrite(ROT, wert_rot); 
    analogWrite(GRUEN, wert_gruen);
    analogWrite(BLAU, wert_blau);
   
 }
