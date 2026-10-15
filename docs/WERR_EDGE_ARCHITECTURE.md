# 📐 WERR v0.5.1 Edge AI Architecture, Mathematical Model & Optimal Parameter Guide (`WerrCoreEdge.h`)

> 🌐 **Language**: **[🇬🇧 English](WERR_EDGE_ARCHITECTURE.md)** | **[🇹🇷 Türkçe](WERR_EDGE_ARCHITECTURE_TR.md)**

This document details how the Zero-Memory System-1 decision engine from **[pCwOrM/werr](https://github.com/pCwOrM/werr) (`v0.5.1`)** is mapped onto bare-metal microcontrollers (`ATmega328P`, `ATmega2560`, `ESP32`), why each hyperparameter profile is optimal for its target robotic platform, and how physical sensor streams are projected onto canonical Mandelbrot boundary seeds ($\partial\mathcal{M}$).

---

## 1. Existential Alignment of WERR Core Parameters

The `pCwOrM/werr` (`v0.5.1`) engine exposes a rich parameter manifold (`resolution`, `max_iter`, `tripod`, 8 orthogonal modes `[enable_domain, enable_lexical, enable_resonance]`, `bandwidth`, `cusp_cutoff`, `cadence_lambda`, `cadence_alpha`, `cadence_beta`, `deadlock_threshold`, and `temp_choice`).

In `WerrCoreEdge.h`, these parameters are pre-calibrated into four `WerrConfig` profiles aligned with each robot's physical purpose:

1. **Resolution (`resolution`) & Iteration Depth (`max_iter`) — Tesla 3-6-9 Harmonic Symmetry**:
   - **Obstacle Avoidance Robot (`WERR_CFG_OBSTACLE_REFLEX`)** & **Market Robot (`WERR_CFG_MARKET_ROUTER`)**: Wheeled collision avoidance requires `< 1.5 ms` loop latency. Choosing `resolution = 6` ($6 \times 6 = 36$ pixels/scale, $108$ pixels across the 3-Point Tripod) and `max_iter = 18` ($2 \times 9$) preserves exact **$\mathbb{Z}/9\mathbb{Z}$ (digital root 9)** harmonic symmetry while executing in **`~1.15 ms`** on a 16 MHz 8-bit ATmega328P.
   - **Biomimetic Bionic Butterfly (`WERR_CFG_BIONIC_KINEMATICS`)**: Wing flapping is a multi-second sinusoidal kinematic stroke where spatial granularity across the 4 quadrants ($Q_1..Q_4$) governs stroke smoothness. Thus `resolution = 12` ($12 \times 12 = 144$ pixels, digital root $9$) and the canonical **`max_iter = 36`** ($4 \times 9$) are selected.
   - **32-Bit FPU Microcontrollers (`WERR_CFG_ESP32_CANONICAL`)**: Uses exact desktop parity (`resolution = 36, max_iter = 36`).

2. **Orthogonal State `[1, 1, 1]` & 4-Tier Gaussian Kernel Sharpness (`2.2`)**:
   - Enabling `enable_domain = true` activates the `DomainGate` tangent projection gain (`0.45 / zoom`), while `[enable_lexical=1, enable_resonance=1]` sets the 4-Tier Gaussian risk kernel sharpness to its optimal hybrid value **`sharpness = 2.2`**, cleanly separating Tier 0 (`-0.85`, Cruise Forward), Tier 1 (`+0.25`, Agile Left Curve), Tier 2 (`+1.15`, Agile Right Curve), and Tier 3 (`+2.80`, Emergency Reverse & Pivot).

3. **Cadence Supercritical Pitchfork Bifurcation ($\lambda, \alpha, \beta, \tau$)**:
   - Wheeled robots facing a head-on wall or symmetrical corner suffer from tie-score deadlocks between Left and Right turns. Elevating `cadence_lambda = 0.22..0.25` and `cadence_beta = 0.28..0.30` ensures deterministic symmetry breaking whenever `top_gap < 0.85`.

---

## 2. Complex Tangent Plane Projection

Normalized sensor inputs $(s_x, s_y, r_{\text{net}})$ perturb the canonical seed $(c_x, c_y, \text{zoom})$ on $\partial\mathcal{M}$:

$$c_{x,\text{eff}} = c_x + \tanh(x_{\text{pri}}) \cdot \frac{0.45}{\text{zoom}}, \qquad c_{y,\text{eff}} = c_y + \tanh(s_y) \cdot \frac{0.45}{\text{zoom}}$$

$$\text{zoom}_{\text{eff}} = \text{zoom} \cdot \left(1 + 0.10 \sin(s_x + s_y + r_{\text{net}})\right)$$

---

## 3. 3-Point Multi-Scale Tripod & Boundary Correction (`arXiv:1810.11107`)

Across scales $z_f = \{0.60, 1.00, 1.60\}$ with weights $w_z = \{0.25, 0.50, 0.25\}$, escape fractions $u = \text{esc} / \text{max\_iter} \in [0, 1]$ are weighted by the Gaussian CDF boundary correction inverse:

$$\omega(u, h) = \text{clip}\!\left(\Phi\!\left(\frac{u}{h}\right) + \Phi\!\left(\frac{1-u}{h}\right) - 1,\; 0.45,\; 1.0\right), \qquad W(u, h) = \frac{1}{\omega(u, h)}$$

yielding bounded quadrant ratios $R_q \in [0, 1]$ and zero-centered quadrant weights $W_q = (R_q - 0.5) \cdot 6.0 \in [-3, +3]$.

---

## 4. 4-Tier Gaussian Choice, Cadence Bifurcation & Softmax ($T = 1.25$)

For each action tier $i \in \{0, 1, 2, 3\}$ with canonical centers $c = \{-0.85, +0.25, +1.15, +2.80\}$:

$$s_i^{(0)} = 2.5\,\hat{R}_i + 0.5\,(1 - \bar{u}) + 6.0 \exp\!\left(-2.2\,(r_{\text{tier}} - c_i)^2\right)$$

followed by Coupled Cadence Pitchfork Bifurcation (`werrApplyCadenceBifurcation`) and temperature-calibrated Softmax ($T = 1.25$).
