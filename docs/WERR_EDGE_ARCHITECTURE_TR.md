# 📐 WERR v0.5.1 Edge AI Mimari ve Matematiksel Kılavuzu (`WerrCoreEdge.h`)

Bu belge, **[pCwOrM/werr](https://github.com/pCwOrM/werr) (`v0.5.1`)** deposundaki Sıfır-Bellek Sistem-1 karar çekirdeğinin gömülü mikrodenetleyicilere (`ATmega328P`, `ATmega2560`, `ESP32`) nasıl aktarıldığını ve fiziksel sensör verilerinin Mandelbrot sınır tohumları ($\partial\mathcal{M}$) üzerinden nasıl motor/servo komutlarına dönüştüğünü açıklar.

---

## 1. Karmaşık Tanjant Düzlemi Projeksiyonu

Fiziksel bir robotun sensör durumu (örneğin ultrasonik mesafe $d(t)$, yaklaşma hızı $\Delta d$, veya akustik genlik $A(t)$) normalize edilerek $(s_x, s_y, r_{\text{net}})$ üçlüsüne dönüştürülür. Seçilen kanonik tohum $(c_x, c_y, \text{zoom})$ etrafında karmaşık düzlemdeki etkin koordinat $c_{\text{eff}} = c_{x,\text{eff}} + i\,c_{y,\text{eff}}$ şöyle hesaplanır:

$$c_{x,\text{eff}} = c_x + \tanh(x_{\text{pri}}) \cdot \frac{0.40}{\text{zoom}}, \qquad c_{y,\text{eff}} = c_y + \tanh(s_y) \cdot \frac{0.40}{\text{zoom}}$$

$$\text{zoom}_{\text{eff}} = \text{zoom} \cdot \left(1 + 0.08 \sin(s_x + s_y + r_{\text{net}})\right)$$

Burada $x_{\text{pri}}$, net risk $r_{\text{net}} \neq 0$ olduğunda $r_{\text{net}}$, aksi halde $s_x$ olarak alınır (`werr/engine.py` satır 264 ile birebir parite).

---

## 2. 3-Noktalı Çok-Ölçekli Tripod Örnekleme ve Sınır Düzeltmesi (`arXiv:1810.11107`)

Tek bir yakınlaştırma (zoom) ölçeğinde örnekleme yapmak fraktal örtüşmeye (*aliasing*) yol açabilir. Bunu önlemek için `WerrCoreEdge.h`, her karar adımında 3 ölçekli **Tripod** tensörünü hesaplar:

- Ölçek Çarpanları: $z_f = \{0.60,\; 1.00,\; 1.60\}$
- Ölçek Ağırlıkları: $w_z = \{0.25,\; 0.50,\; 0.25\}$

Her $(r, c)$ ızgara noktasında $z_{k+1} = z_k^2 + c$ iterasyonu çalıştırılır ve kaçış oranı $u = \text{esc} / \text{max\_iter} \in [0, 1]$ bulunur. `[0, 1]` aralığındaki sınır kesilmesini (*boundary truncation*) telafi etmek için Gaussian CDF tabanlı ağırlık düzeltmesi uygulanır:

$$\omega(u, h) = \text{clip}\!\left(\Phi\!\left(\frac{u}{h}\right) + \Phi\!\left(\frac{1-u}{h}\right) - 1,\; 0.45,\; 1.0\right), \qquad W(u, h) = \frac{1}{\omega(u, h)}$$

4 kadranın ($Q_1, Q_2, Q_3, Q_4$) her biri için sınır-kasp oranı ($u \ge 0.90$) ve ortalama kaçış enerjisi birleştirilir:

$$R_q = 0.65 \cdot \frac{\sum_{i \in Q_q} W(u_i)\,\mathbb{I}(u_i \ge 0.90)}{\sum_{i \in Q_q} W(u_i)} + 0.35 \cdot \frac{\sum_{i \in Q_q} W(u_i)\,u_i}{\sum_{i \in Q_q} W(u_i)}, \qquad W_q = (R_q - 0.5) \cdot 6.0$$

---

## 3. Cadence Süperkritik Dirgen Çatallanması (Nodal Deadlock Breaker)

Bir otonom robot tam karşısında simetrik bir engelle karşılaştığında, "Sola Dön" ($s_1$) ve "Sağa Dön" ($s_2$) adaylarının başlangıç puanları birbirine çok yakın olur ($\Delta s = s_{\max,1} - s_{\max,2} < \tau_{\text{deadlock}} = 0.85$). Klasik algoritmalar bu noktada kararsız kalır veya rastgele sayı üretecine (`rand()`) başvurur.

WERR bunun yerine deterministik **Cadence Süperkritik Dirgen Çatallanması** operatörünü uygular:

$$\Delta s_i = \lambda_{\text{eff}} \sum_{j \neq i} \left[ \text{sgn}(s_i - s_j)\,|s_i - s_j|^\alpha + \beta\,(W_{q,i} - W_{q,j}) \right]$$

$$\lambda_{\text{eff}} = \lambda \left(1 + \frac{\tau_{\text{deadlock}} - \Delta s}{\tau_{\text{deadlock}}}\right)$$

Böylece fraktal kadran alanlarındaki ($W_{q,i}$) mikro-asimetri, simetrik kilitlenmeyi deterministik ve tekrarlanabilir biçimde kırar.

---

## 4. GAP-0331 İnşai $\mathbb{Z}/9\mathbb{Z}$ Modüler Cebiri (Tesla 3-6-9 Faz Koruması)

`pCwOrM/werr` v0.5.1 ve `pCwOrM/gap-lean4-port` (`inverseOpExec_correct`, 0 sorry) ile tam uyumlu olarak $\mathbb{Z}/9\mathbb{Z}$ halkası iki ayrık kümeye ayrılır:

1. **Tersinir Birimler Grubu $(\mathbb{Z}/9\mathbb{Z})^\times = \{1, 2, 4, 5, 7, 8\}$**:
   - $1^{-1} \equiv 1$, $2^{-1} \equiv 5$, $4^{-1} \equiv 7$, $5^{-1} \equiv 2$, $7^{-1} \equiv 4$, $8^{-1} \equiv 8 \pmod 9$.
   - `werr_engelden_kacan.ino` içinde her manevra adımında yanal faz sapmasını dengelemek için kullanılır.
2. **Rezonans Alt-İdeali $I_3 = \{0, 3, 6\}$ (Sıfır Bölenler / Tesla 3-6-9 Fazı)**:
   - Engel kaçan robotta her 3., 6., 9. ardışık manevrada köşe sıkışmasını kırmak üzere dönüş yönünü tersler.
   - Biyonik kelebekte her 3., 6., 9. çırpınışta (`MODE_TESLA_HARMONIC`) çift kanat darbesi tetikler.
