/**
 * ============================================================================
 * werr_kelebek_biyonik.ino — WERR System-1 Biomimetic Bionic Butterfly
 * ============================================================================
 * Repository           : https://github.com/pCwOrM/WERRobotics
 * Baseline Sketch      : C:\Setup\robotics\kelebek.ino (@PrensesMTN)
 * WERR Core Version    : pCwOrM/werr v0.5.1 (Zero-Memory System-1 Edge AI)
 * Active Config Profile: WERR_CFG_BIONIC_KINEMATICS
 *   - resolution = 12 (12x12 = 144 pixels/scale, digital root 9)
 *   - max_iter   = 36 (Canonical pCwOrM/werr 36-iteration depth for smooth servo curves)
 *   - tripod     = true (3-Point Multi-Scale {0.60x, 1.00x, 1.60x})
 *   - orthogonal = [1, 1, 1] Hybrid (DomainGate 0.45f proj + 2.2f Gaussian sharpness)
 *   - cadence    = (lambda=0.10, alpha=0.50, beta=0.15, deadlock_threshold=0.85)
 *   - temp_choice= 1.25 (Calibrated Softmax Temperature)
 * Connected Seeds      :
 *   1) WERR_SEED_UNIVERSAL_CUSP (cx = -0.74364389, cy = 0.13182590, zoom = 50.0)
 *   2) WERR_SEED_SMART_ROUTER   (cx = -0.10109636, cy = 0.95628651, zoom = 45.0)
 * ============================================================================
 */

#include <Servo.h>
#include "../../src/WerrCoreEdge/WerrCoreEdge.h"

using namespace werr;

// --- PİN TANIMLAMALARI (Orijinal Donanım Bacakları %100 Korunmuştur) ---
const int sesDijitalPin = 7;   // Ses sensörü DO pini
const int sesAnalogPin  = A0;  // Ses sensörü AO pini
const int rolePin       = 13;  // Kontrol edilecek röle pini

Servo srv1;
Servo srv2;

// --- ZAMANLAMA VE DURUM DEĞİŞKENLERİ ---
int alkisSayisi = 0;
unsigned long ilkAlkisZamani   = 0;
unsigned long sonSesZamani     = 0;
unsigned long sonHareketZamani = 0;

const int algilamaGecikmesi = 200;               // Debounce süresi (ms)
const int alkisPenceresi    = 1000;              // İkinci alkış için zaman sınırı (ms)
unsigned long dinamikOtomatikSure = 5000;        // WERR Q4 tarafından modüle edilen boş kalma süresi

// --- WERR AKUSTİK VE BİYOMİMETİK DURUM ---
float ortamSesTabani = 300.0f;                   // Ortam gürültüsü EMA tabanı
int16_t cirpinisSayaci = 1;                      // GAP-0331 Z/9Z harmonik faz sayacı
WerrDynamicCalibration akustikKalibrasyon(0.15f, 0.50f);

enum ButterflyMode : uint8_t {
    MODE_CALM_SOARING    = 0,  // Sakin Süzülüş (Yumuşak ve Geniş Kanat Çırpışı)
    MODE_STARTLE_REFLEX  = 1,  // Akustik Refleks (Çift Alkışla Hızlı & Güçlü Çırpış)
    MODE_TESLA_HARMONIC  = 2   // Z/9Z I_3 (3-6-9) Rezonans Çift Kanat Darbesi
};

void werrKanatCirp(const WerrPatchMetrics& kinematik, ButterflyMode mod);

void setup() {
    Serial.begin(9600);
    pinMode(sesDijitalPin, INPUT);
    pinMode(rolePin, OUTPUT);
    digitalWrite(rolePin, LOW);

    srv1.attach(9);
    srv2.attach(10);

    srv1.write(0);
    srv2.write(180);

    ortamSesTabani = (float)analogRead(sesAnalogPin);
    sonHareketZamani = millis();

    Serial.println(F("=== WERRobotics v0.5.1 BIYONIK KELEBEK EDGE AI AKTIF ==="));
    Serial.println(F("Profil  : WERR_CFG_BIONIC_KINEMATICS [res=12, iter=36, Tripod=ON]"));
    Serial.println(F("Tohumlar: universal_cusp (-0.7436, 0.1318) & smart_router (-0.1011, 0.9563)"));
}

void loop() {
    unsigned long simdi = millis();

    int analogSesDegeri  = analogRead(sesAnalogPin);
    int dijitalSesDurumu = digitalRead(sesDijitalPin);

    ortamSesTabani = 0.98f * ortamSesTabani + 0.02f * (float)analogSesDegeri;
    float akustikDelta = fabsf((float)analogSesDegeri - ortamSesTabani) / 256.0f;

    if (dijitalSesDurumu == LOW && (simdi - sonSesZamani > (unsigned long)algilamaGecikmesi)) {
        float zamanOrani = werrClamp((float)(simdi - sonSesZamani) / 1000.0f, 0.0f, 2.0f);
        WerrPatchMetrics sesMetrics = werrEvaluateWithConfig(
            WERR_SEED_UNIVERSAL_CUSP,
            WERR_CFG_BIONIC_KINEMATICS,
            akustikDelta,
            1.0f - zamanOrani,
            akustikDelta * 1.5f,
            akustikKalibrasyon.adaptive_threshold
        );
        float yeniEsik = akustikKalibrasyon.observeAndCalibrate(sesMetrics.noul_prob);

        Serial.print(F("Alkis Algilandi! Siddet: "));
        Serial.print(analogSesDegeri);
        Serial.print(F(" | WERR P(noul): "));
        Serial.print(sesMetrics.noul_prob, 2);
        Serial.print(F(" | EMA Esik: "));
        Serial.println(yeniEsik, 2);

        if (alkisSayisi == 0) {
            ilkAlkisZamani = simdi;
            alkisSayisi = 1;
        } else if (alkisSayisi == 1 && (simdi - ilkAlkisZamani < (unsigned long)alkisPenceresi)) {
            alkisSayisi = 0;
            digitalWrite(rolePin, !digitalRead(rolePin));
            Serial.println(F(">>> CIFT ALKIS BASARILI: Role Durumu Degisti & WERR Refleks Kanat Cirpisi! <<<"));

            float ritimFaz = (float)(simdi - ilkAlkisZamani) / (float)alkisPenceresi;
            WerrPatchMetrics kinematik = werrEvaluateWithConfig(
                WERR_SEED_SMART_ROUTER,
                WERR_CFG_BIONIC_KINEMATICS,
                akustikDelta,
                ritimFaz,
                sesMetrics.hazard_score
            );

            ButterflyMode secilenMod = werrIsResonantSubidealI3(cirpinisSayaci)
                                       ? MODE_TESLA_HARMONIC
                                       : MODE_STARTLE_REFLEX;

            werrKanatCirp(kinematik, secilenMod);
        }

        sonSesZamani     = millis();
        sonHareketZamani = millis();
    }

    if (alkisSayisi == 1 && (simdi - ilkAlkisZamani > (unsigned long)alkisPenceresi)) {
        alkisSayisi = 0;
    }

    if (millis() - sonHareketZamani > dinamikOtomatikSure) {
        Serial.print(F("--- Kelebek "));
        Serial.print(dinamikOtomatikSure);
        Serial.println(F(" ms dinlendi, WERR Homeostatik Suzulus basliyor... ---"));

        float dinlenmeFaz = (float)(cirpinisSayaci % 9) / 9.0f;
        WerrPatchMetrics sakinKinematik = werrEvaluateWithConfig(
            WERR_SEED_SMART_ROUTER,
            WERR_CFG_BIONIC_KINEMATICS,
            dinlenmeFaz,
            akustikDelta,
            0.25f
        );

        werrKanatCirp(sakinKinematik, MODE_CALM_SOARING);
    }
}

void werrKanatCirp(const WerrPatchMetrics& kinematik, ButterflyMode mod) {
    int maksAci = 120 + (int)(kinematik.quad_ratios[2] * 20.0f);
    if (mod == MODE_CALM_SOARING) {
        maksAci = 115 + (int)(kinematik.quad_ratios[2] * 15.0f);
    }

    int bazInisGecikme   = (mod == MODE_STARTLE_REFLEX) ? 2 : 3;
    int bazKalkisGecikme = (mod == MODE_STARTLE_REFLEX) ? 6 : 8;
    float inisOlcek      = 6.0f + 4.0f * (1.0f - kinematik.quad_ratios[0]);
    float kalkisOlcek    = 10.0f + 4.0f * (1.0f - kinematik.quad_ratios[1]);

    uint8_t tekrarSayisi = (mod == MODE_TESLA_HARMONIC) ? 2 : 1;

    for (uint8_t tur = 0; tur < tekrarSayisi; ++tur) {
        for (int i = 0; i <= maksAci; i++) {
            srv1.write(i);
            srv2.write(180 - i);

            float radyan = radians(map(i, 0, maksAci, 0, 180));
            int dinamikGecikme = bazInisGecikme + (int)((1.0f - sinf(radyan)) * inisOlcek);
            delay(dinamikGecikme);
        }

        for (int i = maksAci; i >= 0; i--) {
            srv1.write(i);
            srv2.write(180 - i);

            float radyan = radians(map(i, 0, maksAci, 0, 180));
            int dinamikGecikme = bazKalkisGecikme + (int)((1.0f - sinf(radyan)) * kalkisOlcek);
            delay(dinamikGecikme);
        }
    }

    dinamikOtomatikSure = 3500UL + (unsigned long)(kinematik.quad_ratios[3] * 3500.0f);
    cirpinisSayaci++;
    sonHareketZamani = millis();
}
