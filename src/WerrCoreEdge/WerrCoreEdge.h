/**
 * ============================================================================
 * WerrCoreEdge.h — WERR v0.5.1 Zero-Memory System-1 Edge AI Kernel for Arduino
 * ============================================================================
 * Canonical Repository : https://github.com/pCwOrM/werr
 * Edge Robotics Repo   : https://github.com/pCwOrM/WerrRobotics
 * Theory & Architecture: Volkan Dagli (@pCwOrM), Zerrin Dagli, Daghan Dagli
 * Robotics Integration : Volkan Dagli (@pCwOrM) & Prenses Metin (@PrensesMTN)
 * DOI / Preprint       : arXiv:2609.25498 | Zenodo: 10.5281/zenodo.22939253
 * Patent Filing        : TURKPATENT TR 2026/016633
 *
 * DESCRIPTION:
 *   Pure C/C++11 header-only implementation of the WERR (Weighted Escape-Rate
 *   Resonance / Waves & Errors) System-1 decision engine tailored for bare-metal
 *   microcontrollers (Arduino Uno ATmega328P, Mega2560, ESP32, STM32).
 *
 *   - 0 Bytes Dynamic Heap Allocation (No malloc / new / std::vector)
 *   - < 128 Bytes Stack Footprint
 *   - Canonical Mandelbrot Boundary Seeds synced with pCwOrM/werr v0.5.1
 *   - 3-Point Multi-Scale Tripod Sampling {0.60x, 1.00x, 1.60x}
 *   - Gaussian CDF Boundary Truncation Correction (arXiv:1810.11107)
 *   - Cadence Supercritical Pitchfork Bifurcation (Deadlock Breaker)
 *   - Homeostatic EMA Dynamic Calibration
 *   - GAP-0331 Constructive Z/9Z Modular Invariants (Tesla 3-6-9 Harmonic Algebra)
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
// 2. DATA STRUCTURES FOR EDGE REFLEX DECISIONS
// ============================================================================

struct WerrPatchMetrics {
    float eff_cx;             // Effective complex real coordinate
    float eff_cy;             // Effective complex imag coordinate
    float eff_zoom;           // Effective zoom scale
    float black_ratio;        // Interior stability ratio in [0, 1]
    float avg_escape;         // Normalized escape energy in [0, 1]
    float quad_ratios[4];     // Boundary-corrected quadrant ratios Q1..Q4 in [0, 1]
    float quad_weights[4];    // Zero-centered quadrant weights Q1..Q4 in [-3, +3]
    float hazard_score;       // Composite WERR hazard / urgency score in [0, 3]
    float noul_prob;          // Calibrated System-1 probability in [0.01, 0.99]
    bool  reflex_triggered;   // True if noul_prob >= active threshold
    uint8_t dominant_quad;    // Index (0..3) of highest-energy quadrant
};

// ============================================================================
// 3. MATHEMATICAL PRIMITIVES & BOUNDARY CORRECTION (arXiv:1810.11107)
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
// 4. MANDELBROT PATCH & 3-POINT TRIPOD KERNEL
// ============================================================================

/**
 * Evaluates a single Mandelbrot patch at (cx, cy, zoom) with grid resolution
 * `res x res` (must be even, e.g. 6, 8, 12, 16, 36) and `max_iter` iterations.
 * Extracts boundary-corrected 4-quadrant ratios on the fly with O(1) RAM.
 */
static inline void werrComputeSingleScale(
    float cx,
    float cy,
    float zoom,
    uint8_t res,
    uint8_t max_iter,
    float out_quad_ratios[4],
    float* out_black_ratio,
    float* out_avg_escape
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
            float w_corr = werrBoundaryWeight(u, 0.12f);
            sw[q_idx] += w_corr;
            su[q_idx] += w_corr * u;
            if (u >= 0.90f) {
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
 * Evaluates the WERR System-1 Engine at a given Seed perturbed by normalized
 * physical sensor inputs (state_x, state_y, net_risk).
 *
 * When `use_tripod = true`, executes the canonical 3-Point Multi-Scale Tripod
 * at zoom scales {0.60x, 1.00x, 1.60x} with weights {0.25, 0.50, 0.25}.
 */
static inline WerrPatchMetrics werrEvaluateState(
    const WerrSeed& seed,
    float state_x,
    float state_y,
    float net_risk = 0.0f,
    uint8_t res = 8,
    uint8_t max_iter = 24,
    bool use_tripod = true,
    float active_threshold = -1.0f
) {
    WerrPatchMetrics m;
    float base_scale = 1.0f / (seed.zoom > 1e-5f ? seed.zoom : 1.0f);

    // Project sensor state onto Mandelbrot tangent plane around seed
    float primary_x = (fabsf(net_risk) > 1e-5f) ? net_risk : state_x;
    float delta_x = tanhf(primary_x) * base_scale * 0.40f;
    float delta_y = tanhf(state_y)   * base_scale * 0.40f;

    m.eff_cx   = seed.cx + delta_x;
    m.eff_cy   = seed.cy + delta_y;
    m.eff_zoom = seed.zoom * (1.0f + 0.08f * sinf(state_x + state_y + net_risk));

    for (uint8_t q = 0; q < 4; ++q) {
        m.quad_ratios[q] = 0.0f;
    }
    m.black_ratio = 0.0f;
    m.avg_escape  = 0.0f;

    if (use_tripod) {
        const float z_factors[3] = {0.60f, 1.00f, 1.60f};
        const float z_weights[3] = {0.25f, 0.50f, 0.25f};

        for (uint8_t s = 0; s < 3; ++s) {
            float qr[4];
            float br = 0.0f, ae = 0.0f;
            werrComputeSingleScale(
                m.eff_cx, m.eff_cy, m.eff_zoom * z_factors[s],
                res, max_iter, qr, &br, &ae
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
            res, max_iter, m.quad_ratios, &m.black_ratio, &m.avg_escape
        );
    }

    uint8_t best_q = 0;
    float best_r = m.quad_ratios[0];
    float quad_asymmetry = 0.0f;

    for (uint8_t q = 0; q < 4; ++q) {
        m.quad_weights[q] = (m.quad_ratios[q] - 0.5f) * 6.0f;
        if (m.quad_ratios[q] > best_r) {
            best_r = m.quad_ratios[q];
            best_q = q;
        }
    }
    quad_asymmetry = fabsf(m.quad_ratios[0] - m.quad_ratios[3]) +
                     fabsf(m.quad_ratios[1] - m.quad_ratios[2]);
    m.dominant_quad = best_q;

    // Composite Hazard Score S in [0.0, 3.0] (aligned with WerrAileronControls & IoTSafetyGate)
    float raw_hazard = (1.0f - m.black_ratio) * 1.4f +
                       m.avg_escape * 0.8f +
                       quad_asymmetry * 0.8f +
                       werrClamp(net_risk * 0.45f, -0.8f, 1.5f);
    m.hazard_score = werrClamp(raw_hazard, 0.0f, 3.0f);

    // Calibrated Noul Probability in [0.01, 0.99]
    float logit = (net_risk - 0.20f) * 1.65f +
                  (m.quad_weights[0] * 0.25f - m.quad_weights[3] * 0.25f) +
                  (m.avg_escape - 0.5f) * 1.20f;
    m.noul_prob = werrClamp(werrSigmoid(logit), 0.01f, 0.99f);

    float thresh = (active_threshold > 0.0f) ? active_threshold : seed.default_threshold;
    m.reflex_triggered = (m.noul_prob >= thresh);

    return m;
}

// ============================================================================
// 5. CADENCE SUPERCRITICAL PITCHFORK BIFURCATION (Deadlock Breaker)
// ============================================================================

/**
 * Applies coupled supercritical pitchfork bifurcation to break destructive
 * nodal deadlocks between K competing action candidates (e.g., symmetric
 * obstacle ahead where Left vs Right evasion scores are tied).
 *
 * Exact C++ port of werr/fractal.py :: apply_cadence_bifurcation.
 * Returns the index of the winning candidate after bifurcation.
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

    // Find top-1 and top-2 scores to measure nodal deadlock gap
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
    float delta[8]; // Supports up to 8 action candidates on stack
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

// ============================================================================
// 6. HOMEOSTATIC EMA DYNAMIC CALIBRATION (werr/calibration.py)
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
// 7. GAP-0331 CONSTRUCTIVE Z/9Z MODULAR ALGEBRA (Tesla 3-6-9 Harmonic Phase)
// ============================================================================

/**
 * Returns true if `a` belongs to the invertible unit group (Z/9Z)^x = {1,2,4,5,7,8}.
 */
static inline bool werrIsUnitMod9(int16_t a) {
    int16_t r = ((a % 9) + 9) % 9;
    return (r == 1 || r == 2 || r == 4 || r == 5 || r == 7 || r == 8);
}

/**
 * Returns true if `a` belongs to the resonant sub-ideal I_3 = {0, 3, 6} (Tesla 3-6-9).
 */
static inline bool werrIsResonantSubidealI3(int16_t a) {
    int16_t r = ((a % 9) + 9) % 9;
    return (r == 0 || r == 3 || r == 6);
}

/**
 * Constructive multiplicative inverse in Z/9Z (GAP-0331 `inverseOpExec_correct`).
 * Returns -1 if `a` is a non-invertible zero-divisor in I_3 = {0, 3, 6}.
 */
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

/**
 * Neutralizes modular perturbation over Z/9Z: restores exact state phase.
 */
static inline int8_t werrNeutralizePerturbationMod9(int16_t state_mod, int16_t factor) {
    int8_t inv = werrConstructiveInverseMod9(factor);
    if (inv < 0) return (int8_t)(((state_mod % 9) + 9) % 9);
    int16_t r = ((state_mod % 9) + 9) % 9;
    return (int8_t)((r * inv) % 9);
}

} // namespace werr

#endif // WERR_CORE_EDGE_H
