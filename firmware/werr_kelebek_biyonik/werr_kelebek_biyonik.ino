/**
 * ============================================================================
 * werr_kelebek_biyonik.ino — WERR System-1 Biomimetic Bionic Butterfly
 * ============================================================================
 * Baseline Sketch      : C:\Setup\robotics\kelebek.ino (@PrensesMTN)
 * WERR Core Version    : pCwOrM/werr v0.5.1 (Zero-Memory System-1 Edge AI)
 * Connected Seeds      :
 *   1) WERR_SEED_UNIVERSAL_CUSP (cx = -0.74364389, cy = 0.13182590, zoom = 50.0)
 *      -> Acoustic Energy & Inter-Clap Rhythm Resonance Triage
 *   2) WERR_SEED_SMART_ROUTER   (cx = -0.10109636, cy = 0.95628651, zoom = 45.0)
 *      -> 4-Quadrant Biomimetic Wing Kinematics & Homeostatic Rest Modulation
 *
 * KEY UPGRADES OVER BASELINE:
 *   - Projects analog sound intensity (`A0`), inter-clap timing (`delta_t`),
 *     and ambient noise baseline onto the Mandelbrot Universal Cusp.
 *   - Uses WERR Homeostatic EMA Dynamic Calibration (`WerrDynamicCalibration`)
 *     to adapt to noisy vs. quiet rooms automatically.
 *   - Replaces fixed wing stroke angles (0..130 deg) and fixed delays with
 *     4-Quadrant Fractal Wing Kinematics:
 *       * Q1 ratio -> Downstroke thrust velocity
 *       * Q2 ratio -> Upstroke soaring glide velocity
 *       * Q3 ratio -> Dynamic wing stroke amplitude (95 deg .. 140 deg)
 *       * Q4 ratio -> Homeostatic spontaneous flap period (3000 ms .. 7500 ms,
 *                     inspired by WerrSoma GABAergic homeostasis)
 *   - Incorporates GAP-0331 Z/9Z (Tesla 3-6-9) harmonic double-flap burst on
 *     every 3rd/6th/9th resonant cycle (sub-ideal I_3).
 * ============================================================================
 */

#include <Servo.h>
#include "../../src/WerrCoreEdge/WerrCoreEdge.h"

using namespace werr;

// --- PİN TANIMLAMALARI (Orijinal Donanım Bacakları Korunmuştur) ---
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

// Biyomimetik Uçuş Kipleri
enum ButterflyMode : uint8_t {
    MODE_CALM_SOARING    = 0,  // Sakin Süzülüş (Yumuşak ve Geniş Kanat Çırpışı)
    MODE_STARTLE_REFLEX  = 1,  // Akustik Refleks (Çift Alkışla Hızlı & Güçlü Çırpış)
    MODE_TESLA_HARMONIC  = 2   // Z/9Z I_3 (3-6-9) Rezonans Çift Kanat Darbesi
};

// Fonksiyon Prototipleri
void werrKanatCirp(const WerrPatchMetrics& kinematik, ButterflyMode mod);

void setup() {
    Serial.begin(9600); // Orijinal baud hızı korunmuştur (9600 bps)
    pinMode(sesDijitalPin, INPUT);
    pinMode(rolePin, OUTPUT);
    digitalWrite(rolePin, LOW);

    srv1.attach(9);
    srv2.attach(10);

    // Kelebeğin kanatlarını başlangıçta açık pozisyona getir
    srv1.write(0);
    srv2.write(180);

    ortamSesTabani = (float)analogRead(sesAnalogPin);
    sonHareketZamani = millis();

    Serial.println(F("=== WERR v0.5.1 BIYONIK KELEBEK EDGE AI AKTIF ==="));
    Serial.println(F("Tohumlar: universal_cusp (-0.7436, 0.1318) & smart_router (-0.1011, 0.9563)"));
}

void loop() {
    unsigned long simdi = millis();

    // 1. Analog ve Dijital Ses Değerlerini Oku
    int analogSesDegeri  = analogRead(sesAnalogPin);
    int dijitalSesDurumu = digitalRead(sesDijitalPin);

    // Ortam gürültü tabanını yavaşça güncelle (Homeostatik arka plan izleme)
    ortamSesTabani = 0.98f * ortamSesTabani + 0.02f * (float)analogSesDegeri;
    float akustikDelta = fabsf((float)analogSesDegeri - ortamSesTabani) / 256.0f;

    // 2. Ses Algılandıysa (Dijital LOW tetiklemesi + Debounce)
    if (dijitalSesDurumu == LOW && (simdi - sonSesZamani > (unsigned long)algilamaGecikmesi)) {
        // WERR Universal Cusp üzerinde akustik darbe değerlendirmesi
        float zamanOrani = werrClamp((float)(simdi - sonSesZamani) / 1000.0f, 0.0f, 2.0f);
        WerrPatchMetrics sesMetrics = werrEvaluateState(
            WERR_SEED_UNIVERSAL_CUSP,
            akustikDelta,
            1.0f - zamanOrani,
            akustikDelta * 1.5f,
            6,
            20,
            true,
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
            digitalWrite(rolePin, !digitalRead(rolePin)); // Röleyi aç/kapat
            Serial.println(F(">>> CIFT ALKIS BASARILI: Role Durumu Degisti & WERR Refleks Kanat Cirpisi! <<<"));

            // Smart Router Tohumu ile Kanat Kinematik Parametrelerini Sentezle
            float ritimFaz = (float)(simdi - ilkAlkisZamani) / (float)alkisPenceresi;
            WerrPatchMetrics kinematik = werrEvaluateState(
                WERR_SEED_SMART_ROUTER,
                akustikDelta,
                ritimFaz,
                sesMetrics.hazard_score,
                8,
                24,
                true
            );

            // GAP-0331 Z/9Z I_3 = {0, 3, 6} rezonans kontrolü
            ButterflyMode secilenMod = werrIsResonantSubidealI3(cirpinisSayaci)
                                       ? MODE_TESLA_HARMONIC
                                       : MODE_STARTLE_REFLEX;

            werrKanatCirp(kinematik, secilenMod);
        }

        sonSesZamani     = millis();
        sonHareketZamani = millis();
    }

    // 3. Zaman Aşımı Kontrolü (Alkış penceresi için)
    if (alkisSayisi == 1 && (simdi - ilkAlkisZamani > (unsigned long)alkisPenceresi)) {
        alkisSayisi = 0;
    }

    // 4. WERR HOMEOSTATİK OTOMATİK KANAT ÇIRPMA KONTROLÜ
    if (millis() - sonHareketZamani > dinamikOtomatikSure) {
        Serial.print(F("--- Kelebek "));
        Serial.print(dinamikOtomatikSure);
        Serial.println(F(" ms dinlendi, WERR Homeostatik Suzulus basliyor... ---"));

        float dinlenmeFaz = (float)(cirpinisSayaci % 9) / 9.0f;
        WerrPatchMetrics sakinKinematik = werrEvaluateState(
            WERR_SEED_SMART_ROUTER,
            dinlenmeFaz,
            akustikDelta,
            0.25f,
            6,
            20,
            true
        );

        werrKanatCirp(sakinKinematik, MODE_CALM_SOARING);
    }
}

/**
 * WERR 4-Kadran Fraktal Kanat Çırpma Sentezleyicisi:
 *   - Q1 (quad_ratios[0]): Aşağı İniş (Downstroke) hız katsayısı
 *   - Q2 (quad_ratios[1]): Yukarı Kalkış (Upstroke) süzülme katsayısı
 *   - Q3 (quad_ratios[2]): Kanat açı genliği (110° .. 140°)
 *   - Q4 (quad_ratios[3]): Bir sonraki otomatik çırpışa kadar homeostatik dinlenme süresi
 */
void werrKanatCirp(const WerrPatchMetrics& kinematik, ButterflyMode mod) {
    // Q3 oranından dinamik kanat kapanma açısı (Orijinal 130° etrafında fraktal modülasyon)
    int maksAci = 120 + (int)(kinematik.quad_ratios[2] * 20.0f); // 120..140 derece
    if (mod == MODE_CALM_SOARING) {
        maksAci = 115 + (int)(kinematik.quad_ratios[2] * 15.0f);
    }

    // Q1 ve Q2 oranlarından iniş ve kalkış baz gecikmeleri (ms)
    int bazInisGecikme   = (mod == MODE_STARTLE_REFLEX) ? 2 : 3;
    int bazKalkisGecikme = (mod == MODE_STARTLE_REFLEX) ? 6 : 8;
    float inisOlcek      = 6.0f + 4.0f * (1.0f - kinematik.quad_ratios[0]);
    float kalkisOlcek    = 10.0f + 4.0f * (1.0f - kinematik.quad_ratios[1]);

    uint8_t tekrarSayisi = (mod == MODE_TESLA_HARMONIC) ? 2 : 1;

    for (uint8_t tur = 0; tur < tekrarSayisi; ++tur) {
        // Kanatları kapat (Aşağı İniş - Daha Hızlı & İtki Odaklı)
        for (int i = 0; i <= maksAci; i++) {
            srv1.write(i);
            srv2.write(180 - i);

            float radyan = radians(map(i, 0, maksAci, 0, 180));
            int dinamikGecikme = bazInisGecikme + (int)((1.0f - sinf(radyan)) * inisOlcek);
            delay(dinamikGecikme);
        }

        // Kanatları aç (Yukarı Kalkış - Daha Yavaş ve Süzülerek)
        for (int i = maksAci; i >= 0; i--) {
            srv1.write(i);
            srv2.write(180 - i);

            float radyan = radians(map(i, 0, maksAci, 0, 180));
            int dinamikGecikme = bazKalkisGecikme + (int)((1.0f - sinf(radyan)) * kalkisOlcek);
            delay(dinamikGecikme);
        }
    }

    // Q4 kadran ağırlığına göre bir sonraki otomatik çırpınma süresini (3500 ms - 7000 ms) belirle
    dinamikOtomatikSure = 3500UL + (unsigned long)(kinematik.quad_ratios[3] * 3500.0f);
    cirpinisSayaci++;
    sonHareketZamani = millis();
}
