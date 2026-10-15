/**
 * ============================================================================
 * werr_engelden_kacan.ino — WERR System-1 Autonomous Obstacle & Reflex Robot
 * ============================================================================
 * Baseline Sketch      : C:\Setup\robotics\engeldenkacan.ino (@PrensesMTN)
 * WERR Core Version    : pCwOrM/werr v0.5.1 (Zero-Memory System-1 Edge AI)
 * Connected Seeds      :
 *   1) WERR_SEED_IOT_SAFETY  (cx = -0.7450, cy = 0.1120, zoom = 85.0) -> Hazard & Proximity Triage
 *   2) WERR_SEED_GAME_COMBAT (cx = -0.7445, cy = 0.1250, zoom = 65.0) -> Tactical Evasion & Vectoring
 *
 * KEY UPGRADES OVER BASELINE:
 *   - Replaces rigid `if (mesafe > 15)` binary cutoff with continuous Mandelbrot
 *     escape-rate hazard dynamics and Homeostatic EMA Dynamic Calibration.
 *   - Replaces blind left-only turns (`motor_hareketleri(1,0,1,0)`) with a 4-candidate
 *     System-1 Tactical Router (Cruise, Agile Left Curve, Agile Right Curve,
 *     Emergency Reverse + Bifurcated Pivot).
 *   - Uses Cadence Supercritical Pitchfork Bifurcation (`werrApplyCadenceBifurcation`)
 *     and GAP-0331 Z/9Z Modular Phase to break symmetric corner/wall deadlocks.
 *   - Modulates PWM motor speeds (`motor1`, `motor2`) continuously via fractal
 *     escape energy.
 * ============================================================================
 */

#include "../../src/WerrCoreEdge/WerrCoreEdge.h"

using namespace werr;

// --- PİN TANIMLAMALARI (Orijinal Donanım Bacakları Korunmuştur) ---
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
int baz_hiz = 240;         // Maksimum seyir PWM hızı
int calisma_modu = 1;      // 1 = WERR Otonom Refleks Modu, 2 = Kumandalı Mod

int onceki_mesafe = 100;   // Hız (yaklaşma türevi) hesabı için bir önceki mesafe (cm)
int16_t manevra_sayaci = 1; // GAP-0331 Z/9Z modüler faz sayacı

// WERR Homeostatik EMA Dinamik Kalibrasyon Motoru
WerrDynamicCalibration werrKalibrasyon(0.15f, 0.50f);

// Karar Adayları (Tactical Evasion Candidates)
enum EvasionAction : uint8_t {
    ACT_CRUISE_FORWARD  = 0,  // Serbest / Orantılı İleri Seyir
    ACT_AGILE_LEFT      = 1,  // Fraktal Sol Kavis (Diferansiyel PWM)
    ACT_AGILE_RIGHT     = 2,  // Fraktal Sağ Kavis (Diferansiyel PWM)
    ACT_EMERGENCY_PIVOT = 3   // Acil Fren + Geri Çekilme + Çatallanmalı Dönüş
};

// Fonksiyon Prototipleri
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
    Serial.println(F(" WERR v0.5.1 EDGE AI — OTONOM ENGELDEN KACAN ROBOT AKTIF"));
    Serial.println(F(" Tohumlar: iot_safety (-0.745, 0.112) & game_combat (-0.7445, 0.125)"));
    Serial.println(F("=========================================================="));
}

// --- ANA DÖNGÜ ---
void loop() {
    int mesafe = mesafe_olcumu();               // Güncel ultrasonik mesafe (cm)
    int delta_d = mesafe - onceki_mesafe;       // Yaklaşma hızı (negatifse engele hızla yaklaşıyor)
    onceki_mesafe = mesafe;

    // 1. Fiziksel Sensör Durumunu Normalize Et (IoTSafetyGate & GameCombatGate Projeksiyonu)
    //    Kritik mesafe bölgesi: 15 cm - 45 cm
    float norm_proximity = werrClamp((45.0f - (float)mesafe) / 30.0f, -1.5f, 2.5f);
    float norm_closing   = werrClamp((float)(-delta_d) / 15.0f, -1.5f, 2.5f);
    float net_risk       = norm_proximity * 1.4f + norm_closing * 0.6f;

    // 2. Tohum 1: WERR_SEED_IOT_SAFETY ile Çarpışma Tehlike Triyajı (Tripod 6x6, 20 iter)
    float adaptif_esik = werrKalibrasyon. adaptive_threshold;
    WerrPatchMetrics safetyMetrics = werrEvaluateState(
        WERR_SEED_IOT_SAFETY,
        norm_proximity,
        norm_closing,
        net_risk,
        6,      // 6x6 mikro-ızgara (ATmega328P üzerinde ~1.4 ms)
        20,     // 20 Mandelbrot iterasyonu
        true,   // 3-Noktalı Tripod (0.60x, 1.00x, 1.60x) aktif
        adaptif_esik
    );

    // Homeostatik EMA eşik güncellemesi
    adaptif_esik = werrKalibrasyon.observeAndCalibrate(safetyMetrics.noul_prob);

    // 3. Tohum 2: WERR_SEED_GAME_COMBAT ile Taktiksel Kaçış & Yönelim Seçimi
    //    GAP-0331 Z/9Z modüler fazı yanal simetri kırıcı olarak projekte edilir
    int8_t mod9_inv = werrConstructiveInverseMod9(manevra_sayaci);
    float phase_bias = (mod9_inv > 0) ? ((float)(mod9_inv - 4) * 0.12f) : 0.0f;

    WerrPatchMetrics combatMetrics = werrEvaluateState(
        WERR_SEED_GAME_COMBAT,
        norm_proximity,
        norm_closing + phase_bias,
        net_risk,
        6,
        20,
        false, // Tek ölçekli hızlı taktik refleks
        adaptif_esik
    );

    // 4. Dört Eylem Adayı İçin Başlangıç Skorlarını Hesapla
    //    [0] İleri Seyir, [1] Sol Kavis, [2] Sağ Kavis, [3] Acil Geri + Dönüş
    float candidate_scores[4];
    float fractal_fields[4] = {
        combatMetrics.quad_weights[0],
        combatMetrics.quad_weights[1],
        combatMetrics.quad_weights[2],
        combatMetrics.quad_weights[3]
    };

    // Mesafe açıkken ACT_CRUISE_FORWARD baskın; daraldıkça kavis veya acil dönüş yükselir
    candidate_scores[ACT_CRUISE_FORWARD]  = (mesafe > 32) ? (2.2f - safetyMetrics.hazard_score) : (-1.5f);
    candidate_scores[ACT_AGILE_LEFT]      = (mesafe <= 35 && mesafe > 15) ? (1.4f + phase_bias) : 0.2f;
    candidate_scores[ACT_AGILE_RIGHT]     = (mesafe <= 35 && mesafe > 15) ? (1.4f - phase_bias) : 0.2f;
    candidate_scores[ACT_EMERGENCY_PIVOT] = (mesafe <= 15 || safetyMetrics.hazard_score > 2.15f) ? 3.2f : -1.0f;

    // 5. Cadence Süperkritik Dirgen Çatallanması (Simetrik Engel Kilitlenmesini Kırar!)
    uint8_t secilen_eylem = werrApplyCadenceBifurcation(
        candidate_scores,
        fractal_fields,
        4,
        0.22f, // lambda_param
        0.50f, // alpha
        0.28f, // beta
        0.85f  // deadlock_threshold
    );

    // 6. Seri Port Telemetri Çıktısı
    Serial.print(F("[WERR-NAV] d="));
    Serial.print(mesafe);
    Serial.print(F("cm | S="));
    Serial.print(safetyMetrics.hazard_score, 2);
    Serial.print(F(" | P(noul)="));
    Serial.print(safetyMetrics.noul_prob, 2);
    Serial.print(F(" | EMA="));
    Serial.print(adaptif_esik, 2);
    Serial.print(F(" | Qdom="));
    Serial.print(combatMetrics.dominant_quad + 1);
    Serial.print(F(" | Act="));
    Serial.println(secilen_eylem);

    // 7. Motor Eylemini Uygula
    if (secilen_eylem == ACT_CRUISE_FORWARD) {
        // Fraktal kaçış enerjisiyle modüle edilen pürüzsüz seyir hızı
        float hiz_carpani = werrClamp(1.15f - 0.30f * safetyMetrics.hazard_score, 0.55f, 1.0f);
        int dinamik_hiz = (int)((float)baz_hiz * hiz_carpani);
        motor_hareketleri(1, 0, 0, 1, dinamik_hiz, dinamik_hiz);
    }
    else if (secilen_eylem == ACT_AGILE_LEFT) {
        // Durmadan akıcı sol kavis (Sağ motor hızlı, Sol motor yavaşlatılmış)
        int hiz_sag = baz_hiz;
        int hiz_sol = (int)((float)baz_hiz * werrClamp(combatMetrics.quad_ratios[1], 0.25f, 0.65f));
        motor_hareketleri(1, 0, 0, 1, hiz_sag, hiz_sol);
        manevra_sayaci++;
    }
    else if (secilen_eylem == ACT_AGILE_RIGHT) {
        // Durmadan akıcı sağ kavis (Sol motor hızlı, Sağ motor yavaşlatılmış)
        int hiz_sag = (int)((float)baz_hiz * werrClamp(combatMetrics.quad_ratios[2], 0.25f, 0.65f));
        int hiz_sol = baz_hiz;
        motor_hareketleri(1, 0, 0, 1, hiz_sag, hiz_sol);
        manevra_sayaci++;
    }
    else {
        // ACT_EMERGENCY_PIVOT: Engele <= 15 cm yaklaşıldı veya WERR Tehlike Skoru S > 2.15
        motor_hareketleri(0, 0, 0, 0, 0, 0); // Önce dur
        delay(150);

        // Fraktal kaçış enerjisine göre dinamik geri çekilme süresi (280 ms - 480 ms)
        int geri_sure = 280 + (int)(safetyMetrics.avg_escape * 200.0f);
        motor_hareketleri(0, 1, 1, 0, baz_hiz, baz_hiz); // Geri gel
        delay(geri_sure);

        // Dönüş yönünü Cadence çatallanma skorlarından (Sol vs Sağ) ve Z/9Z fazından belirle
        bool sola_don_secildi = (candidate_scores[ACT_AGILE_LEFT] >= candidate_scores[ACT_AGILE_RIGHT]);
        // Tesla 3-6-9 rezonans alt-idealinde (I_3 = {0, 3, 6}) kilitlenmeyi kırmak için yönü tersle
        if (werrIsResonantSubidealI3(manevra_sayaci)) {
            sola_don_secildi = !sola_don_secildi;
        }

        int donus_sure = 320 + (int)(combatMetrics.avg_escape * 180.0f);
        if (sola_don_secildi) {
            motor_hareketleri(1, 0, 1, 0, baz_hiz, baz_hiz); // Sola yerinde pivot dönüş
        } else {
            motor_hareketleri(0, 1, 0, 1, baz_hiz, baz_hiz); // Sağa yerinde pivot dönüş
        }
        delay(donus_sure);

        motor_hareketleri(0, 0, 0, 0, 0, 0); // Dur ve yeni ölçüm döngüsüne gir
        manevra_sayaci++;
    }
}

// --- MESAFE ÖLÇÜM FONKSİYONU ---
int mesafe_olcumu() {
    long sure, mesafe;

    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    sure = pulseIn(echo, HIGH, 25000UL); // 25ms timeout (~430cm) kilitlenmeyi önler
    if (sure == 0) {
        mesafe = 250; // Yankı dönmediyse önü açık kabul et
    } else {
        mesafe = sure / 29.1 / 2;
    }
    delay(35);

    return (int)mesafe;
}

// --- MOTOR HAREKETLERİ FONKSİYONU (Bağımsız Sağ/Sol PWM Destekli) ---
void motor_hareketleri(int deger1, int deger2, int deger3, int deger4, int hiz_sag, int hiz_sol) {
    analogWrite(motor1, hiz_sag);
    analogWrite(motor2, hiz_sol);

    digitalWrite(OUT1, deger1);
    digitalWrite(OUT2, deger2);
    digitalWrite(OUT3, deger3);
    digitalWrite(OUT4, deger4);
}
