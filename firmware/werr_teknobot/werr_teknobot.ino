/**
 * ============================================================================
 * werr_teknobot.ino — WERR v0.5.1 Zero-Memory System-1 Edge AI Teknobot Humanoid
 * ============================================================================
 * Repository           : https://github.com/pCwOrM/WERRobotics
 * Baseline Author      : Burak Salar (TeknoBotv4_ino.ino)
 * Edge AI Architecture : Volkan Dagli (@pCwOrM) & Prenses Metin (@PrensesMTN)
 * License              : BSL 1.1 (Business Source License 1.1 - ITouch Systems)
 *
 * ACTIVE CONFIG PROFILE: WERR_CFG_BIPED_HUMANOID
 *   - Grid Resolution  : 6x6 (36 pixels per scale, 108 total Tripod)
 *   - Iteration Depth  : 18 (Multiples of 9, sub-1.2 ms reflex latency on 16 MHz AVR)
 *   - Multi-Scale      : 3-Point Tripod {0.60x, 1.00x, 1.60x}
 *   - Sharpness Bitmask: [1, 1, 1] Hybrid (DomainGate 0.45f + 2.2f Gaussian Sharpness)
 *   - Cadence Config   : lambda=0.20, alpha=0.50, beta=0.22, deadlock_threshold=0.85
 *   - Softmax Temp     : 1.25 (Calibrated JevBench temperature)
 *
 * CONNECTED CANONICAL MANDELBROT SEEDS:
 *   1) WERR_SEED_SECURITY_GUARD (cx = -0.74364389, cy = 0.13182590, zoom = 120.0)
 *      -> Postural Equilibrium & 3-Axis Accel (LSM303) Anti-Fall Guard.
 *   2) WERR_SEED_IOT_SAFETY     (cx = -0.74500000, cy = 0.11200000, zoom = 85.0)
 *      -> Ultrasonic Proximity d(t) and Closing Rate Delta d / Delta t Hazard Triage.
 *   3) WERR_SEED_UNIVERSAL_CUSP (cx = -0.74364389, cy = 0.13182590, zoom = 50.0)
 *      -> Acoustic Clapping & Beat Resonance with Dynamic EMA Noise Floor.
 *   4) WERR_SEED_SMART_ROUTER   (cx = -0.10109636, cy = 0.95628651, zoom = 45.0)
 *      -> 4-Quadrant Bipedal Gait Kinematics & Organic Idle Breathing.
 *
 * HARDWARE PINOUT (100% Preserved from Burak Salar's Teknobot):
 *   - Servos: solAyak=2, sagAyak=3, solBacak=4, sagBacak=5, solKol=6, sagKol=7
 *   - I2C Accel: LSM303 (SDA=A4, SCL=A5)
 *   - Ultrasonic: trig=8, echo=9
 *   - Buzzer: 13
 *   - LDR Sensor: A0
 *   - Sound Sensor: A1
 * ============================================================================
 */

#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include "../../src/WerrCoreEdge/WerrCoreEdge.h"

using namespace werr;

// ============================================================================
// 1. HARDWARE PIN & SERVO DEFINITIONS (BURAK SALAR TEKNOBOT PINOUT)
// ============================================================================
Servo solAyak, sagAyak, solBacak, sagBacak, solKol, sagKol;
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

const int trigPin   = 8;
const int echoPin   = 9;
const int buzzerPin = 13;
const int ldrPin    = A0;
const int sesPin    = A1;

// Calibrated Servo Angles
const int AYAK_MERKEZ     = 90;
const int BACAK_MERKEZ    = 90;
const int KOL_SOL_NOTR    = 45;
const int KOL_SAG_NOTR    = 135;
const int KOL_SOL_KAPALI  = 15;
const int KOL_SAG_KAPALI  = 165;
const int KOL_SOL_YUKARI  = 145;
const int KOL_SAG_YUKARI  = 35;

// Walking Direction Multiplier
const int YURUME_YONU     = 1; 

// Light Sensor Thresholds with Hysteresis
const int LDR_UYKU_ESIK   = 800; // Darkness (Sleep)
const int LDR_UYANIS_ESIK = 450; // Light (Wake)

// ============================================================================
// 2. WERR EDGE AI STATE ENGINE & DYNAMIC CALIBRATORS
// ============================================================================
WerrDynamicCalibration posturalKalibrasyon(0.15f, 0.50f);
WerrDynamicCalibration akustikKalibrasyon(0.12f, 0.50f);
WerrDynamicCalibration proximityKalibrasyon(0.18f, 0.50f);

float ortamSesTabani      = 855.0f; // Dynamic EMA sound floor
long oncekiMesafe         = 50;     // Previous distance for delta d / delta t
unsigned long sonDongu    = 0;
unsigned long sonIdleZamani = 0;
int idleAdim              = 0;
bool uykudaMi             = false;
bool lsmHazir             = false;
int16_t bipedRefleksSayaci = 1;     // GAP-0331 Z/9Z harmonic maneuver counter

enum TeknobotBehavior : uint8_t {
    BEH_IDLE_BREATH        = 0, // Rhythmic organic resting sway
    BEH_GREETING_WAVE      = 1, // Friendly wave & bipedal curiosity lean
    BEH_BACKWARD_EVASION   = 2, // Close obstacle collision evasion
    BEH_MOONWALK_DANCE     = 3, // Synchronized smooth moonwalk dance
    BEH_FALL_RECOVERY      = 4, // Anti-fall flailing & self-righting alarm
    BEH_CIRCADIAN_SLEEP    = 5  // Low-power sleep with folded arms
};

// Function prototypes
void servolariOrtala();
void kollariKapat();
void kollariKaldir();
void uyanisAnimasyonu();
void windowsKapanisSesi();
void panikSesi();
void cirpinmaAnimasyonu();
void werrGeriYuruAdim(int adimSayisi, float strideModulator);
void werrMoonwalkYavas(float rhythmTempo);
void werrStaggeredWrite(Servo &s1, int a1, Servo &s2, int a2, uint8_t delayMs);
long mesafeOlcHizli();

// ============================================================================
// 3. SETUP
// ============================================================================
void setup() {
    Serial.begin(9600);
    delay(200);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    pinMode(sesPin, INPUT);

    Serial.println(F("\n========================================================"));
    Serial.println(F("   TEKNOBOT V4 + WERR v0.5.1 ZERO-MEMORY EDGE AI CORE   "));
    Serial.println(F("========================================================"));
    Serial.println(F("Profile : WERR_CFG_BIPED_HUMANOID (6x6 @ 18 iter, Tripod=ON)"));
    Serial.println(F("Heap RAM: 0 Bytes (Pure Zero-Malloc Stack Guarantee)"));

    Wire.begin();
    #if defined(WIRE_HAS_TIMEOUT)
    Wire.setWireTimeout(3000, true);
    #endif

    if (accel.begin()) {
        lsmHazir = true;
        Serial.println(F("[OK] LSM303 3-Eksen Ivmeolcer aktif."));
    } else {
        Serial.println(F("[!] LSM303 bulunamadi - Postur fallback modu."));
    }

    // Anti-Inrush Staggered Servo Initialization (Prevents brownout resets)
    solAyak.attach(2);   solAyak.write(AYAK_MERKEZ);   delay(80);
    sagAyak.attach(3);   sagAyak.write(AYAK_MERKEZ);   delay(80);
    solBacak.attach(4);  solBacak.write(BACAK_MERKEZ); delay(80);
    sagBacak.attach(5);  sagBacak.write(BACAK_MERKEZ); delay(80);
    solKol.attach(6);    solKol.write(KOL_SOL_NOTR);   delay(80);
    sagKol.attach(7);    sagKol.write(KOL_SAG_NOTR);   delay(80);

    // Initial sound floor calibration
    ortamSesTabani = (float)analogRead(sesPin);

    uyanisAnimasyonu();
    Serial.println(F("[HAZIR] Teknobot WERR Omurilik Refleksi Calisiyor!"));
}

// ============================================================================
// 4. MAIN WERR SYSTEM-1 REFLEX LOOP
// ============================================================================
void loop() {
    unsigned long simdi = millis();
    unsigned long dt = (sonDongu > 0) ? (simdi - sonDongu) : 20;
    sonDongu = simdi;

    // ------------------------------------------------------------------------
    // SENSOR READINGS & NORMALIZATION
    // ------------------------------------------------------------------------
    long mesafe = mesafeOlcHizli();
    long deltaMesafe = mesafe - oncekiMesafe;
    oncekiMesafe = mesafe;

    int ldrHam = analogRead(ldrPin);
    int sesHam = analogRead(sesPin);

    // Dynamic EMA Sound Floor Tracking (Adapts to ambient room acoustics)
    ortamSesTabani = 0.96f * ortamSesTabani + 0.04f * (float)sesHam;
    float akustikDelta = fabsf((float)sesHam - ortamSesTabani);

    // ------------------------------------------------------------------------
    // WERR SEED 1: CIRCADIAN SLEEP / WAKE GATE
    // ------------------------------------------------------------------------
    if (!uykudaMi && ldrHam > LDR_UYKU_ESIK) {
        Serial.println(F("[WERR-CIRCADIAN] Karanlik tespit edildi, uykuya geciliyor..."));
        windowsKapanisSesi();
        kollariKapat();
        uykudaMi = true;
        delay(200);
        return;
    } 
    else if (uykudaMi && ldrHam < LDR_UYANIS_ESIK) {
        Serial.println(F("[WERR-CIRCADIAN] Isik acildi, Teknobot uyaniyor..."));
        uyanisAnimasyonu();
        uykudaMi = false;
        delay(200);
    }

    if (uykudaMi) {
        delay(120);
        return;
    }

    // ------------------------------------------------------------------------
    // WERR SEED 2: WERR_SEED_SECURITY_GUARD -> ACCELEROMETER POSTURE & FALL TRIAGE
    // ------------------------------------------------------------------------
    bool robotDustu = false;
    float tiltEnerjisi = 0.0f;

    if (lsmHazir) {
        sensors_event_t event;
        accel.getEvent(&event);
        float ax = event.acceleration.x;
        float ay = event.acceleration.y;
        float az = event.acceleration.z;

        // Vector magnitude deviation from 1G gravity (9.81 m/s^2)
        tiltEnerjisi = sqrtf(ax * ax + ay * ay + (az - 9.81f) * (az - 9.81f));

        // Normalize posture states for WERR boundary projection
        float norm_pitch = werrClamp(ax / 7.0f, -1.5f, 2.5f);
        float norm_roll  = werrClamp(ay / 7.0f, -1.5f, 2.5f);
        float norm_tilt  = werrClamp(tiltEnerjisi / 8.0f, -1.5f, 2.5f);

        WerrPatchMetrics posturMetrics = werrEvaluateWithConfig(
            WERR_SEED_SECURITY_GUARD,
            WERR_CFG_BIPED_HUMANOID,
            norm_pitch,
            norm_roll,
            norm_tilt,
            posturalKalibrasyon.adaptive_threshold
        );
        posturalKalibrasyon.observeAndCalibrate(posturMetrics.noul_prob);

        // Security Guard boundary alert: Extreme tip-over or rapid acceleration loss
        robotDustu = (posturMetrics.noul_prob >= 0.72f || fabsf(ax) > 6.2f || fabsf(ay) > 6.8f || az < 3.8f);
    }

    // ------------------------------------------------------------------------
    // WERR SEED 3: WERR_SEED_IOT_SAFETY -> ULTRASONIC PROXIMITY & RATE TRIAGE
    // ------------------------------------------------------------------------
    float norm_prox = werrClamp((40.0f - (float)mesafe) / 25.0f, -1.5f, 2.5f);
    float norm_clos = werrClamp((float)(-deltaMesafe) / 12.0f, -1.5f, 2.5f);
    float net_risk  = norm_prox * 1.4f + norm_clos * 0.6f;

    WerrPatchMetrics safetyMetrics = werrEvaluateWithConfig(
        WERR_SEED_IOT_SAFETY,
        WERR_CFG_BIPED_HUMANOID,
        norm_prox,
        norm_clos,
        net_risk,
        proximityKalibrasyon.adaptive_threshold
    );
    proximityKalibrasyon.observeAndCalibrate(safetyMetrics.noul_prob);

    // ------------------------------------------------------------------------
    // WERR SEED 4: WERR_SEED_UNIVERSAL_CUSP -> ACOUSTIC CLAPPING / BEAT TRIAGE
    // ------------------------------------------------------------------------
    float norm_sound = werrClamp(akustikDelta / 120.0f, -1.5f, 2.5f);
    WerrPatchMetrics akustikMetrics = werrEvaluateWithConfig(
        WERR_SEED_UNIVERSAL_CUSP,
        WERR_CFG_BIPED_HUMANOID,
        norm_sound,
        0.50f,
        norm_sound * 1.5f,
        akustikKalibrasyon.adaptive_threshold
    );
    akustikKalibrasyon.observeAndCalibrate(akustikMetrics.noul_prob);

    bool gucluSesTespitEdildi = (akustikDelta > 65.0f && akustikMetrics.noul_prob > 0.65f);

    // ------------------------------------------------------------------------
    // WERR SEED 5: 4-TIER MULTI-BEHAVIOR ARBITRATION VIA CADENCE BIFURCATION
    // ------------------------------------------------------------------------
    // Map environmental signals into 4 behavioral candidates:
    // Candidate 0: Cruise Idle / Rhythmic Sway
    // Candidate 1: Greeting Wave (15 - 35 cm proximity)
    // Candidate 2: Sound Beat / Moonwalk Dance
    // Candidate 3: Emergency Backward Evasion (distance < 15 cm or High Hazard)

    float tier_risk = -0.85f;
    if (mesafe > 0 && (mesafe < 15 || safetyMetrics.hazard_score > 2.15f)) {
        tier_risk = 2.80f; // Evasion Tier
    } else if (gucluSesTespitEdildi) {
        tier_risk = 1.15f; // Musical Beat Tier
    } else if (mesafe >= 15 && mesafe <= 35) {
        tier_risk = 0.25f; // Social Greeting Tier
    }

    int8_t mod9_phase = werrConstructiveInverseMod9(bipedRefleksSayaci);
    float phase_bias = (mod9_phase > 0) ? ((float)(mod9_phase - 4) * 0.12f) : 0.0f;

    WerrChoiceResult karar = werrEvaluate4TierChoice(
        safetyMetrics,
        WERR_CFG_BIPED_HUMANOID,
        tier_risk,
        phase_bias
    );

    // ========================================================================
    // 5. PRIORITY EXECUTIVE ACTIONS
    // ========================================================================

    // 1. HIGHEST PRIORITY: ACCELEROMETER FALL / TIP-OVER ALARM
    if (robotDustu) {
        Serial.println(F("[WERR-REFLEX] ALARM: ROBOT DEVRILDI! Guvenli Cirpinma Aktif."));
        panikSesi();
        cirpinmaAnimasyonu();
        servolariOrtala();
        bipedRefleksSayaci++;
        delay(150);
        return;
    }

    // 2. TIER 3 / DISTANCE < 15 CM: RETROGRADE EVASIVE STRIDE (GERİ YÜRÜME)
    if (karar.choice_idx == 3 || (mesafe > 0 && mesafe < 15)) {
        Serial.print(F("[WERR-REFLEX] Yakin Engel (d="));
        Serial.print(mesafe);
        Serial.print(F("cm, S="));
        Serial.print(safetyMetrics.hazard_score, 2);
        Serial.println(F(")! Yumusak Geri Yuruyus..."));

        float strideMod = werrClamp(safetyMetrics.avg_escape * 1.5f, 0.8f, 1.4f);
        werrGeriYuruAdim(2, strideMod);
        servolariOrtala();
        bipedRefleksSayaci++;
        delay(100);
    }
    // 3. TIER 2 / HIGH SOUND: WERR-STABILIZED MOONWALK DANCE
    else if (karar.choice_idx == 2 || gucluSesTespitEdildi) {
        Serial.print(F("[WERR-DANS] Akustik Rezonans (Delta="));
        Serial.print(akustikDelta, 1);
        Serial.println(F(")! Moonwalk Senkronizasyonu..."));

        float tempo = werrClamp(akustikMetrics.avg_escape * 1.2f, 0.7f, 1.3f);
        werrMoonwalkYavas(tempo);
        servolariOrtala();
        bipedRefleksSayaci++;
        delay(100);
    }
    // 4. TIER 1 / 15-35 CM: DYNAMIC GREETING & WAVING
    else if (karar.choice_idx == 1 || (mesafe >= 15 && mesafe <= 35)) {
        Serial.print(F("[WERR-SOSYAL] Selamlama Bolgesi (d="));
        Serial.print(mesafe);
        Serial.println(F(" cm). Kol Sallaniyor..."));

        tone(buzzerPin, 1000, 100); delay(110);
        tone(buzzerPin, 1500, 120); delay(130);

        // Friendly double wave
        sagKol.write(80);           delay(140);
        sagKol.write(KOL_SAG_NOTR); delay(140);
        sagKol.write(75);           delay(140);
        sagKol.write(KOL_SAG_NOTR); delay(140);
        bipedRefleksSayaci++;
    }
    // 5. TIER 0 / IDLE: ORGANIC BIPEDAL BREATHING SWAY (SMART ROUTER MODULATION)
    else {
        if (simdi - sonIdleZamani > 1000) {
            idleAdim++;
            WerrPatchMetrics routerMetrics = werrEvaluateWithConfig(
                WERR_SEED_SMART_ROUTER,
                WERR_CFG_BIPED_HUMANOID,
                0.10f * (float)(idleAdim % 4),
                -0.10f,
                0.20f,
                0.50f
            );

            int bacakSalinim = 8 + (int)(routerMetrics.quad_ratios[1] * 8.0f);

            if (idleAdim == 1) {
                solBacak.write(BACAK_MERKEZ + bacakSalinim);
            } else if (idleAdim == 2) {
                solBacak.write(BACAK_MERKEZ);
            } else if (idleAdim == 3) {
                sagBacak.write(BACAK_MERKEZ - bacakSalinim);
            } else {
                sagBacak.write(BACAK_MERKEZ);
                idleAdim = 0;
            }
            sonIdleZamani = simdi;
        }
    }
}

// ============================================================================
// 6. MOTOR & KINEMATIC GAIT FUNCTIONS (STAGGERED ANTI-BROWNOUT SERVO WRITES)
// ============================================================================

/**
 * Micro-staggered write: Delays 4ms between left and right motor pulses
 * to flatten simultaneous current inrush spikes and prevent brownouts.
 */
void werrStaggeredWrite(Servo &s1, int a1, Servo &s2, int a2, uint8_t delayMs) {
    s1.write(a1);
    delay(4);
    s2.write(a2);
    if (delayMs > 4) {
        delay(delayMs - 4);
    }
}

/**
 * WERR Backward Walking Gait: Smooth, natural stride with staggered pulses.
 */
void werrGeriYuruAdim(int adimSayisi, float strideModulator) {
    int bacakSalinim = (int)(20.0f * (float)YURUME_YONU * strideModulator);

    for (int i = 0; i < adimSayisi; i++) {
        tone(buzzerPin, 1200, 60);

        // 1. Shift weight to left foot
        werrStaggeredWrite(solAyak, AYAK_MERKEZ + 20, sagAyak, AYAK_MERKEZ + 12, 180);

        // 2. Rotate hips backward
        werrStaggeredWrite(solBacak, BACAK_MERKEZ - bacakSalinim, sagBacak, BACAK_MERKEZ - bacakSalinim, 180);

        // 3. Feet flat on ground
        werrStaggeredWrite(solAyak, AYAK_MERKEZ, sagAyak, AYAK_MERKEZ, 160);

        tone(buzzerPin, 1200, 60);

        // 4. Shift weight to right foot
        werrStaggeredWrite(sagAyak, AYAK_MERKEZ - 20, solAyak, AYAK_MERKEZ - 12, 180);

        // 5. Rotate hips opposite direction
        werrStaggeredWrite(solBacak, BACAK_MERKEZ + bacakSalinim, sagBacak, BACAK_MERKEZ + bacakSalinim, 180);

        // 6. Feet flat on ground
        werrStaggeredWrite(solAyak, AYAK_MERKEZ, sagAyak, AYAK_MERKEZ, 160);
    }
}

/**
 * Smooth Foot Slide for Moonwalk.
 */
void werrAyakKaydir(Servo &m1, int bas1, int bit1, Servo &m2, int bas2, int bit2, int hizMs) {
    for (int adim = 0; adim <= 16; adim++) {
        int a1 = map(adim, 0, 16, bas1, bit1);
        int a2 = map(adim, 0, 16, bas2, bit2);
        m1.write(a1);
        delay(hizMs / 2);
        m2.write(a2);
        delay(hizMs / 2);
    }
}

/**
 * WERR Synchronized Soft Moonwalk (Brownout-Protected).
 */
void werrMoonwalkYavas(float rhythmTempo) {
    int notalar[] = {330, 392, 440, 392, 330, 294};
    int kaymaHizi = (int)(16.0f / rhythmTempo);
    if (kaymaHizi < 10) kaymaHizi = 10;

    solBacak.write(BACAK_MERKEZ);
    sagBacak.write(BACAK_MERKEZ);

    for (int tur = 0; tur < 2; tur++) {
        // Phase 1: Right arm lifts, left foot smoothly slides to tiptoe (90 -> 125)
        tone(buzzerPin, notalar[(tur * 3) % 6], 100);
        sagKol.write(90);
        werrAyakKaydir(solAyak, 90, 125, sagAyak, 90, 90, kaymaHizi);

        // Phase 2: Left foot descends while right foot glides (gliding illusion)
        tone(buzzerPin, notalar[(tur * 3 + 1) % 6], 100);
        sagKol.write(KOL_SAG_NOTR);
        werrAyakKaydir(solAyak, 125, 90, sagAyak, 90, 55, kaymaHizi);

        // Phase 3: Left arm lifts, right foot descends
        tone(buzzerPin, notalar[(tur * 3 + 2) % 6], 100);
        solKol.write(90);
        werrAyakKaydir(solAyak, 90, 60, sagAyak, 55, 90, kaymaHizi);

        // Phase 4: Symmetrical return to center
        tone(buzzerPin, 330, 100);
        solKol.write(KOL_SOL_NOTR);
        werrAyakKaydir(solAyak, 60, 90, sagAyak, 90, 120, kaymaHizi);
        werrAyakKaydir(solAyak, 90, 90, sagAyak, 120, 90, kaymaHizi);
    }

    tone(buzzerPin, 523, 100); delay(120);
    tone(buzzerPin, 659, 150); delay(160);
    noTone(buzzerPin);
}

// ============================================================================
// 7. UTILITY & SOUND ANIMATIONS
// ============================================================================

void cirpinmaAnimasyonu() {
    for (int i = 0; i < 3; i++) {
        solKol.write(KOL_SOL_YUKARI); 
        sagKol.write(KOL_SAG_NOTR);
        solAyak.write(120);           
        sagAyak.write(120);
        delay(120);
        
        solKol.write(KOL_SOL_NOTR);   
        sagKol.write(KOL_SAG_YUKARI);
        solAyak.write(60);            
        sagAyak.write(60);
        delay(120);
    }
}

void panikSesi() {
    for (int i = 0; i < 4; i++) {
        tone(buzzerPin, 2500, 60); delay(65);
        tone(buzzerPin, 1500, 60); delay(65);
    }
}

void windowsKapanisSesi() {
    tone(buzzerPin, 466, 140); delay(150);
    tone(buzzerPin, 349, 140); delay(150);
    tone(buzzerPin, 311, 180); delay(190);
    tone(buzzerPin, 208, 400); delay(420);
    noTone(buzzerPin);
}

void servolariOrtala() {
    solAyak.write(AYAK_MERKEZ);
    sagAyak.write(AYAK_MERKEZ);
    solBacak.write(BACAK_MERKEZ);
    sagBacak.write(BACAK_MERKEZ);
    solKol.write(KOL_SOL_NOTR);
    sagKol.write(KOL_SAG_NOTR);
}

void kollariKapat() {
    solKol.write(KOL_SOL_KAPALI);
    delay(60);
    sagKol.write(KOL_SAG_KAPALI);
    solAyak.write(AYAK_MERKEZ);
    sagAyak.write(AYAK_MERKEZ);
}

void kollariKaldir() {
    solKol.write(KOL_SOL_YUKARI);
    delay(50);
    sagKol.write(KOL_SAG_YUKARI);
}

void uyanisAnimasyonu() {
    tone(buzzerPin, 500, 120);  delay(120);
    tone(buzzerPin, 800, 120);  delay(120);
    tone(buzzerPin, 1200, 220); delay(220);
    kollariKaldir();
    delay(300);
    servolariOrtala();
}

long mesafeOlcHizli() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // 20ms hardware timeout ensures no CPU hang
    long sure = pulseIn(echoPin, HIGH, 20000UL);
    if (sure == 0) return 999;
    return sure / 58.2;
}
