
const int sesDijitalPin = 7;  // Modülün DO pini
const int sesAnalogPin = A0;  // Modülün AO pini
const int rolePin = 13;       // Kontrol edilecek röle pini

#include <Servo.h>

Servo srv1;
Servo srv2;
int alkisSayisi = 0;
unsigned long ilkAlkisZamani = 0;
unsigned long sonSesZamani = 0;
unsigned long sonHareketZamani = 0;  // Kelebeğin en son ne zaman kanat çırptığını tutar

const int algilamaGecikmesi = 200;                 // Debounce süresi (ms)
const int alkisPencerisi = 1000;                   // İkinci alkış için zaman sınırı (ms)
const unsigned long otomatikhareketSuresi = 5000;  // Boş kalma süresi (3 saniye)

void setup() {
  Serial.begin(9600);  // Bilgisayardan ses seviyesini izlemek için
  pinMode(sesDijitalPin, INPUT);
  pinMode(rolePin, OUTPUT);
  digitalWrite(rolePin, LOW);

  srv1.attach(9);
  srv2.attach(10);

  // Kelebeğin kanatlarını başlangıçta açık pozisyona getir
  srv1.write(0);
  srv2.write(180);

  sonHareketZamani = millis();  // Başlangıç zamanını kaydet
}

void loop() {
  // Analog değeri oku ve seri port ekranına yazdır (Hassasiyet ayarı için)
  int analogSesDegeri = analogRead(sesAnalogPin);

  // Dijital durumu oku
  int dijitalSesDurumu = digitalRead(sesDijitalPin);

  // Ses algılandıysa (Sensör tetiklendiğinde genellikle LOW verir)
  if (dijitalSesDurumu == LOW && (millis() - sonSesZamani > algilamaGecikmesi)) {

    Serial.print("Alkış Algılandı! Şiddeti: ");
    Serial.println(analogSesDegeri);

    if (alkisSayisi == 0) {
      ilkAlkisZamani = millis();
      alkisSayisi++;
    } else if (alkisSayisi == 1 && (millis() - ilkAlkisZamani < alkisPencerisi)) {
      alkisSayisi = 0;
      digitalWrite(rolePin, !digitalRead(rolePin));  // Röleyi aç/kapat
      Serial.println(">>> ÇİFT ALKIŞ BAŞARILI: Röle Durumu Değişti! <<<");

      // Alkışla tetiklenen kanat çırpma hareketi
      kanatCirp();
    }
    sonSesZamani = millis();
    sonHareketZamani = millis();  // Ses geldiği için otomatik sayacı da sıfırlıyoruz
  }

  // Zaman aşımı kontrolü (Alkış penceresi için)
  if (alkisSayisi == 1 && (millis() - ilkAlkisZamani > alkisPencerisi)) {
    alkisSayisi = 0;
  }

  // OTOMATİK KANAT ÇIRPMA KONTROLÜ
  // Eğer son kanat çırpışından veya alkıştan sonra 3 saniye geçtiyse
  if (millis() - sonHareketZamani > otomatikhareketSuresi) {
    Serial.println("--- Kelebek uzun süre boş durdu, otomatik çırpınıyor... ---");
    kanatCirp();
  }
}

// Kanat Çırpma Fonksiyonu (Kodun okunabilirliğini artırmak için fonksiyonlaştırdık)
void kanatCirp() {
  // Kanatları kapat (Aşağı İniş - Daha Hızlı)
  for (int i = 0; i <= 130; i++) {
    srv1.write(i);
    srv2.write(180 - i);

    float radyan = radians(map(i, 0, 130, 0, 180));
    int dinamikGecikme = 3 + (1.0 - sin(radyan)) * 8;
    delay(dinamikGecikme);
  }

  // Kanatları aç (Yukarı Kalkış - Daha Yavaş ve Süzülerek)
  for (int i = 130; i >= 0; i--) {
    srv1.write(i);
    srv2.write(180 - i);

    float radyan = radians(map(i, 0, 130, 0, 180));
    int dinamikGecikme = 8 + (1.0 - sin(radyan)) * 12;
    delay(dinamikGecikme);
  }

  // Hareket bittikten sonra sayacı güncelliyoruz ki döngüye girmesin
  sonHareketZamani = millis();
}
