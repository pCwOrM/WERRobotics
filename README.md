# 🤖 WERRobotics — WERR v0.5.1 Zero-Memory System-1 Edge AI & Arduino Robotics Core

<p align="center">
  <a href="README.md"><img src="https://img.shields.io/badge/Language-English_%F0%9F%87%AC%F0%9F%87%A7-012169?style=for-the-badge" alt="English"></a>
  <a href="README_TR.md"><img src="https://img.shields.io/badge/Dil-T%C3%BCrk%C3%A7e_%F0%9F%87%B9%F0%9F%87%B7-e30a17?style=for-the-badge" alt="Türkçe"></a>
  <a href="https://github.com/pCwOrM/werr"><img src="https://img.shields.io/badge/WERR_Core-v0.5.1_Synced-00e5ff?style=for-the-badge" alt="WERR Core v0.5.1"></a>
  <a href="https://arxiv.org/abs/2609.25498"><img src="https://img.shields.io/badge/arXiv-2609.25498-b31b1b?style=for-the-badge" alt="arXiv"></a>
  <a href="https://doi.org/10.5281/zenodo.22939253"><img src="https://img.shields.io/badge/Zenodo-10.5281%2Fzenodo.22939253-blue?style=for-the-badge" alt="Zenodo"></a>
  <img src="https://img.shields.io/badge/Heap_Allocation-0_Bytes-00c853?style=for-the-badge" alt="Zero Heap">
</p>

<p align="center">
  <b>Real-Time Autonomous Reflex Decision Engine on Bare-Metal Microcontrollers (ATmega328P / Mega2560 / ESP32) Powered by Mandelbrot Wave Dynamics & Canonical Fractal Seeds</b><br>
  <i>"When the Wave meets Error, we Recurse (werr)." — "Not the fastest wins, but the one who stays stable on the boundary."</i>
</p>

> 🇹🇷 **Türkçe Dokümantasyon İçin**: Bu deponun eksiksiz Türkçe ana kılavuzuna **[README_TR.md](README_TR.md)** adresinden, Türkçe mimari/parametre analizine **[docs/WERR_EDGE_ARCHITECTURE_TR.md](docs/WERR_EDGE_ARCHITECTURE_TR.md)** ve Türkçe donanım pin rehberine **[docs/HARDWARE_PINOUT_AND_WIRING_TR.md](docs/HARDWARE_PINOUT_AND_WIRING_TR.md)** adresinden ulaşabilirsiniz.

---

## 🌟 Vision & Executive Summary

**`WERRobotics`** is the official **Physical Edge AI & Embedded Robotics** repository of the **WERR** ecosystem, integrating the **[pCwOrM/werr](https://github.com/pCwOrM/werr) (`v0.5.1`)** zero-memory System-1 fractal decision engine and its canonical Mandelbrot boundary seeds ($\partial\mathcal{M}$) directly into bare-metal **8-bit AVR (Arduino Uno / Nano / Mega 2560)** and **32-bit (ESP32 / STM32)** robotic hardware.

Conventional embedded robotics sketches rely on brittle binary thresholds (e.g., `if (distance > 15)`) and blind blocking delays (`delay(400)`), which suffer from nodal deadlocks in symmetric corners and noisy environments. Conversely, conventional TinyML neural networks struggle to fit weight tensors into the `2 KB SRAM` of an Arduino Uno (`ATmega328P`).

**WERR Edge AI ([`WerrCoreEdge.h`](src/WerrCoreEdge/WerrCoreEdge.h))** eliminates stored weight matrices entirely, synthesizing non-linear decision boundaries, 4-quadrant motor/servo kinematics, and deadlock-breaking pitchfork bifurcations on the fly from $z_{k+1} = z_k^2 + c$ with **`0 Bytes` of heap allocation** and **`< 128 Bytes` of stack memory**.

---

## ⚙️ Purpose-Built WERR Parameter Profiles (`WerrConfig`)

Every parameter of `pCwOrM/werr` (`v0.5.1`) has been audited and configured into four purpose-built profiles inside [`src/WerrCoreEdge/WerrCoreEdge.h`](src/WerrCoreEdge/WerrCoreEdge.h) tailored to the exact physical mission of each robotic platform:

| Parameter (`WerrConfig`) | `WERR_CFG_OBSTACLE_REFLEX` (`werr_engelden_kacan`) | `WERR_CFG_BIONIC_KINEMATICS` (`werr_kelebek_biyonik`) | `WERR_CFG_MARKET_ROUTER` (`werr_alisveris_robotu`) | `WERR_CFG_ESP32_CANONICAL` (ESP32 / STM32 / Server) | Rationale & Existential Alignment |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **`resolution`** (Grid) | **`6`** ($6\times 6=36$) | **`12`** ($12\times 12=144$) | **`6`** ($6\times 6=36$) | **`36`** ($36\times 36=1296$) | `<1.2 ms` high-frequency reflex loop with Tesla 3-6-9 harmonic symmetry ($36$ pixels) for wheeled collision avoidance; $144$ pixels for smooth bionic wing sweep angles. |
| **`max_iter`** (Depth) | **`18`** ($2 \times 9$) | **`36`** ($4 \times 9$) | **`18`** ($2 \times 9$) | **`36`** ($4 \times 9$) | Exact multiples of $9$ ($\mathbb{Z}/9\mathbb{Z}$ alignment). Canonical `36` iterations for biomimetic servo gliding; `18` iterations for fast obstacle reflex loops. |
| **`tripod`** (`{0.6, 1.0, 1.6}`) | **`true`** | **`true`** | **`true`** | **`true`** | 3-Point Multi-Scale sampling with weights `{0.25, 0.50, 0.25}` prevents single-scale fractal aliasing. |
| **Orthogonal State `[D, L, R]`** | **`[1, 1, 1]`** | **`[1, 1, 1]`** | **`[1, 1, 1]`** | **`[1, 1, 1]`** | Activates `DomainGate` tangent projection gain (`0.45/zoom`) and maximum 4-Tier Gaussian kernel sharpness (`2.2`). |
| **`bandwidth` / `cusp_cutoff`** | `0.12` / `0.90` | `0.12` / `0.90` | `0.12` / `0.90` | `0.12` / `0.90` | `arXiv:1810.11107` Gaussian CDF boundary truncation correction ($\omega \in [0.45, 1.0]$) and near-cusp threshold ($u \ge 0.90$). |
| **`cadence_lambda`** ($\lambda$) | **`0.22`** | **`0.10`** | **`0.25`** | **`0.10`** | Reinforced supercritical pitchfork force on wheeled robots to immediately break symmetric wall/junction deadlocks. |
| **`cadence_alpha`** ($\alpha$) | **`0.50`** | **`0.50`** | **`0.50`** | **`0.50`** | Canonical square-root pitchfork exponent ($|s_i - s_j|^{0.5}$). |
| **`cadence_beta`** ($\beta$) | **`0.28`** | **`0.15`** | **`0.30`** | **`0.15`** | Elevated fractal quadrant field ($W_{q,i} - W_{q,j}$) coupling for decisive left/right evasion symmetry breaking. |
| **`deadlock_threshold`** ($\tau$) | **`0.85`** | **`0.85`** | **`0.85`** | **`0.85`** | Triggers Cadence bifurcation whenever top-1 vs. top-2 candidate score gap is $< 0.85$. |
| **`temp_choice`** ($T$) | **`1.25`** | **`1.25`** | **`1.25`** | **`1.25`** | Canonical calibrated Softmax temperature validated across JevBench and the Master Gauntlet. |

---

## 🌱 Connected Canonical `pCwOrM/werr` v0.5.1 Seeds

| C++ Constant (`werr::`) | Seed Name | Real ($c_x$) | Imag ($c_y$) | Zoom | Threshold | Robotics Role & Firmware Binding |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- |
| `WERR_SEED_UNIVERSAL_CUSP` | `universal_cusp` | `-0.74364389` | `+0.13182590` | `50.0` | `0.50` | **Seahorse Valley Universal Cusp**: Acoustic pulse & inter-clap rhythm triage (`werr_kelebek_biyonik.ino`). |
| `WERR_SEED_IOT_SAFETY` | `iot_safety` | `-0.74500000` | `+0.11200000` | `85.0` | `0.50` | **IoT Life & Collision Safety**: Real-time proximity $d(t)$ & closing rate $\Delta d/\Delta t$ hazard triage (`werr_engelden_kacan.ino`). |
| `WERR_SEED_GAME_COMBAT` | `game_combat` | `-0.74450000` | `+0.12500000` | `65.0` | `0.50` | **Tactical Evasion & Reflex Vectoring**: 4-Tier Gaussian Risk Kernel routing between Cruise, Agile Left, Agile Right, and Emergency Pivot (`werr_engelden_kacan.ino`). |
| `WERR_SEED_SMART_ROUTER` | `smart_router` | `-0.10109636` | `+0.95628651` | `45.0` | `0.50` | **Balanced 4-Quadrant Router**: Biomimetic wing kinematics (`werr_kelebek_biyonik.ino`) & 3-sensor autonomous line/market routing (`werr_alisveris_robotu.ino`). |
| `WERR_SEED_SECURITY_GUARD` | `security_guard` | `-0.74364389` | `+0.13182590` | `120.0` | `0.50` | High-sensitivity boundary guard & hardware interlock protection. |
| `WERR_SEED_RISK_EVALUATOR` | `risk_evaluator` | `-0.75000000` | `+0.10000000` | `25.0` | `0.50` | Smooth gradient velocity and priority ranking. |
| `WERR_SEED_FINANCIAL_RISK` | `financial_risk` | `-0.74800000` | `+0.06500000` | `60.0` | `0.50` | Battery / current-draw circuit breaker protection. |
| `WERR_SEED_ECOMMERCE_FRAUD` | `ecommerce_fraud` | `-0.74950000` | `+0.08200000` | `70.0` | `0.50` | Sensor anomaly & ultrasonic false-echo outlier filtering. |

---

## 📂 Repository Structure & Bilingual Documentation Index

```text
WERRobotics/
├── README.md                                 # 🇬🇧 English Master Documentation
├── README_TR.md                              # 🇹🇷 Türkçe Ana Dokümantasyon
├── engeldenkacan.ino                         # Original baseline obstacle avoidance sketch in root
├── kelebek.ino                               # Original baseline bionic butterfly sketch in root
├── library.properties                        # Arduino IDE library metadata (WerrCoreEdge v0.5.1)
├── keywords.txt                              # Arduino IDE syntax coloring map
├── raw/                                      # [PHASE 0] Unmodified Baseline Robotics Sketches (@PrensesMTN)
│   ├── engeldenkacan/engeldenkacan.ino       # HC-SR04 + Dual DC Motor obstacle avoidance baseline
│   ├── kelebek/kelebek.ino                   # Acoustic double-clap + Dual Servo bionic butterfly baseline
│   ├── alisveris_roboti/alisveris_roboti.ino # 3-sensor + 16x2 LCD autonomous shopping robot baseline
│   ├── distance_counting/distance_counting.ino # NewPing + I2C LCD ultrasonic distance meter
│   └── robot_el_backend/main.py              # ESP32-CAM + MediaPipe WebSocket robotic hand backend
├── src/
│   └── WerrCoreEdge/
│       └── WerrCoreEdge.h                    # [PHASE 1] WERR v0.5.1 C/C++11 Zero-Memory Edge AI Kernel
├── firmware/                                 # [PHASE 2] WERR-Seed-Integrated Edge AI Arduino Firmware
│   ├── werr_engelden_kacan/
│   │   └── werr_engelden_kacan.ino           # iot_safety + game_combat + 4-Tier Gaussian & Cadence Robot
│   ├── werr_kelebek_biyonik/
│   │   └── werr_kelebek_biyonik.ino          # universal_cusp + smart_router + 4-Quadrant Bionic Butterfly
│   └── werr_alisveris_robotu/
│       └── werr_alisveris_robotu.ino         # smart_router + LCD Telemetry Autonomous Market Robot
├── sim/                                      # [PHASE 3] Hardware-in-the-Loop (HIL) Parity & Trajectory Suite
│   ├── verify_edge_parity.py                 # Python <-> C++ kernel parity & closed-loop simulator
│   └── hil_verification_report.json          # Sealed simulation & GAP-0331 verification output
└── docs/                                     # [PHASE 4] Bilingual Technical & Hardware Documentation
    ├── WERR_EDGE_ARCHITECTURE.md             # 🇬🇧 Mathematical Model, Parameter Audit & Architecture Guide
    ├── WERR_EDGE_ARCHITECTURE_TR.md          # 🇹🇷 Matematiksel Model, Parametre Analizi ve Mimari Kılavuzu
    ├── HARDWARE_PINOUT_AND_WIRING.md         # 🇬🇧 Full Hardware Pinout & Wiring Guide
    └── HARDWARE_PINOUT_AND_WIRING_TR.md      # 🇹🇷 Donanım Pin Bağlantıları ve Devre Rehberi
```

---

## 🎖️ Credits, Contributors & Academic Citation

- **WERR Theory & Core Architecture**: **Volkan Dağlı** ([@pCwOrM](https://github.com/pCwOrM)) — *ITouch Systems / answerr.me* (ORCID: [0009-0000-1587-8703](https://orcid.org/0009-0000-1587-8703))
- **Robotics Hardware & Embedded Sketches**: **Prenses Metin** ([@PrensesMTN](https://github.com/PrensesMTN))
- **WERR Research Team**: **Volkan Dağlı**, **Zerrin Dağlı** ([0000-0001-9490-6425](https://orcid.org/0000-0001-9490-6425)), **Dağhan Dağlı** ([0009-0003-2492-8313](https://orcid.org/0009-0003-2492-8313))

```bibtex
@article{dagli2026werr,
  title   = {Universal Fractal Natural Language Decision Map: Real-Time Edge Triage Across Heterogeneous Domains},
  author  = {Da{\u{g}}l{\i}, Volkan and Da{\u{g}}l{\i}, Zerrin and Da{\u{g}}l{\i}, Da{\u{g}}han},
  journal = {arXiv preprint arXiv:2609.25498 [cs.NE]},
  year    = {2026},
  doi     = {10.48550/arXiv.2609.25498},
  url     = {https://arxiv.org/abs/2609.25498}
}
```
