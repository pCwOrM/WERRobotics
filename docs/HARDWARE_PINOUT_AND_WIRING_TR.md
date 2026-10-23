# 🔌 Donanım Pin Bağlantıları ve Devre Rehberi (`HARDWARE_PINOUT_AND_WIRING_TR.md`)

> 🌐 **Dil Seçimi**: **[🇹🇷 Türkçe](HARDWARE_PINOUT_AND_WIRING_TR.md)** | **[🇬🇧 English](HARDWARE_PINOUT_AND_WIRING.md)**

Bu doküman, `WERRobotics` deposundaki tüm robotik yazılımların (`raw/` ve `firmware/`) Arduino bacak (pin) bağlantılarını içerir. **WERR Edge AI yazılımları (`firmware/`), orijinal donanım şemalarıyla %100 pin uyumludur**; kablolamada hiçbir değişiklik yapmadan doğrudan yüklenebilir.

---

## 1. Otonom Engelden Kaçan Robot (`engeldenkacan.ino` & `werr_engelden_kacan.ino`)

- **Hedef Kart**: Arduino Uno / Arduino Nano (`ATmega328P`)
- **Motor Sürücü**: L298N Çift H-Köprüsü DC Motor Sürücü
- **Mesafe Sensörü**: HC-SR04 Ultrasonik Mesafe Sensörü

| Bileşen / Sinyal | Arduino Pini | Mod | Açıklama |
| :--- | :---: | :---: | :--- |
| `ir_kumanda` | **D2** | `INPUT` | Kızılötesi (IR) alıcı sensör sinyal bacağı |
| `OUT1` | **D5** | `OUTPUT` | Sağ motor ileri yön kontrolü (L298N IN1) |
| `OUT2` | **D6** | `OUTPUT` | Sağ motor geri yön kontrolü (L298N IN2) |
| `OUT3` | **D7** | `OUTPUT` | Sol motor ileri yön kontrolü (L298N IN3) |
| `OUT4` | **D8** | `OUTPUT` | Sol motor geri yön kontrolü (L298N IN4) |
| `motor1` (ENA) | **D9 (PWM)** | `OUTPUT` | Sağ motor PWM hız kontrolü (`0..255`) |
| `motor2` (ENB) | **D10 (PWM)** | `OUTPUT` | Sol motor PWM hız kontrolü (`0..255`) |
| `echo` | **D11** | `INPUT` | HC-SR04 ultrasonik yansıyan ses darbesi |
| `trig` | **D12** | `OUTPUT` | HC-SR04 ultrasonik tetikleme darbesi |

---

## 2. Biyomimetik Biyonik Kelebek (`kelebek.ino` & `werr_kelebek_biyonik.ino`)

- **Hedef Kart**: Arduino Uno / Arduino Nano (`ATmega328P`)
- **Sensör**: KY-037 / KY-038 Mikrofon Ses Algılama Modülü (Analog + Dijital Çıkışlı)
- **Aktüatörler**: 2x SG90 / MG90S Mikro Servo Motor + 1x 5V Röle Modülü

| Bileşen / Sinyal | Arduino Pini | Mod | Açıklama |
| :--- | :---: | :---: | :--- |
| `sesAnalogPin` | **A0** | `INPUT` | Ses modülü `AO` (Analog akustik şiddet) |
| `sesDijitalPin` | **D7** | `INPUT` | Ses modülü `DO` (Dijital eşik tetiklemesi, aktif `LOW`) |
| `srv1` (Sol Kanat) | **D9 (PWM)** | `SERVO` | Sol kanat servosu (`0° .. 140°`) |
| `srv2` (Sağ Kanat) | **D10 (PWM)** | `SERVO` | Sağ kanat servosu (`180° .. 40°` simetrik ters faz) |
| `rolePin` | **D13** | `OUTPUT` | Çift alkışla durum değiştiren röle / ışık çıkışı |

---

## 3. Otonom Market / Çizgi Robotu (`alisveris_roboti.ino` & `werr_alisveris_robotu.ino`)

- **Hedef Kart**: Arduino Mega 2560 (`ATmega2560`)
- **Ekran**: 16x2 Karakter LCD (`LiquidCrystal(54, 55, 56, 57, 58, 59)`)

| Bileşen / Sinyal | Arduino Mega Pini | Mod | Açıklama |
| :--- | :---: | :---: | :--- |
| `INA1`, `INA2` | **D38, D36** | `OUTPUT` | A Motoru yön pinleri |
| `HIZA` | **D44 (PWM)** | `OUTPUT` | A Motoru PWM hız kontrolü |
| `INB1`, `INB2` | **D42, D40** | `OUTPUT` | B Motoru yön pinleri |
| `HIZB` | **D46 (PWM)** | `OUTPUT` | B Motoru PWM hız kontrolü |
| `onFar`, `arkFar` | **D51, D48** | `OUTPUT` | Ön far ve arka stop lambası |
| `solSin`, `sagSin` | **D53, D49** | `OUTPUT` | Sol ve sağ dönüş sinyalleri |
| `solSen`, `ortSen`, `sagSen` | **A6, A7, A8** | `INPUT` | Sol, orta ve sağ kızılötesi engel/çizgi sensörleri |
| `lcd` | **D54..D59 (A0..A5)** | `OUTPUT` | 16x2 paralel LCD ekran pinleri |

---

## 4. İki Ayaklı İnsansı Eşlikçi Robot: Teknobot (`TeknoBotv4_ino.ino` & `werr_teknobot.ino`)

- **Hedef Kart**: Arduino Uno / Arduino Nano (`ATmega328P`)
- **Orijinal Donanım Tasarımı**: Burak Salar (`TeknoBotv4_ino.ino`)
- **Sensörler**: LSM303 3-Eksen İvmeölçer (I2C), HC-SR04 Ultrasonik, LDR Işık, Analog Ses Sensörü, Buzzer
- **Aktüatörler**: 6x SG90 / MG90S Mikro Servo Motor (2 Ayak + 2 Bacak/Kalça + 2 Kol)

| Bileşen / Sinyal | Arduino Pini | Mod | Açıklama |
| :--- | :---: | :---: | :--- |
| `solAyak` | **D2 (PWM)** | `SERVO` | Sol ayak bileği eğim servosu (Merkez: `90°`) |
| `sagAyak` | **D3 (PWM)** | `SERVO` | Sağ ayak bileği eğim servosu (Merkez: `90°`) |
| `solBacak` | **D4 (PWM)** | `SERVO` | Sol bacak/kalça adım servosu (Merkez: `90°`) |
| `sagBacak` | **D5 (PWM)** | `SERVO` | Sağ bacak/kalça adım servosu (Merkez: `90°`) |
| `solKol` | **D6 (PWM)** | `SERVO` | Sol kol servosu (Nötr: `45°`, Kapalı: `15°`, Yukarı: `145°`) |
| `sagKol` | **D7 (PWM)** | `SERVO` | Sağ kol servosu (Nötr: `135°`, Kapalı: `165°`, Yukarı: `35°`) |
| `trigPin` | **D8** | `OUTPUT` | HC-SR04 ultrasonik tetikleme darbesi |
| `echoPin` | **D9** | `INPUT` | HC-SR04 ultrasonik yankı okuma |
| `buzzerPin` | **D13** | `OUTPUT` | Piezo buzzer ses ve alarm çıkışı |
| `ldrPin` | **A0** | `INPUT` | LDR ortam ışığı analog sensör girişi |
| `sesPin` | **A1** | `INPUT` | Analog mikrofon ses şiddeti sensör girişi |
| `LSM303 SDA` | **A4** | `I2C` | LSM303 ivmeölçer I2C veri hattı |
| `LSM303 SCL` | **A5** | `I2C` | LSM303 ivmeölçer I2C saat hattı |

