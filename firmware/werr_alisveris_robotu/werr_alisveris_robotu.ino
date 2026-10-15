/**
 * ============================================================================
 * werr_alisveris_robotu.ino — WERR System-1 Autonomous Market & Line Robot
 * ============================================================================
 * Repository           : https://github.com/pCwOrM/WERRobotics
 * Baseline Sketch      : PrensesMTN/alisveris_roboti/robot_kodlari/robot_kodlari.ino
 * WERR Core Version    : pCwOrM/werr v0.5.1 (Zero-Memory System-1 Edge AI)
 * Active Config Profile: WERR_CFG_MARKET_ROUTER
 *   - resolution = 6 (6x6 = 36 pixels/scale, 108 pixels Tripod — Tesla 3-6-9 harmonic)
 *   - max_iter   = 18 (2 x 9 harmonic depth)
 *   - tripod     = true (3-Point Multi-Scale {0.60x, 1.00x, 1.60x})
 *   - cadence    = (lambda=0.25, alpha=0.50, beta=0.30, deadlock_threshold=0.85)
 *   - temp_choice= 1.25 (Calibrated Softmax Temperature)
 * Connected Seeds      :
 *   1) WERR_SEED_SMART_ROUTER (cx = -0.10109636, cy = 0.95628651, zoom = 45.0)
 * ============================================================================
 */

#include <LiquidCrystal.h>
#include "../../src/WerrCoreEdge/WerrCoreEdge.h"

using namespace werr;

LiquidCrystal lcd(54, 55, 56, 57, 58, 59);

#define INA1  38
#define INA2  36
#define HIZA  44
#define INB1  42
#define INB2  40
#define HIZB  46

#define onFar  51
#define arkFar 48
#define solSin 53
#define sagSin 49

#define solSen A6
#define ortSen A7
#define sagSen A8

int16_t adimSayaci = 1;

void ileri(int hiz);
void geri(int hiz);
void dur();
void sola_don(int hiz);
void saga_don(int hiz);
void isiklariAyarla(int solSinyal, int sagSinyal, int onLamba, int arkaLamba);

void setup() {
    pinMode(INA1, OUTPUT); pinMode(INA2, OUTPUT); pinMode(HIZA, OUTPUT);
    pinMode(INB1, OUTPUT); pinMode(INB2, OUTPUT); pinMode(HIZB, OUTPUT);

    pinMode(onFar, OUTPUT);  pinMode(arkFar, OUTPUT);
    pinMode(solSin, OUTPUT); pinMode(sagSin, OUTPUT);

    pinMode(solSen, INPUT);
    pinMode(ortSen, INPUT);
    pinMode(sagSen, INPUT);

    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("WERRobotics v0.5");
    lcd.setCursor(0, 1);
    lcd.print("MARKET ROBOTU   ");
    delay(800);
}

void loop() {
    int solD = digitalRead(solSen);
    int ortD = digitalRead(ortSen);
    int sagD = digitalRead(sagSen);

    float yanalAsimetri = (float)(sagD - solD);
    float merkezRisk    = (ortD == 0) ? -0.85f : 0.70f;
    if (solD == 0 && ortD == 0 && sagD == 0) {
        merkezRisk = 2.80f;
    }

    WerrPatchMetrics routerMetrics = werrEvaluateWithConfig(
        WERR_SEED_SMART_ROUTER,
        WERR_CFG_MARKET_ROUTER,
        yanalAsimetri,
        merkezRisk,
        merkezRisk
    );

    int werrHiz = 135 + (int)(routerMetrics.avg_escape * 45.0f);

    lcd.setCursor(0, 0);
    lcd.print("S:");
    lcd.print(routerMetrics.hazard_score, 1);
    lcd.print(" Q");
    lcd.print(routerMetrics.dominant_quad + 1);
    lcd.print(" V:");
    lcd.print(werrHiz);
    lcd.print("  ");

    lcd.setCursor(0, 1);
    if (sagD == 0 && solD == 1 && ortD == 1) {
        lcd.print("YON: SAGA DONUS ");
        isiklariAyarla(0, 1, 0, 0);
        saga_don(werrHiz);
    }
    else if (sagD == 1 && solD == 0 && (ortD == 1 || ortD == 0)) {
        lcd.print("YON: SOLA DONUS ");
        isiklariAyarla(1, 0, 0, 0);
        sola_don(werrHiz);
    }
    else if (sagD == 1 && solD == 1 && ortD == 0) {
        lcd.print("YON: ILERI SEYIR");
        isiklariAyarla(0, 0, 1, 0);
        ileri(werrHiz);
    }
    else if (sagD == 0 && solD == 0 && ortD == 0) {
        lcd.print("DURAK: WERR STOP");
        isiklariAyarla(0, 0, 0, 1);
        dur();
        delay(3000);
        ileri(werrHiz);
    }
    else {
        WerrChoiceResult karar = werrEvaluate4TierChoice(
            routerMetrics,
            WERR_CFG_MARKET_ROUTER,
            0.70f,
            yanalAsimetri * 0.15f
        );
        if (karar.scores[1] >= karar.scores[2]) {
            lcd.print("WERR: SOL KAVIS ");
            isiklariAyarla(1, 0, 0, 0);
            sola_don(werrHiz);
        } else {
            lcd.print("WERR: SAG KAVIS ");
            isiklariAyarla(0, 1, 0, 0);
            saga_don(werrHiz);
        }
    }

    adimSayaci++;
}

void isiklariAyarla(int solSinyal, int sagSinyal, int onLamba, int arkaLamba) {
    digitalWrite(solSin, solSinyal);
    digitalWrite(sagSin, sagSinyal);
    digitalWrite(onFar,  onLamba);
    digitalWrite(arkFar, arkaLamba);
}

void geri(int hiz) {
    digitalWrite(INA1, LOW);  digitalWrite(INA2, HIGH); analogWrite(HIZA, hiz);
    digitalWrite(INB1, LOW);  digitalWrite(INB2, HIGH); analogWrite(HIZB, hiz);
}

void ileri(int hiz) {
    digitalWrite(INA1, HIGH); digitalWrite(INA2, LOW);  analogWrite(HIZA, hiz);
    digitalWrite(INB1, HIGH); digitalWrite(INB2, LOW);  analogWrite(HIZB, hiz);
    delay(150);
}

void dur() {
    digitalWrite(INA1, LOW);  digitalWrite(INA2, HIGH); analogWrite(HIZA, 0);
    digitalWrite(INB1, LOW);  digitalWrite(INB2, HIGH); analogWrite(HIZB, 0);
}

void sola_don(int hiz) {
    digitalWrite(INA1, LOW);  digitalWrite(INA2, HIGH); analogWrite(HIZA, hiz);
    digitalWrite(INB1, LOW);  digitalWrite(INB2, LOW);  analogWrite(HIZB, hiz);
    delay(150);
}

void saga_don(int hiz) {
    digitalWrite(INA1, LOW);  digitalWrite(INA2, LOW);  analogWrite(HIZA, hiz);
    digitalWrite(INB1, LOW);  digitalWrite(INB2, HIGH); analogWrite(HIZB, hiz);
    delay(150);
}
