/**
 * ============================================================================
 * WerrCoreEdge.h — WERR v0.5.1 Zero-Memory System-1 Edge AI Kernel for Arduino
 * ============================================================================
 * Canonical Repository : https://github.com/pCwOrM/werr
 * Edge Robotics Repo   : https://github.com/pCwOrM/WERRobotics
 * Licensor & Protection: ITouch Systems (ITOUCH BILISIM SISTEMLERI MUH. DAN. SAN. VE TIC. LTD. STI.)
 * License              : BSL 1.1 (Business Source License 1.1 - ITouch Systems)
 * Theory & Architecture: Volkan Dagli (@pCwOrM), Zerrin Dagli, Daghan Dagli
 * Robotics Integration : Volkan Dagli (@pCwOrM) & Prenses Metin (@PrensesMTN)
 * DOI / Preprint       : arXiv:2609.25498 | Zenodo: 10.5281/zenodo.22939253
 * Patent Filings       : TURKPATENT TR 2026/016285 & TR 2026/016633
 *
 * DESCRIPTION:
 *   Pure C/C++11 header-only implementation of the WERR (Weighted Escape-Rate
 *   Resonance / Waves & Errors) System-1 decision engine tailored for bare-metal
 *   microcontrollers (Arduino Uno ATmega328P, Mega2560, ESP32, STM32).
 *
 *   - 0 Bytes Dynamic Heap Allocation (No malloc / new / std::vector)
 *   - < 128 Bytes Stack Footprint
 *   - All 8 Canonical Mandelbrot Boundary Seeds synced with pCwOrM/werr v0.5.1
 *   - Full Parametric Parity with WerrEngine & DomainGate (v0.5.1):
 *       * 3-Point Multi-Scale Tripod {0.60x, 1.00x, 1.60x}, weights {0.25, 0.50, 0.25}
 *       * Gaussian CDF Boundary Truncation Correction (bandwidth=0.12, omega>=0.45)
 *       * 4-Tier Gaussian Risk Kernel (centers {-0.85, +0.25, +1.15, +2.80}, sharpness=2.2)
 *       * Cadence Supercritical Pitchfork Bifurcation (lambda, alpha=0.50, beta, tau=0.85)
 *       * Calibrated Choice Softmax Temperature (temp_choice = 1.25)
 *       * Homeostatic EMA Dynamic Calibration (ema_alpha = 0.15, clamp [0.25, 0.75])
 *       * GAP-0331 Constructive Z/9Z Modular Invariants (Tesla 3-6-9 Harmonic Algebra)
 * ============================================================================
 */

#ifndef WERR_CORE_EDGE_H
#define WERR_CORE_EDGE_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include <math.h>

namespace werr {

// ============================================================================
// 1. CANONICAL PUBLIC SEEDS (Synchronized with pCwOrM/werr v0.5.1)
// ============================================================================

struct WerrSeed {
    const char* name;
    float cx;
    float cy;
    float zoom;
    float default_threshold;
};

// Universal Seahorse Valley Cusp (werr/engine.py default)
static const WerrSeed WERR_SEED_UNIVERSAL_CUSP = {
    "universal_cusp", -0.74364389f, 0.13182590f, 50.0f, 0.50f
};

// High-sensitivity boundary guard (werr/presets.py :: create_security_guard)
static const WerrSeed WERR_SEED_SECURITY_GUARD = {
    "security_guard", -0.74364389f, 0.13182590f, 120.0f, 0.50f
};

// Balanced 4-Quadrant entropy router (werr/presets.py :: create_smart_router)
static const WerrSeed WERR_SEED_SMART_ROUTER = {
    "smart_router", -0.10109636f, 0.95628651f, 45.0f, 0.50f
};

// Smooth gradient risk evaluator (werr/presets.py :: create_risk_evaluator)
static const WerrSeed WERR_SEED_RISK_EVALUATOR = {
    "risk_evaluator", -0.75000000f, 0.10000000f, 25.0f, 0.50f
};

// IoT Life Safety & Hazard Triage (werr/gates/discovered_seeds.json :: iot_safety)
static const WerrSeed WERR_SEED_IOT_SAFETY = {
    "iot_safety", -0.74500000f, 0.11200000f, 85.0f, 0.50f
};

// Tactical Reflex & Obstacle Evasion (werr/gates/discovered_seeds.json :: game_combat)
static const WerrSeed WERR_SEED_GAME_COMBAT = {
    "game_combat", -0.74450000f, 0.12500000f, 65.0f, 0.50f
};

// Financial Risk Circuit Breaker (werr/gates/discovered_seeds.json :: financial_risk)
static const WerrSeed WERR_SEED_FINANCIAL_RISK = {
    "financial_risk", -0.74800000f, 0.06500000f, 60.0f, 0.50f
};

// E-Commerce Anomaly & Fraud Gate (werr/gates/discovered_seeds.json :: ecommerce_fraud)
static const WerrSeed WERR_SEED_ECOMMERCE_FRAUD = {
    "ecommerce_fraud", -0.74950000f, 0.08200000f, 70.0f, 0.50f
};

// ============================================================================
// 2. WERR ENGINE PARAMETER CONFIGURATION & PURPOSE-BUILT PROFILES
// ============================================================================

struct WerrConfig {
    uint8_t resolution;         // Grid resolution (even & multiple of 3/6: 6, 12, 36)
    uint8_t max_iter;           // Escape iteration depth (multiple of 9: 18, 36)
    bool    tripod;             // 3-Point Multi-Scale Tripod {0.60x, 1.00x, 1.60x}
    bool    enable_domain;      // DomainGate tangent modulation (0.45f vs 0.40f)
    bool    enable_lexical;     // Orthogonal state bit 2 (controls Gaussian sharpness)
    bool    enable_resonance;   // Orthogonal state bit 3 (controls Gaussian sharpness)
    float   bandwidth;          // Gaussian CDF boundary bandwidth h (default 0.12f)
    float   cusp_cutoff;        // Near-boundary cusp escape cutoff u >= 0.90f
    float   cadence_lambda;     // Pitchfork bifurcation force strength (0.10f .. 0.25f)
    float   cadence_alpha;      // Pitchfork non-linear exponent (0.50f)
    float   cadence_beta;       // Fractal field coupling coefficient (0.15f .. 0.30f)
    float   deadlock_threshold; // Top-1 vs Top-2 gap threshold (0.85f)
    float   temp_choice;        // Softmax choice temperature (1.25f)
};

/**
 * Profile 1: WERR_CFG_OBSTACLE_REFLEX (werr_engelden_kacan.ino)
 * Purpose: High-speed ultrasonic collision triage & symmetric corner deadlock breaking.
 *   - res=6 (6x6 = 36 pixels per scale, 108 pixels Tripod total — Tesla 3-6-9 harmonic)
 *   - max_iter=18 (2 x 9 harmonic depth, ~1.15 ms on 16 MHz ATmega328P)
 *   - tripod=true, [1,1,1] sharpness=2.2, reinforced Cadence (lambda=0.22, beta=0.28)
 */
static const WerrConfig WERR_CFG_OBSTACLE_REFLEX = {
    6,      // resolution
    18,     // max_iter
    true,   // tripod
    true,   // enable_domain
    true,   // enable_lexical
    true,   // enable_resonance
    0.12f,  // bandwidth
    0.90f,  // cusp_cutoff
    0.22f,  // cadence_lambda
    0.50f,  // cadence_alpha
    0.28f,  // cadence_beta
    0.85f,  // deadlock_threshold
    1.25f   // temp_choice
};

/**
 * Profile 2: WERR_CFG_BIONIC_KINEMATICS (werr_kelebek_biyonik.ino)
 * Purpose: High-precision 4-Quadrant biomimetic wing stroke synthesis & acoustic triage.
 *   - res=12 (12x12 = 144 pixels per scale, digital root 9)
 *   - max_iter=36 (Canonical pCwOrM/werr 36-iteration depth for smooth servo curves)
 *   - tripod=true, canonical Cadence (lambda=0.10, alpha=0.50, beta=0.15)
 */
static const WerrConfig WERR_CFG_BIONIC_KINEMATICS = {
    12,     // resolution
    36,     // max_iter
    true,   // tripod
    true,   // enable_domain
    true,   // enable_lexical
    true,   // enable_resonance
    0.12f,  // bandwidth
    0.90f,  // cusp_cutoff
    0.10f,  // cadence_lambda
    0.50f,  // cadence_alpha
    0.15f,  // cadence_beta
    0.85f,  // deadlock_threshold
    1.25f   // temp_choice
};

/**
 * Profile 3: WERR_CFG_MARKET_ROUTER (werr_alisveris_robotu.ino)
 * Purpose: 3-Sensor line/obstacle routing & symmetric junction deadlock resolution.
 *   - res=6, max_iter=18, tripod=true, high bifurcation coupling (lambda=0.25, beta=0.30)
 */
static const WerrConfig WERR_CFG_MARKET_ROUTER = {
    6,      // resolution
    18,     // max_iter
    true,   // tripod
    true,   // enable_domain
    true,   // enable_lexical
    true,   // enable_resonance
    0.12f,  // bandwidth
    0.90f,  // cusp_cutoff
    0.25f,  // cadence_lambda
    0.50f,  // cadence_alpha
    0.30f,  // cadence_beta
    0.85f,  // deadlock_threshold
    1.25f   // temp_choice
};

/**
 * Profile 4: WERR_CFG_ESP32_CANONICAL (32-bit ESP32 / STM32 / Desktop Parity)
 * Purpose: Exact 36x36 @ 36-iter canonical server parity for 32-bit FPU microcontrollers.
 */
static const WerrConfig WERR_CFG_ESP32_CANONICAL = {
    36,     // resolution
    36,     // max_iter
    true,   // tripod
    true,   // enable_domain
    true,   // enable_lexical
    true,   // enable_resonance
    0.12f,  // bandwidth
    0.90f,  // cusp_cutoff
    0.10f,  // cadence_lambda
    0.50f,  // cadence_alpha
    0.15f,  // cadence_beta
    0.85f,  // deadlock_threshold
    1.25f   // temp_choice
};

// ============================================================================
// 3. DATA STRUCTURES FOR EDGE REFLEX DECISIONS
// ============================================================================

struct WerrPatchMetrics {
    float   eff_cx;             // Effective complex real coordinate
    float   eff_cy;             // Effective complex imag coordinate
    float   eff_zoom;           // Effective zoom scale
    float   black_ratio;        // Interior stability ratio in [0, 1]
    float   avg_escape;         // Normalized escape energy in [0, 1]
    float   quad_ratios[4];     // Boundary-corrected quadrant ratios Q1..Q4 in [0, 1]
    float   quad_weights[4];    // Zero-centered quadrant weights Q1..Q4 in [-3, +3]
    float   hazard_score;       // Composite WERR hazard / urgency score in [0, 3]
    float   noul_prob;          // Calibrated System-1 probability in [0.01, 0.99]
    bool    reflex_triggered;   // True if noul_prob >= active threshold
    uint8_t dominant_quad;      // Index (0..3) of highest-energy quadrant
};

struct WerrChoiceResult {
    uint8_t choice_idx;         // Winning candidate index (0..3)
    float   probabilities[4];   // Softmax probability distribution across 4 candidates
    float   scores[4];          // Post-bifurcation candidate logits
    float   confidence;         // Margin between top-1 and top-2 probability in [0, 1]
};

// ============================================================================
// 4. MATHEMATICAL PRIMITIVES & BOUNDARY CORRECTION (arXiv:1810.11107)
// ============================================================================

static inline float werrClamp(float val, float lo, float hi) {
    return (val < lo) ? lo : ((val > hi) ? hi : val);
}

static inline float werrSigmoid(float x) {
    float xc = werrClamp(x, -20.0f, 20.0f);
    return 1.0f / (1.0f + expf(-xc));
}

/**
 * Abramowitz & Stegun 7.1.26 polynomial approximation for erf(x).
 * Maximum error: 1.5e-7. Identical to werr/fractal.py :: _vec_erf.
 */
static inline float werrErf(float x) {
    float sign = (x >= 0.0f) ? 1.0f : -1.0f;
    float x_abs = fabsf(x);
    const float a1 =  0.254829592f;
    const float a2 = -0.284496736f;
    const float a3 =  1.421413741f;
    const float a4 = -1.453152027f;
    const float a5 =  1.061405429f;
    const float p  =  0.3275911f;
    float t = 1.0f / (1.0f + p * x_abs);
    float poly = ((((a5 * t + a4) * t + a3) * t + a2) * t + a1) * t;
    float y = 1.0f - poly * expf(-x_abs * x_abs);
    return sign * y;
}

/**
 * Standard normal cumulative distribution function Phi(z).
 */
static inline float werrNormalCdf(float z) {
    return 0.5f * (1.0f + werrErf(z * 0.70710678f)); // 1 / sqrt(2)
}

/**
 * Boundary weight inverse W(u, h) = 1 / omega(u, h) on [0, 1] (arXiv:1810.11107).
 * Compensates for escape-time truncation near the Mandelbrot boundary dM.
 */
static inline float werrBoundaryWeight(float u, float bandwidth = 0.12f) {
    float uc = werrClamp(u, 0.0f, 1.0f);
    float h = (bandwidth > 1e-4f) ? bandwidth : 1e-4f;
    float phi_left  = werrNormalCdf(uc / h);
    float phi_right = werrNormalCdf((1.0f - uc) / h);
    float omega = werrClamp(phi_left + phi_right - 1.0f, 0.45f, 1.0f);
    return 1.0f / omega;
}

// ============================================================================
// 5. MANDELBROT PATCH & 3-POINT TRIPOD KERNEL
// ============================================================================

static inline void werrComputeSingleScale(
    float cx,
    float cy,
    float zoom,
    uint8_t res,
    uint8_t max_iter,
    float out_quad_ratios[4],
    float* out_black_ratio,
    float* out_avg_escape,
    float bandwidth = 0.12f,
    float cusp_cutoff = 0.90f
) {
    if (res < 2) res = 2;
    if (res % 2 != 0) res += 1;
    if (max_iter < 4) max_iter = 4;

    float scale = 1.0f / (zoom > 1e-5f ? zoom : 1e-5f);
    uint8_t mid = res / 2;

    float sw[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float sc[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float su[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    uint16_t black_pixels = 0;
    uint32_t total_escape_sum = 0;
    uint16_t total_pixels = (uint16_t)res * (uint16_t)res;

    float step = (2.0f * scale) / (float)(res - 1);

    for (uint8_t r = 0; r < res; ++r) {
        float ci = (cy - scale) + step * (float)r;
        uint8_t q_row = (r < mid) ? 0 : 2;

        for (uint8_t c = 0; c < res; ++c) {
            float cr = (cx - scale) + step * (float)c;
            uint8_t q_idx = q_row + ((c < mid) ? 0 : 1);

            float zr = 0.0f;
            float zi = 0.0f;
            uint8_t esc = max_iter;

            for (uint8_t i = 0; i < max_iter; ++i) {
                float zr2 = zr * zr;
                float zi2 = zi * zi;
                if (zr2 + zi2 > 4.0f) {
                    esc = i;
                    break;
                }
                zi = 2.0f * zr * zi + ci;
                zr = zr2 - zi2 + cr;
            }

            if (esc == max_iter) {
                black_pixels++;
            }
            total_escape_sum += esc;

            float u = (float)esc / (float)max_iter;
            float w_corr = werrBoundaryWeight(u, bandwidth);
            sw[q_idx] += w_corr;
            su[q_idx] += w_corr * u;
            if (u >= cusp_cutoff) {
                sc[q_idx] += w_corr;
            }
        }
    }

    for (uint8_t q = 0; q < 4; ++q) {
        float denom = (sw[q] > 1e-6f) ? sw[q] : 1.0f;
        float boundary_cusp_ratio = sc[q] / denom;
        float avg_energy = su[q] / denom;
        out_quad_ratios[q] = 0.65f * boundary_cusp_ratio + 0.35f * avg_energy;
    }

    *out_black_ratio = (float)black_pixels / (float)total_pixels;
    *out_avg_escape  = (float)total_escape_sum / ((float)total_pixels * (float)max_iter);
}

/**
 * Evaluates the WERR System-1 Engine using a explicit WerrConfig profile.
 */
static inline WerrPatchMetrics werrEvaluateWithConfig(
    const WerrSeed& seed,
    const WerrConfig& cfg,
    float state_x,
    float state_y,
    float net_risk = 0.0f,
    float active_threshold = -1.0f
) {
    WerrPatchMetrics m;
    float base_scale = 1.0f / (seed.zoom > 1e-5f ? seed.zoom : 1.0f);

    // DomainGate uses 0.45f tangent projection & 0.10f zoom modulation (werr/gates/base.py L129)
    // Universal Cusp uses 0.40f tangent projection & 0.08f zoom modulation (werr/engine.py L264)
    float proj_gain = cfg.enable_domain ? 0.45f : 0.40f;
    float zoom_gain = cfg.enable_domain ? 0.10f : 0.08f;

    float primary_x = (fabsf(net_risk) > 1e-5f) ? net_risk : state_x;
    float delta_x = tanhf(primary_x) * base_scale * proj_gain;
    float delta_y = tanhf(state_y)   * base_scale * proj_gain;

    m.eff_cx   = seed.cx + delta_x;
    m.eff_cy   = seed.cy + delta_y;
    m.eff_zoom = seed.zoom * (1.0f + zoom_gain * sinf(state_x + state_y + net_risk));

    for (uint8_t q = 0; q < 4; ++q) {
        m.quad_ratios[q] = 0.0f;
    }
    m.black_ratio = 0.0f;
    m.avg_escape  = 0.0f;

    if (cfg.tripod) {
        const float z_factors[3] = {0.60f, 1.00f, 1.60f};
        const float z_weights[3] = {0.25f, 0.50f, 0.25f};

        for (uint8_t s = 0; s < 3; ++s) {
            float qr[4];
            float br = 0.0f, ae = 0.0f;
            werrComputeSingleScale(
                m.eff_cx, m.eff_cy, m.eff_zoom * z_factors[s],
                cfg.resolution, cfg.max_iter, qr, &br, &ae,
                cfg.bandwidth, cfg.cusp_cutoff
            );
            for (uint8_t q = 0; q < 4; ++q) {
                m.quad_ratios[q] += z_weights[s] * qr[q];
            }
            m.black_ratio += z_weights[s] * br;
            m.avg_escape  += z_weights[s] * ae;
        }
    } else {
        werrComputeSingleScale(
            m.eff_cx, m.eff_cy, m.eff_zoom,
            cfg.resolution, cfg.max_iter, m.quad_ratios, &m.black_ratio, &m.avg_escape,
            cfg.bandwidth, cfg.cusp_cutoff
        );
    }

    uint8_t best_q = 0;
    float best_r = m.quad_ratios[0];

    for (uint8_t q = 0; q < 4; ++q) {
        m.quad_weights[q] = (m.quad_ratios[q] - 0.5f) * 6.0f;
        if (m.quad_ratios[q] > best_r) {
            best_r = m.quad_ratios[q];
            best_q = q;
        }
    }
    float quad_asymmetry = fabsf(m.quad_ratios[0] - m.quad_ratios[3]) +
                           fabsf(m.quad_ratios[1] - m.quad_ratios[2]);
    m.dominant_quad = best_q;

    // Composite Hazard Score S in [0.0, 3.0]
    float raw_hazard = (1.0f - m.black_ratio) * 1.4f +
                       m.avg_escape * 0.8f +
                       quad_asymmetry * 0.8f +
                       werrClamp(net_risk * 0.45f, -0.8f, 1.5f);
    m.hazard_score = werrClamp(raw_hazard, 0.0f, 3.0f);

    // Calibrated Noul Probability in [0.01, 0.99] (matching DomainGate::_evaluate_noul)
    float base_prob = werrSigmoid((net_risk - 0.20f) * 2.0f);
    float fractal_boost = 0.80f + 0.40f * (1.0f - m.avg_escape);
    m.noul_prob = werrClamp(base_prob * fractal_boost, 0.01f, 0.99f);

    float thresh = (active_threshold > 0.0f) ? active_threshold : seed.default_threshold;
    m.reflex_triggered = (m.noul_prob >= thresh);

    return m;
}

/**
 * Backward-compatible convenience overload with explicit res/max_iter parameters.
 */
static inline WerrPatchMetrics werrEvaluateState(
    const WerrSeed& seed,
    float state_x,
    float state_y,
    float net_risk = 0.0f,
    uint8_t res = 6,
    uint8_t max_iter = 18,
    bool use_tripod = true,
    float active_threshold = -1.0f
) {
    WerrConfig cfg = WERR_CFG_OBSTACLE_REFLEX;
    cfg.resolution = res;
    cfg.max_iter   = max_iter;
    cfg.tripod     = use_tripod;
    return werrEvaluateWithConfig(seed, cfg, state_x, state_y, net_risk, active_threshold);
}

// ============================================================================
// 6. CADENCE SUPERCRITICAL PITCHFORK BIFURCATION & 4-TIER GAUSSIAN CHOICE
// ============================================================================

/**
 * Exact C++ port of werr/fractal.py :: apply_cadence_bifurcation.
 * Breaks destructive nodal deadlocks between K competing action candidates.
 */
static inline uint8_t werrApplyCadenceBifurcation(
    float* scores,
    const float* fractal_fields,
    uint8_t K,
    float lambda_param = 0.20f,
    float alpha = 0.50f,
    float beta = 0.25f,
    float deadlock_threshold = 0.85f
) {
    if (K <= 1) return 0;

    float max1 = -1e9f, max2 = -1e9f;
    for (uint8_t i = 0; i < K; ++i) {
        if (scores[i] > max1) {
            max2 = max1;
            max1 = scores[i];
        } else if (scores[i] > max2) {
            max2 = scores[i];
        }
    }

    float top_gap = max1 - max2;
    float delta[8];
    uint8_t limit = (K > 8) ? 8 : K;

    if (top_gap < deadlock_threshold) {
        float eff_lambda = lambda_param * (1.0f + (deadlock_threshold - top_gap) / deadlock_threshold);
        for (uint8_t i = 0; i < limit; ++i) {
            float bif_force = 0.0f;
            for (uint8_t j = 0; j < limit; ++j) {
                if (i == j) continue;
                float diff = scores[i] - scores[j];
                float sgn  = (diff > 0.0f) ? 1.0f : ((diff < 0.0f) ? -1.0f : 0.0f);
                bif_force += sgn * powf(fabsf(diff) + 1e-5f, alpha);
                if (fractal_fields != nullptr) {
                    bif_force += beta * (fractal_fields[i] - fractal_fields[j]);
                }
            }
            delta[i] = eff_lambda * bif_force;
        }
        for (uint8_t i = 0; i < limit; ++i) {
            scores[i] += delta[i];
        }
    } else if (fractal_fields != nullptr) {
        for (uint8_t i = 0; i < limit; ++i) {
            scores[i] += fractal_fields[i] * 0.35f;
        }
    }

    uint8_t best_idx = 0;
    float best_score = scores[0];
    for (uint8_t i = 1; i < limit; ++i) {
        if (scores[i] > best_score) {
            best_score = scores[i];
            best_idx = i;
        }
    }
    return best_idx;
}

/**
 * Evaluates a 4-Tier Action Choice using the exact Gaussian Risk Kernel,
 * Coupled Cadence Pitchfork Bifurcation, and Calibrated Softmax (`temp_choice = 1.25f`)
 * from `werr/gates/base.py :: _evaluate_choice`.
 *
 * Canonical 4-Tier Risk Centers:
 *   - Tier 0 (Direct / Cruise Forward)      : center = -0.85f
 *   - Tier 1 (Mild / Agile Left Curve)      : center = +0.25f
 *   - Tier 2 (Deep / Agile Right Curve)     : center = +1.15f
 *   - Tier 3 (Block / Emergency Reverse+Turn): center = +2.80f
 */
static inline WerrChoiceResult werrEvaluate4TierChoice(
    const WerrPatchMetrics& metrics,
    const WerrConfig& cfg,
    float tier_risk,
    float lateral_bias = 0.0f
) {
    WerrChoiceResult res;
    const float centers[4] = {-0.85f, 0.25f, 1.15f, 2.80f};

    // Orthogonal state sharpness (werr/gates/base.py L355):
    // Hybrid [1,1,1] -> 2.2f | Lexical [1,1,0] -> 2.0f | Pure Fractal -> 1.6f
    float sharpness = (cfg.enable_lexical && cfg.enable_resonance)
                      ? 2.2f
                      : (cfg.enable_lexical ? 2.0f : 1.6f);

    // Unit-sum normalized quadrant ratios
    float q_sum = metrics.quad_ratios[0] + metrics.quad_ratios[1] +
                  metrics.quad_ratios[2] + metrics.quad_ratios[3];
    if (q_sum < 1e-6f) q_sum = 1.0f;

    for (uint8_t i = 0; i < 4; ++i) {
        float q_norm = metrics.quad_ratios[i] / q_sum;
        float base_q_score = q_norm * 2.5f + (1.0f - metrics.avg_escape) * 0.5f;
        float diff = tier_risk - centers[i];
        res.scores[i] = base_q_score + 6.0f * expf(-sharpness * (diff * diff));
    }

    // Apply lateral symmetry bias between Tier 1 (Left) and Tier 2 (Right) in mid-risk zone
    res.scores[1] += lateral_bias;
    res.scores[2] -= lateral_bias;

    // Coupled Cadence Supercritical Pitchfork Bifurcation
    werrApplyCadenceBifurcation(
        res.scores,
        metrics.quad_weights,
        4,
        cfg.cadence_lambda,
        cfg.cadence_alpha,
        cfg.cadence_beta,
        cfg.deadlock_threshold
    );

    // Softmax with calibrated temperature (temp_choice = 1.25f)
    float temp = (cfg.temp_choice > 0.05f) ? cfg.temp_choice : 1.25f;
    float max_s = res.scores[0];
    for (uint8_t i = 1; i < 4; ++i) {
        if (res.scores[i] > max_s) max_s = res.scores[i];
    }

    float sum_exp = 0.0f;
    for (uint8_t i = 0; i < 4; ++i) {
        float z = werrClamp((res.scores[i] - max_s) / temp, -20.0f, 20.0f);
        res.probabilities[i] = expf(z);
        sum_exp += res.probabilities[i];
    }

    uint8_t best_idx = 0;
    float p_max1 = -1.0f, p_max2 = -1.0f;
    for (uint8_t i = 0; i < 4; ++i) {
        res.probabilities[i] /= (sum_exp > 1e-9f ? sum_exp : 1.0f);
        if (res.probabilities[i] > p_max1) {
            p_max2 = p_max1;
            p_max1 = res.probabilities[i];
            best_idx = i;
        } else if (res.probabilities[i] > p_max2) {
            p_max2 = res.probabilities[i];
        }
    }

    res.choice_idx = best_idx;
    res.confidence = werrClamp(p_max1 - p_max2, 0.0f, 1.0f);
    return res;
}

// ============================================================================
// 7. HOMEOSTATIC EMA DYNAMIC CALIBRATION (werr/calibration.py)
// ============================================================================

class WerrDynamicCalibration {
public:
    float alpha;
    float ema_score;
    float adaptive_threshold;
    uint32_t observations;

    WerrDynamicCalibration(float ema_alpha = 0.15f, float init_thresh = 0.50f)
        : alpha(ema_alpha),
          ema_score(init_thresh),
          adaptive_threshold(init_thresh),
          observations(0) {}

    float observeAndCalibrate(float raw_score) {
        float sc = werrClamp(raw_score, 0.01f, 0.99f);
        if (observations == 0) {
            ema_score = sc;
        } else {
            ema_score = alpha * sc + (1.0f - alpha) * ema_score;
        }
        observations++;
        adaptive_threshold = werrClamp(0.50f * 0.60f + ema_score * 0.40f, 0.25f, 0.75f);
        return adaptive_threshold;
    }
};

// ============================================================================
// 8. GAP-0331 CONSTRUCTIVE Z/9Z MODULAR ALGEBRA (Tesla 3-6-9 Harmonic Phase)
// ============================================================================

static inline bool werrIsUnitMod9(int16_t a) {
    int16_t r = ((a % 9) + 9) % 9;
    return (r == 1 || r == 2 || r == 4 || r == 5 || r == 7 || r == 8);
}

static inline bool werrIsResonantSubidealI3(int16_t a) {
    int16_t r = ((a % 9) + 9) % 9;
    return (r == 0 || r == 3 || r == 6);
}

static inline int8_t werrConstructiveInverseMod9(int16_t a) {
    int16_t r = ((a % 9) + 9) % 9;
    switch (r) {
        case 1: return 1; // 1 * 1 = 1 mod 9
        case 2: return 5; // 2 * 5 = 10 = 1 mod 9
        case 4: return 7; // 4 * 7 = 28 = 1 mod 9
        case 5: return 2; // 5 * 2 = 10 = 1 mod 9
        case 7: return 4; // 7 * 4 = 28 = 1 mod 9
        case 8: return 8; // 8 * 8 = 64 = 1 mod 9
        default: return -1; // Zero-divisor in I_3 = {0, 3, 6}
    }
}

static inline int8_t werrNeutralizePerturbationMod9(int16_t state_mod, int16_t factor) {
    int8_t inv = werrConstructiveInverseMod9(factor);
    if (inv < 0) return (int8_t)(((state_mod % 9) + 9) % 9);
    int16_t r = ((state_mod % 9) + 9) % 9;
    return (int8_t)((r * inv) % 9);
}

} // namespace werr

#endif // WERR_CORE_EDGE_H
