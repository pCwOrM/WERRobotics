#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

// =============================================================
// 1. NESNE VE PİN TANIMLAMALARI
// =============================================================
Servo solAyak, sagAyak, solBacak, sagBacak, solKol, sagKol;
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

const int trigPin   = 8;
const int echoPin   = 9;
const int buzzerPin = 13;
const int ldrPin    = A0;
const int sesPin    = A1;

// =============================================================
// 2. KALİBRASYON SABİTLERİ
// =============================================================
const int SES_BOS_DEGERI  = 855; 
const int SES_HASSASIYETI = 70;  

const int LDR_UYKU_ESIK   = 800; // Karanlık (Uyku)
const int LDR_UYANIS_ESIK = 450; // Aydınlık (Uyanış)

// Servo Merkez ve Ofset Açıları
const int AYAK_MERKEZ     = 90;
const int BACAK_MERKEZ    = 90;
const int KOL_SOL_NOTR    = 45;
const int KOL_SAG_NOTR    = 135;
const int KOL_SOL_KAPALI  = 15;
const int KOL_SAG_KAPALI  = 165;
const int KOL_SOL_YUKARI  = 145;
const int KOL_SAG_YUKARI  = 35;

// Yürüme Yönü Çarpanı (İleri yürürse -1 yapın)
const int YURUME_YONU     = 1; 

unsigned long sonIdleZamani = 0;
int idleAdim = 0;
bool uykudaMi = false;
bool lsmHazir = false;

// =============================================================
// 3. KURULUM (SETUP)
// =============================================================
void setup() {
  Serial.begin(9600);
  delay(300);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(sesPin, INPUT);

  Serial.println("\n--- TEKNOBOT V4 BASLATILIYOR ---");

  Wire.begin();
  #if defined(WIRE_HAS_TIMEOUT)
    Wire.setWireTimeout(3000, true);
  #endif

  if (accel.begin()) {
    lsmHazir = true;
    Serial.println("[OK] LSM303 Ivmeolcer aktif.");
  } else {
    Serial.println("[!] LSM303 baslatilamadi!");
  }

  // Kademeli Servo Başlatma
  solAyak.attach(2);   solAyak.write(AYAK_MERKEZ);  delay(120);
  sagAyak.attach(3);   sagAyak.write(AYAK_MERKEZ);  delay(120);
  solBacak.attach(4);  solBacak.write(BACAK_MERKEZ); delay(120);
  sagBacak.attach(5);  sagBacak.write(BACAK_MERKEZ); delay(120);
  solKol.attach(6);    solKol.write(KOL_SOL_NOTR);   delay(120);
  sagKol.attach(7);    sagKol.write(KOL_SAG_NOTR);   delay(120);

  uyanisAnimasyonu();
  Serial.println("[HAZIR] Teknobot V4 Calisiyor!");
}

// =============================================================
// 4. ANA DÖNGÜ (LOOP)
// =============================================================
void loop() {
  long mesafe = mesafeOlc();
  int ldrHam = analogRead(ldrPin);
  int sesDegeri = analogRead(sesPin);

  bool yuksekSesVar = abs(sesDegeri - SES_BOS_DEGERI) > SES_HASSASIYETI;

  bool dustuMu = false;
  if (lsmHazir) {
    sensors_event_t event;
    accel.getEvent(&event);
    float x = event.acceleration.x;
    float y = event.acceleration.y;
    float z = event.acceleration.z;
    dustuMu = (abs(x) > 6.0 || abs(y) > 6.5 || z < 4.0);
  }

  // --- UYKU VE UYANIŞ KONTROLÜ ---
  if (!uykudaMi && ldrHam > LDR_UYKU_ESIK) {
    Serial.println("[DURUM] Karanlik tespit edildi, uyunuyor...");
    windowsKapanisSesi();
    kollariKapat();
    uykudaMi = true;
    delay(300);
    return;
  } 
  else if (uykudaMi && ldrHam < LDR_UYANIS_ESIK) {
    Serial.println("[DURUM] Isik acildi, uyaniyor...");
    uyanisAnimasyonu();
    uykudaMi = false;
    delay(300);
  }

  if (uykudaMi) {
    delay(100);
    return;
  }

  // --- ÖNCELİKLİ TEPKİLER ---

  // 1. DÜŞME / DEVRİLME (Eski hızlı hali)
  if (dustuMu) {
    Serial.println("[ALARM] ROBOT DEVRILDI!");
    panikSesi();
    cirpinmaAnimasyonu();
    servolariOrtala();
    delay(300);
  }
  // 2. ÇOK YAKIN ENGEL (0 - 15 cm) -> GERİ YÜRÜME
  else if (mesafe > 0 && mesafe < 15) {
    Serial.print("[TEPKI] Cok yakin engel (");
    Serial.print(mesafe);
    Serial.println(" cm)! Geri yuruyor...");
    geriYuru(2);
    servolariOrtala();
    delay(300);
  }
  // 3. YÜKSEK SES -> YAVAŞ MOONWALK DANS (Resetlenmeyi önleyen yumuşak mod)
  else if (yuksekSesVar) {
    Serial.print("[DANS] Yuksek ses (");
    Serial.print(sesDegeri);
    Serial.println(")! Yavas Moonwalk basliyor...");
    moonwalkYavas();
    servolariOrtala();
    delay(300);
  }
  // 4. SELAMLAMA (15 - 35 cm) (Eski hızlı hali)
  else if (mesafe >= 15 && mesafe <= 35) {
    Serial.print("[TEPKI] Selamlama: ");
    Serial.println(mesafe);
    tone(buzzerPin, 1000, 120); delay(130);
    tone(buzzerPin, 1500, 120); delay(130);

    sagKol.write(80);           delay(180);
    sagKol.write(KOL_SAG_NOTR); delay(180);
    sagKol.write(80);           delay(180);
    sagKol.write(KOL_SAG_NOTR); delay(180);
  }
  // 5. BOŞTA (IDLE) DURUMU (Eski hızlı hali)
  else {
    servolariOrtala();
    if (millis() - sonIdleZamani > 1200) {
      idleAdim++;
      if (idleAdim == 1)      { solBacak.write(100); }
      else if (idleAdim == 2) { solBacak.write(90);  }
      else if (idleAdim == 3) { sagBacak.write(80);  }
      else                    { sagBacak.write(90); idleAdim = 0; }
      sonIdleZamani = millis();
    }
  }
}

// =============================================================
// 5. YENİ ÖZELLİKLER: GERİ YÜRÜME VE YAVAŞ MOONWALK
// =============================================================

// Geri yürüme: Motorlar sırayla ve 250ms doğal adım aralığıyla hareket eder
void geriYuru(int adimSayisi) {
  int bacakSalinim = 20 * YURUME_YONU;

  for (int i = 0; i < adimSayisi; i++) {
    tone(buzzerPin, 1200, 80);

    // 1. Ağırlığı sol ayağa ver
    solAyak.write(AYAK_MERKEZ + 20);
    delay(60);
    sagAyak.write(AYAK_MERKEZ + 12);
    delay(240);

    // 2. Kalçaları geriye çevir
    solBacak.write(BACAK_MERKEZ - bacakSalinim);
    delay(60);
    sagBacak.write(BACAK_MERKEZ - bacakSalinim);
    delay(240);

    // 3. Ayakları yere düz bas
    solAyak.write(AYAK_MERKEZ);
    delay(60);
    sagAyak.write(AYAK_MERKEZ);
    delay(220);

    tone(buzzerPin, 1200, 80);

    // 4. Ağırlığı sağ ayağa ver
    sagAyak.write(AYAK_MERKEZ - 20);
    delay(60);
    solAyak.write(AYAK_MERKEZ - 12);
    delay(240);

    // 5. Kalçaları ters yöne çevir
    solBacak.write(BACAK_MERKEZ + bacakSalinim);
    delay(60);
    sagBacak.write(BACAK_MERKEZ + bacakSalinim);
    delay(240);

    // 6. Ayakları tekrar düzle
    solAyak.write(AYAK_MERKEZ);
    delay(60);
    sagAyak.write(AYAK_MERKEZ);
    delay(220);
  }
}

// Sadece Moonwalk içinde kullanılan yumuşak geçiş yardımcı fonksiyonu
void ayakKaydir(Servo &m1, int bas1, int bit1, Servo &m2, int bas2, int bit2, int hizMs) {
  for (int adim = 0; adim <= 20; adim++) {
    int a1 = map(adim, 0, 20, bas1, bit1);
    int a2 = map(adim, 0, 20, bas2, bit2);
    m1.write(a1);
    delay(hizMs / 2);
    m2.write(a2);
    delay(hizMs / 2);
  }
}

// Yüksek seste çalışan Yavaş ve Reset Korumalı Moonwalk
void moonwalkYavas() {
  int notalar[] = {330, 392, 440, 392, 330, 294};
  int kaymaHizi = 18; // Moonwalk hızı (Büyütürsen daha da yavaşlar)

  // Başlangıçta bacakları merkezde tut
  solBacak.write(90);
  sagBacak.write(90);

  for (int tur = 0; tur < 2; tur++) {
    // 1. Sağ kol hafif kalkar, sol ayak yavaşça parmak ucuna kayar (90 -> 125)
    tone(buzzerPin, notalar[(tur * 3) % 6], 120);
    sagKol.write(90);
    ayakKaydir(solAyak, 90, 125, sagAyak, 90, 90, kaymaHizi);

    // 2. Sol ayak inerken sağ ayak yavaşça yükselir (Kayma efekti)
    tone(buzzerPin, notalar[(tur * 3 + 1) % 6], 120);
    sagKol.write(KOL_SAG_NOTR);
    ayakKaydir(solAyak, 125, 90, sagAyak, 90, 55, kaymaHizi);

    // 3. Sol kol hafif kalkar, sağ ayak inerken sol ayak ters yöne bükülür
    tone(buzzerPin, notalar[(tur * 3 + 2) % 6], 120);
    solKol.write(90);
    ayakKaydir(solAyak, 90, 60, sagAyak, 55, 90, kaymaHizi);

    // 4. Ayaklar merkeze süzülür
    tone(buzzerPin, 330, 120);
    solKol.write(KOL_SOL_NOTR);
    ayakKaydir(solAyak, 60, 90, sagAyak, 90, 120, kaymaHizi);
    ayakKaydir(solAyak, 90, 90, sagAyak, 120, 90, kaymaHizi);
  }

  // Bitiş melodisi
  tone(buzzerPin, 523, 120); delay(140);
  tone(buzzerPin, 659, 180); delay(200);
  noTone(buzzerPin);
}

// =============================================================
// 6. ESKİ HIZLI STANDART FONKSİYONLAR
// =============================================================

void cirpinmaAnimasyonu() {
  for (int i = 0; i < 3; i++) {
    solKol.write(KOL_SOL_YUKARI); 
    sagKol.write(KOL_SAG_NOTR);
    solAyak.write(120);           
    sagAyak.write(120);
    delay(150);
    
    solKol.write(KOL_SOL_NOTR);   
    sagKol.write(KOL_SAG_YUKARI);
    solAyak.write(60);            
    sagAyak.write(60);
    delay(150);
  }
}

void panikSesi() {
  for (int i = 0; i < 4; i++) {
    tone(buzzerPin, 2500, 70); delay(75);
    tone(buzzerPin, 1500, 70); delay(75);
  }
}

void windowsKapanisSesi() {
  tone(buzzerPin, 466, 160); delay(180);
  tone(buzzerPin, 349, 160); delay(180);
  tone(buzzerPin, 311, 200); delay(220);
  tone(buzzerPin, 208, 450); delay(480);
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
  delay(120);
  sagKol.write(KOL_SAG_KAPALI);
  solAyak.write(AYAK_MERKEZ);
  sagAyak.write(AYAK_MERKEZ);
}

void kollariKaldir() {
  solKol.write(KOL_SOL_YUKARI);
  delay(80);
  sagKol.write(KOL_SAG_YUKARI);
}

void uyanisAnimasyonu() {
  tone(buzzerPin, 500, 140);  delay(140);
  tone(buzzerPin, 800, 140);  delay(140);
  tone(buzzerPin, 1200, 260); delay(260);
  kollariKaldir();
  delay(400);
  servolariOrtala();
}

long mesafeOlc() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long sure = pulseIn(echoPin, HIGH, 22000);
  if (sure == 0) return 999;
  return sure / 58.2;
}