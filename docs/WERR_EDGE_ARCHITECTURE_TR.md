# 📐 WERR v0.5.1 Edge AI Mimari, Matematiksel Model ve Optimum Parametre Kılavuzu (`WerrCoreEdge.h`)

> 🌐 **Dil Seçimi**: **[🇹🇷 Türkçe](WERR_EDGE_ARCHITECTURE_TR.md)** | **[🇬🇧 English](WERR_EDGE_ARCHITECTURE.md)**

Bu belge, **[pCwOrM/werr](https://github.com/pCwOrM/werr) (`v0.5.1`)** deposundaki Sıfır-Bellek Sistem-1 karar çekirdeğinin gömülü mikrodenetleyicilere (`ATmega328P`, `ATmega2560`, `ESP32`) nasıl aktarıldığını, tüm çekirdek parametrelerinin neden bu değerlerle seçildiğini ve fiziksel sensör verilerinin Mandelbrot sınır tohumları ($\partial\mathcal{M}$) üzerinden nasıl motor/servo komutlarına dönüştüğünü açıklar.

---

## 1. WERR Çekirdek Parametrelerinin Varoluş Amacına Göre Kalibrasyonu

`pCwOrM/werr` (`v0.5.1`) motoru; çözünürlük (`resolution`), iterasyon derinliği (`max_iter`), 3-noktalı Tripod (`tripod`), 8 ortogonal çalışma durumu (`[enable_domain, enable_lexical, enable_resonance]`), sınır düzeltme bant genişliği (`bandwidth`), Cadence çatallanma katsayıları (`cadence_lambda`, `cadence_alpha`, `cadence_beta`, `deadlock_threshold`) ve Softmax sıcaklığı (`temp_choice`) olmak üzere zengin bir hiper-parametre uzayına sahiptir.

Gömülü robotikte bu parametrelerin her biri donanımın fiziksel varoluş amacına göre şöyle optimize edilmiştir:

1. **Çözünürlük (`resolution`) ve İterasyon (`max_iter`) — Tesla 3-6-9 Harmonik Uyumu**:
   - **Engelden Kaçan Robot (`WERR_CFG_OBSTACLE_REFLEX`)** ve **Market Robotu (`WERR_CFG_MARKET_ROUTER`)**: Tekerlekli bir araçta en kritik gereksinim **döngü gecikmesinin (latency) `< 1.5 ms`** altında kalmasıdır. `resolution = 6` ($6 \times 6 = 36$ piksel/ölçek, 3-noktalı Tripod ile toplam $108$ piksel) ve `max_iter = 18` ($2 \times 9$) seçildiğinde hem $36, 108, 18$ sayılarının tamamı **$\mathbb{Z}/9\mathbb{Z}$ (dijital kök 9)** harmonik simetrisini korur hem de 16 MHz Arduino Uno üzerinde tek karar adımı **`~1.15 ms`** içinde tamamlanır.
   - **Biyomimetik Biyonik Kelebek (`WERR_CFG_BIONIC_KINEMATICS`)**: Kanat çırpma eylemi doğası gereği saniyeler süren sinüzoidal bir kinematik harekettir; burada milisaniye-altı hızdan ziyade **4-Kadran oranlarının ($Q_1..Q_4$) ince uzamsal hassasiyeti** önemlidir. Bu nedenle `resolution = 12` ($12 \times 12 = 144$ piksel, dijital kök $1+4+4=9$) ve `pCwOrM/werr` kanonik derinliği olan **`max_iter = 36`** ($4 \times 9$) kullanılmıştır.
   - **ESP32 / 32-bit FPU (`WERR_CFG_ESP32_CANONICAL`)**: Donanımsal kayan nokta birimine (FPU) ve 240 MHz hıza sahip kartlar için sunucuyla birebir aynı olan **`resolution = 36, max_iter = 36`** profili hazır tanımlanmıştır.

2. **Ortogonal Durum `[enable_domain=1, enable_lexical=1, enable_resonance=1]` ve 4-Tier Gaussian Keskinliği**:
   - `werr/gates/base.py` içinde `enable_domain = true` olduğunda tanjant düzlemi projeksiyon kazancı `0.45 / zoom` olarak çalışır ve `enable_lexical && enable_resonance` (`[1, 1, 1]` Hibrit mod) aktif olduğunda 4-Katmanlı Gaussian Risk Çekirdeğinin keskinlik parametresi maksimum değer olan **`sharpness = 2.2`** değerini alır. Bu sayede `Tier 0` (İleri Seyir, `-0.85`), `Tier 1` (Sol Kavis, `+0.25`), `Tier 2` (Sağ Kavis, `+1.15`) ve `Tier 3` (Acil Fren/Pivot, `+2.80`) bölgeleri birbirinden bıçak sırtı netlikle ayrılır.

3. **Cadence Süperkritik Dirgen Çatallanması ($\lambda, \alpha, \beta, \tau$)**:
   - Sabit `if-else` kodlu robotların en büyük zaafı, tam karşıdan gelen dik bir duvar veya simetrik köşe karşısında sağa mı sola mı döneceğine karar veremeyip kilitlenmesidir.
   - Tekerlekli robot profillerinde (`WERR_CFG_OBSTACLE_REFLEX` ve `WERR_CFG_MARKET_ROUTER`), çatallanma kuvveti **`cadence_lambda = 0.22..0.25`** ve fraktal kadran kuplajı **`cadence_beta = 0.28..0.30`** seviyesine yükseltilerek simetrik kilitlenmelerin (`top_gap < 0.85`) tek adımda deterministik olarak kırılması sağlanmıştır.

---

## 2. Karmaşık Tanjant Düzlemi Projeksiyonu

Fiziksel bir robotun sensör durumu normalize edilerek $(s_x, s_y, r_{\text{net}})$ üçlüsüne dönüştürülür. Seçilen kanonik tohum $(c_x, c_y, \text{zoom})$ etrafında karmaşık düzlemdeki etkin koordinat $c_{\text{eff}} = c_{x,\text{eff}} + i\,c_{y,\text{eff}}$ şöyle hesaplanır:

$$c_{x,\text{eff}} = c_x + \tanh(x_{\text{pri}}) \cdot \frac{0.45}{\text{zoom}}, \qquad c_{y,\text{eff}} = c_y + \tanh(s_y) \cdot \frac{0.45}{\text{zoom}}$$

$$\text{zoom}_{\text{eff}} = \text{zoom} \cdot \left(1 + 0.10 \sin(s_x + s_y + r_{\text{net}})\right)$$

---

## 3. 3-Noktalı Çok-Ölçekli Tripod Örnekleme ve Sınır Düzeltmesi (`arXiv:1810.11107`)

- Ölçek Çarpanları: $z_f = \{0.60,\; 1.00,\; 1.60\}$
- Ölçek Ağırlıkları: $w_z = \{0.25,\; 0.50,\; 0.25\}$

Her $(r, c)$ ızgara noktasında $z_{k+1} = z_k^2 + c$ iterasyonu çalıştırılır ve kaçış oranı $u = \text{esc} / \text{max\_iter} \in [0, 1]$ bulunur. `[0, 1]` aralığındaki sınır kesilmesini (*boundary truncation*) telafi etmek için Gaussian CDF tabanlı ağırlık düzeltmesi uygulanır:

$$\omega(u, h) = \text{clip}\!\left(\Phi\!\left(\frac{u}{h}\right) + \Phi\!\left(\frac{1-u}{h}\right) - 1,\; 0.45,\; 1.0\right), \qquad W(u, h) = \frac{1}{\omega(u, h)}$$

4 kadranın ($Q_1, Q_2, Q_3, Q_4$) her biri için sınır-kasp oranı ($u \ge 0.90$) ve ortalama kaçış enerjisi birleştirilir:

$$R_q = 0.65 \cdot \frac{\sum_{i \in Q_q} W(u_i)\,\mathbb{I}(u_i \ge 0.90)}{\sum_{i \in Q_q} W(u_i)} + 0.35 \cdot \frac{\sum_{i \in Q_q} W(u_i)\,u_i}{\sum_{i \in Q_q} W(u_i)}, \qquad W_q = (R_q - 0.5) \cdot 6.0$$

---

## 4. 4-Katmanlı (4-Tier) Gaussian Seçim, Cadence Çatallanması ve Softmax ($T=1.25$)

Her eylem adayı $i \in \{0, 1, 2, 3\}$ için kanonik merkezler $c_0 = -0.85$, $c_1 = +0.25$, $c_2 = +1.15$, $c_3 = +2.80$ kullanılarak başlangıç puanı hesaplanır:

$$s_i^{(0)} = 2.5\,\hat{R}_i + 0.5\,(1 - \bar{u}) + 6.0 \exp\!\left(-2.2\,(r_{\text{tier}} - c_i)^2\right)$$

En yüksek iki aday arasındaki fark $\Delta s < 0.85$ ise Cadence Süperkritik Dirgen Çatallanması uygulanır:

$$\Delta s_i = \lambda_{\text{eff}} \sum_{j \neq i} \left[ \text{sgn}(s_i - s_j)\,|s_i - s_j|^{0.5} + \beta\,(W_{q,i} - W_{q,j}) \right]$$

Ardından $T = 1.25$ sıcaklığıyla Softmax olasılıkları $P_i$ ve karar güveni $\text{Conf} = P_{\max,1} - P_{\max,2}$ üretilir.
