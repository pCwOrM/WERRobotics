#include <LiquidCrystal.h>
LiquidCrystal lcd(54, 55, 56, 57, 58, 59);

#define INA1  38
#define INA2  36
#define HIZA  44
#define INB1  42
#define INB2  40
#define HIZB  46

#define onFar  51
#define arkfar 48
#define solSin 53
#define sagSin 49

#define solSen A6
#define ortSen A7
#define sagSen A8

void setup() {
  pinMode(INA1, OUTPUT); pinMode(INA2, OUTPUT); pinMode(HIZA, OUTPUT);
  pinMode(INB1, OUTPUT); pinMode(INB2, OUTPUT); pinMode(HIZB, OUTPUT);
  lcd.begin(16, 2);
  //Serial.begin(9600);
  pinMode(solSen, OUTPUT); pinMode(ortSen, OUTPUT); pinMode(sagSen, OUTPUT);
}

void loop() {
  sagD = digitalRead(sagSen);
  solD = digitalRead(solSen);
  ortD = digitalRead(ortSen);

  if (sagD == 0 & solD == 1 & ortD == 1 ) {
    saga_don();
    digitalWrite(sagSin, 1);
    digitalWrite(solSin, 0);
    digitalWrite(onFar,  0);
    digitalWrite(arkFar, 0);
  }
  else if (sagD == 1 & solD == 0 & ortD == 1 ) {
    sola_don();

    digitalWrite(sagSin, 0);
    digitalWrite(solSin, 1);
    digitalWrite(onFar,  0);
    digitalWrite(arkFar, 0);

  }
  else if (sagD == 1 & solD == 0 & ortD == 0 ) {
    sola_don();

    digitalWrite(sagSin, 0);
    digitalWrite(solSin, 1);
    digitalWrite(onFar,  0);
    digitalWrite(arkFar, 0);

  }
  else if (sagD == 1 & solD == 1 & ortD == 0) {
    ileri();

    digitalWrite(sagSin, 0);
    digitalWrite(solSin, 0);
    digitalWrite(onFar,  1);
    digitalWrite(arkFar, 0);

  }
  else if (sagD == 0 & solD == 0 & ortD == 0 ) {
    dur();
    digitalWrite(sagSin, 0);
    digitalWrite(solSin, 0);
    digitalWrite(onFar,  0);
    digitalWrite(arkFar, 1);
    delay(10000);
    ileri();
  }
}
void geri() {
  digitalWrite(INA1, LOW); digitalWrite(INA2, HIGH); analogWrite(HIZA, 150);
  digitalWrite(INB1, LOW); digitalWrite(INB2, HIGH); analogWrite(HIZB, 150);
}

void ileri() {
  digitalWrite(INA1, HIGH); digitalWrite(INA2, LOW); analogWrite(HIZA, 150);
  digitalWrite(INB1, HIGH); digitalWrite(INB2, LOW); analogWrite(HIZB, 150);
  delay(2000);
}
void dur() {
  digitalWrite(INA1, 0); digitalWrite(INA2, 1); analogWrite(HIZA, 0);
  digitalWrite(INB1, 0); digitalWrite(INB2, 1); analogWrite(HIZB, 0);
  delay(2000);
}
void sol() {
  digitalWrite(INA1, LOW); digitalWrite(INA2, HIGH); analogWrite(HIZA, 150);
  digitalWrite(INB1, LOW); digitalWrite(INB2, LOW); analogWrite(HIZB, 150);
}

void sag() {
  digitalWrite(INA1, LOW); digitalWrite(INA2, LOW); analogWrite(HIZA, 150);
  digitalWrite(INB1, LOW); digitalWrite(INB2, HIGH); analogWrite(HIZB, 150);
}

