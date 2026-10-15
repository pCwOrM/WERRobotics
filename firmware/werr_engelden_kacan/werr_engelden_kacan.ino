/**
 * ============================================================================
 * werr_engelden_kacan.ino — WERR System-1 Autonomous Obstacle & Reflex Robot
 * ============================================================================
 * Repository           : https://github.com/pCwOrM/WERRobotics
 * Baseline Sketch      : C:\Setup\robotics\engeldenkacan.ino (@PrensesMTN)
 * WERR Core Version    : pCwOrM/werr v0.5.1 (Zero-Memory System-1 Edge AI)
 * Active Config Profile: WERR_CFG_OBSTACLE_REFLEX
 *   - resolution = 6 (6x6 = 36 pixels/scale, 108 pixels Tripod — Tesla 3-6-9 harmonic)
 *   - max_iter   = 18 (2 x 9 harmonic depth, ~1.15 ms on 16 MHz ATmega328P)
 *   - tripod     = true (3-Point Multi-Scale {0.60x, 1.00x, 1.60x})
 *   - orthogonal = [1, 1, 1] Hybrid (DomainGate 0.45f proj + 2.2f Gaussian sharpness)
 *   - cadence    = (lambda=0.22, alpha=0.50, beta=0.28, deadlock_threshold=0.85)
 *   - temp_choice= 1.25 (Calibrated Softmax Temperature)
 * Connected Seeds      :
 *   1) WERR_SEED_IOT_SAFETY  (cx = -0.7450, cy = 0.1120, zoom = 85.0) -> Hazard & Proximity Triage
 *   2) WERR_SEED_GAME_COMBAT (cx = -0.7445, cy = 0.1250, zoom = 65.0) -> 4-Tier Tactical Evasion
 * ============================================================================
 */

#include "../../src/WerrCoreEdge/WerrCoreEdge.h"

using namespace werr;

// --- PİN TANIMLAMALARI (Orijinal Donanım Bacakları %100 Korunmuştur) ---
const int ir_kumanda = 2;  // IR alıcı sensörünün sinyal bacağı
const int OUT1 = 5;        // Sağ motor ileri
const int OUT2 = 6;        // Sağ motor geri
const int OUT3 = 7;        // Sol motor ileri
const int OUT4 = 8;        // Sol motor geri

const int motor1 = 9;      // Sağ motorların hız kontrolü (PWM)
const int motor2 = 10;     // Sol motorların hız kontrolü (PWM)
const int echo   = 11;     // Ultrasonik sensör yansıyan ses
const int trig   = 12;     // Ultrasonik sensör ses gönderimi

// --- WERR DURUM VE KALİBRASYON DEĞİŞKENLERİ ---
int baz_hiz = 240;          // Maksimum seyir PWM hızı
int calisma_modu = 1;       // 1 = WERR Otonom Refleks Modu, 2 = Kumandalı Mod

int onceki_mesafe = 100;    // Hız (yaklaşma türevi) hesabı için bir önceki mesafe (cm)
int16_t manevra_sayaci = 1; // GAP-0331 Z/9Z modüler faz sayacı

// WERR Homeostatik EMA Dinamik Kalibrasyon Motoru (alpha=0.15, clamp [0.25, 0.75])
WerrDynamicCalibration werrKalibrasyon(0.15f, 0.50f);

// 4-Katmanlı (4-Tier) Taktiksel Karar Adayları (werr/gates/base.py ile birebir uyumlu)
enum EvasionAction : uint8_t {
    ACT_CRUISE_FORWARD  = 0,  // Tier 0 (center = -0.85): Serbest / Orantılı İleri Seyir
    ACT_AGILE_LEFT      = 1,  // Tier 1 (center = +0.25): Fraktal Sol Kavis (Diferansiyel PWM)
    ACT_AGILE_RIGHT     = 2,  // Tier 2 (center = +1.15): Fraktal Sağ Kavis (Diferansiyel PWM)
    ACT_EMERGENCY_PIVOT = 3   // Tier 3 (center = +2.80): Acil Fren + Geri + Çatallanmalı Dönüş
};

int mesafe_olcumu();
void motor_hareketleri(int deger1, int deger2, int deger3, int deger4, int hiz_sag, int hiz_sol);

// --- KURULUM BÖLÜMÜ ---
void setup() {
    Serial.begin(115200);

    pinMode(OUT1, OUTPUT);
    pinMode(OUT2, OUTPUT);
    pinMode(OUT3, OUTPUT);
    pinMode(OUT4, OUTPUT);
    pinMode(motor1, OUTPUT);
    pinMode(motor2, OUTPUT);

    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    pinMode(ir_kumanda, INPUT);

    digitalWrite(OUT1, LOW);
    digitalWrite(OUT2, LOW);
    digitalWrite(OUT3, LOW);
    digitalWrite(OUT4, LOW);
    analogWrite(motor1, 0);
    analogWrite(motor2, 0);

    Serial.println(F("=========================================================="));
    Serial.println(F(" WERRobotics v0.5.1 — OTONOM ENGELDEN KACAN ROBOT AKTIF"));
    Serial.println(F(" Profil  : WERR_CFG_OBSTACLE_REFLEX [res=6, iter=18, Tripod=ON]"));
    Serial.println(F(" Tohumlar: iot_safety (-0.745, 0.112) & game_combat (-0.7445, 0.125)"));
    Serial.println(F("=========================================================="));
}

// --- ANA DÖNGÜ ---
void loop() {
    int mesafe = mesafe_olcumu();               // Güncel ultrasonik mesafe (cm)
    int delta_d = mesafe - onceki_mesafe;       // Yaklaşma hızı (negatifse engele hızla yaklaşıyor)
    onceki_mesafe = mesafe;

    // 1. Fiziksel Sensör Durumunu Normalize Et (IoTSafetyGate & GameCombatGate Projeksiyonu)
    float norm_proximity = werrClamp((45.0f - (float)mesafe) / 30.0f, -1.5f, 2.5f);
    float norm_closing   = werrClamp((float)(-delta_d) / 15.0f, -1.5f, 2.5f);
    float net_risk       = norm_proximity * 1.4f + norm_closing * 0.6f;

    // 2. Tohum 1: WERR_SEED_IOT_SAFETY + WERR_CFG_OBSTACLE_REFLEX ile Çarpışma Triyajı
    float adaptif_esik = werrKalibrasyon.adaptive_threshold;
    WerrPatchMetrics safetyMetrics = werrEvaluateWithConfig(
        WERR_SEED_IOT_SAFETY,
        WERR_CFG_OBSTACLE_REFLEX,
        norm_proximity,
        norm_closing,
        net_risk,
        adaptif_esik
    );

    // Homeostatik EMA eşik güncellemesi
    adaptif_esik = werrKalibrasyon.observeAndCalibrate(safetyMetrics.noul_prob);

    // 3. Tohum 2: WERR_SEED_GAME_COMBAT ile Taktiksel Kaçış & 4-Tier Gaussian Choice
    int8_t mod9_inv = werrConstructiveInverseMod9(manevra_sayaci);
    float phase_bias = (mod9_inv > 0) ? ((float)(mod9_inv - 4) * 0.15f) : 0.0f;

    WerrPatchMetrics combatMetrics = werrEvaluateWithConfig(
        WERR_SEED_GAME_COMBAT,
        WERR_CFG_OBSTACLE_REFLEX,
        norm_proximity,
        norm_closing + phase_bias,
        net_risk,
        adaptif_esik
    );

    // 4-Katmanlı Risk Haritası (werr/gates/base.py :: _project_4tier_choice_risk)
    //   d > 35 cm        -> tier_risk = -0.85 (Tier 0: Doğrudan İleri Seyir)
    //   15 < d <= 35 cm  -> tier_risk = +0.70 (Tier 1 & Tier 2: Sol/Sağ Çevik Kavis)
    //   d <= 15 cm       -> tier_risk = +2.80 (Tier 3: Acil Dur + Geri + Pivot Dönüş)
    float tier_risk = -0.85f;
    if (mesafe <= 15 || safetyMetrics.hazard_score > 2.25f) {
        tier_risk = 2.80f;
    } else if (mesafe <= 35) {
        tier_risk = 0.70f;
    }

    WerrChoiceResult karar = werrEvaluate4TierChoice(
        combatMetrics,
        WERR_CFG_OBSTACLE_REFLEX,
        tier_risk,
        phase_bias
    );
    uint8_t secilen_eylem = karar.choice_idx;

    // 4. Seri Port Telemetri Çıktısı
    Serial.print(F("[WERR-NAV] d="));
    Serial.print(mesafe);
    Serial.print(F("cm | S="));
    Serial.print(safetyMetrics.hazard_score, 2);
    Serial.print(F(" | P(noul)="));
    Serial.print(safetyMetrics.noul_prob, 2);
    Serial.print(F(" | Conf="));
    Serial.print(karar.confidence, 2);
    Serial.print(F(" | Act="));
    Serial.println(secilen_eylem);

    // 5. Motor Eylemini Uygula
    if (secilen_eylem == ACT_CRUISE_FORWARD) {
        float hiz_carpani = werrClamp(1.15f - 0.30f * safetyMetrics.hazard_score, 0.55f, 1.0f);
        int dinamik_hiz = (int)((float)baz_hiz * hiz_carpani);
        motor_hareketleri(1, 0, 0, 1, dinamik_hiz, dinamik_hiz);
    }
    else if (secilen_eylem == ACT_AGILE_LEFT) {
        int hiz_sag = baz_hiz;
        int hiz_sol = (int)((float)baz_hiz * werrClamp(combatMetrics.quad_ratios[1], 0.25f, 0.65f));
        motor_hareketleri(1, 0, 0, 1, hiz_sag, hiz_sol);
        manevra_sayaci++;
    }
    else if (secilen_eylem == ACT_AGILE_RIGHT) {
        int hiz_sag = (int)((float)baz_hiz * werrClamp(combatMetrics.quad_ratios[2], 0.25f, 0.65f));
        int hiz_sol = baz_hiz;
        motor_hareketleri(1, 0, 0, 1, hiz_sag, hiz_sol);
        manevra_sayaci++;
    }
    else {
        // ACT_EMERGENCY_PIVOT: Engele <= 15 cm yaklaşıldı
        motor_hareketleri(0, 0, 0, 0, 0, 0);
        delay(150);

        int geri_sure = 280 + (int)(safetyMetrics.avg_escape * 200.0f);
        motor_hareketleri(0, 1, 1, 0, baz_hiz, baz_hiz);
        delay(geri_sure);

        bool sola_don_secildi = (karar.scores[ACT_AGILE_LEFT] >= karar.scores[ACT_AGILE_RIGHT]);
        if (werrIsResonantSubidealI3(manevra_sayaci)) {
            sola_don_secildi = !sola_don_secildi;
        }

        int donus_sure = 320 + (int)(combatMetrics.avg_escape * 180.0f);
        if (sola_don_secildi) {
            motor_hareketleri(1, 0, 1, 0, baz_hiz, baz_hiz);
        } else {
            motor_hareketleri(0, 1, 0, 1, baz_hiz, baz_hiz);
        }
        delay(donus_sure);

        motor_hareketleri(0, 0, 0, 0, 0, 0);
        manevra_sayaci++;
    }
}

int mesafe_olcumu() {
    long sure, mesafe;

    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    sure = pulseIn(echo, HIGH, 25000UL);
    if (sure == 0) {
        mesafe = 250;
    } else {
        mesafe = sure / 29.1 / 2;
    }
    delay(35);

    return (int)mesafe;
}

void motor_hareketleri(int deger1, int deger2, int deger3, int deger4, int hiz_sag, int hiz_sol) {
    analogWrite(motor1, hiz_sag);
    analogWrite(motor2, hiz_sol);

    digitalWrite(OUT1, deger1);
    digitalWrite(OUT2, deger2);
    digitalWrite(OUT3, deger3);
    digitalWrite(OUT4, deger4);
}
