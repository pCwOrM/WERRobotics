
// --- PİN TANIMLAMALARI ---
const int ir_kumanda = 2;  // IR alıcı sensörünün sinyal bacağı
const int OUT1 = 5;        // Sağ motor ileri
const int OUT2 = 6;        // Sağ motor geri
const int OUT3 = 7;        // Sol motor ileri
const int OUT4 = 8;        // Sol motor geri

const int motor1 = 9;   // Sağ motorların hız kontrolü (PWM)
const int motor2 = 10;  // Sol motorların hız kontrolü (PWM)
const int echo = 11;    // Ultrasonik sensör yansıyan ses
const int trig = 12;    // Ultrasonik sensör ses gönderimi

// --- DEĞİŞKENLER ---
int hiz = 240;         // Aracın başlangıç hızı
int calisma_modu = 2;  // 1 = Engelden Kaçan Mod, 2 = Kumandalı Mod (Başlangıçta kumandalı başlasın)



// --- KURULUM BÖLÜMÜ ---
void setup() {
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  digitalWrite(OUT1, LOW);
  digitalWrite(OUT2, LOW);
  digitalWrite(OUT3, LOW);
  digitalWrite(OUT4, LOW);
  analogWrite(motor1, 0);
  analogWrite(motor2, 0);
}

// --- ANA DÖNGÜ ---
void loop() {

  int mesafe = mesafe_olcumu();  // Mesafeyi sürekli ölç

  if (mesafe > 15) {
    // Engel yoksa sürekli ileri git
    motor_hareketleri(1, 0, 0, 1, hiz);
  } else {
    // Engele yaklaşıldıysa (15 cm veya daha az)
    motor_hareketleri(0, 0, 0, 0, 0);  // Önce dur
    delay(200);                        // 0.2 saniye bekle

    motor_hareketleri(0, 1, 1, 0, hiz);  // Biraz geri gel
    delay(400);                          // 0.4 saniye boyunca geri gel

    motor_hareketleri(1, 0, 1, 0, hiz);  // Kurtulmak için sola dön
    delay(400);                          // 0.4 saniye boyunca dön

    motor_hareketleri(0, 0, 0, 0, 0);  // Dur ve döngüyü tekrar başlat (yeni mesafeyi ölçecek)
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

  sure = pulseIn(echo, HIGH);
  mesafe = sure / 29.1 / 2;
  delay(50);

  return (mesafe);
}

// --- MOTOR HAREKETLERİ FONKSİYONU ---
void motor_hareketleri(int deger1, int deger2, int deger3, int deger4, int hiz_degeri) {
  analogWrite(motor1, hiz_degeri);
  analogWrite(motor2, hiz_degeri);

  digitalWrite(OUT1, deger1);
  digitalWrite(OUT2, deger2);
  digitalWrite(OUT3, deger3);
  digitalWrite(OUT4, deger4);
}