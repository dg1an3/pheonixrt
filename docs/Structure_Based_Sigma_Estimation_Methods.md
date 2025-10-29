# Structure-Based Sigma Estimation for Brimstone

## Overview

This document describes the structure-based sigma estimation approach for adaptive multi-scale optimization in the Brimstone radiotherapy planning algorithm.

## Current Approach: Fixed Sigmas

**Implementation:** `RtModel/PlanOptimizer.cpp:32-36`

```cpp
const REAL DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25};
```

**Limitations:**
- Same sigmas for all patients, anatomies, prescriptions
- No adaptation to structure complexity or dose requirements
- Heuristic values based on general experience
- May be suboptimal for specific cases (e.g., highly complex H&N vs. simple prostate)

---

## Structure-Based Adaptive Approach

### Core Philosophy

**Key Insight:** Different treatment plans have different optimization complexity levels that should inform the multi-scale pyramid parameters.

**Factors Affecting Optimal Sigma:**

1. **Geometric Complexity** - Complex shapes need finer resolution
2. **Dose Gradients** - Sharp dose transitions require smaller scales
3. **Prescription Range** - Wide dose ranges increase optimization difficulty
4. **Structure Volume** - Larger structures can tolerate coarser initial scales

---

## Method 1: Geometric Complexity Estimation

### Concept
Complex anatomy shapes (concave, irregular) require finer spatial resolution compared to simple convex structures.

### Implementation
**Function:** `SigmaEstimator::EstimateFromGeometry()`

**Metrics Used:**

1. **Sphericity**
   ```
   Sphericity = (π^(1/3) × (6V)^(2/3)) / A

   where:
   - V = structure volume
   - A = surface area
   - Perfect sphere: sphericity = 1.0
   - Complex shape: sphericity → 0
   ```

2. **Surface-to-Volume Ratio**
   ```
   SV_ratio = Surface_Area / Volume

   Higher ratio → more complex boundary → finer sigma needed
   ```

### Mapping to Sigma

```
complexity_shape = 1.0 - sphericity
complexity_SV = SV_ratio / avg_SV_ratio

total_complexity = 0.6 × complexity_shape + 0.4 × complexity_SV

sigma_geometry = base_sigma × (1.0 - 0.7 × total_complexity)
```

**Expected Range:**
- Simple sphere (sphericity=1.0): sigma ≈ 0.5 (can use coarser)
- Complex irregular (sphericity=0.3): sigma ≈ 0.2 (needs finer)

### Example Results

| Structure | Sphericity | SV Ratio | Estimated Sigma | Reasoning |
|-----------|-----------|----------|-----------------|-----------|
| Prostate (PTV) | 0.85 | 0.8 | 0.45 | Simple, convex → coarser OK |
| Parotid | 0.65 | 1.5 | 0.35 | Moderate complexity |
| Spinal Cord | 0.40 | 3.2 | 0.22 | Long thin structure → fine needed |
| H&N PTV (concave) | 0.55 | 2.0 | 0.28 | Complex avoidance → finer |

---

## Method 2: Dose Gradient Estimation

### Concept
Regions with steep dose gradients (e.g., tumor boundary, OAR interfaces) require finer spatial sampling for accurate optimization.

### Implementation
**Function:** `SigmaEstimator::EstimateFromDoseGradient()`

**Algorithm:**
1. Extract dose distribution within structure region
2. Calculate spatial gradient at each voxel using central differences:
   ```
   ∇D = (∂D/∂x, ∂D/∂y, ∂D/∂z)
   |∇D| = √((∂D/∂x)² + (∂D/∂y)² + (∂D/∂z)²)
   ```
3. Compute average gradient magnitude across structure
4. Calculate dose heterogeneity (standard deviation)

### Mapping to Sigma

```
normalized_gradient = avg_gradient / 5.0 Gy/mm
normalized_heterogeneity = dose_stddev / 5.0 Gy

dose_complexity = 0.7 × normalized_gradient + 0.3 × normalized_heterogeneity

sigma_gradient = base_sigma / (1.0 + dose_complexity)
```

**Rationale:**
- High gradient → need fine resolution → small sigma
- Low gradient (uniform dose) → coarse resolution acceptable → large sigma

### Example Results

| Scenario | Avg Gradient (Gy/mm) | Dose StdDev (Gy) | Estimated Sigma | Reasoning |
|----------|---------------------|------------------|-----------------|-----------|
| Uniform PTV | 0.5 | 1.0 | 0.48 | Gentle gradients → coarser OK |
| PTV edge (steep) | 8.0 | 6.0 | 0.18 | Sharp boundary → fine needed |
| OAR with hotspot | 4.0 | 8.0 | 0.22 | High heterogeneity → finer |
| Normal tissue | 1.0 | 2.0 | 0.38 | Moderate gradients |

---

## Method 3: Prescription Range Estimation

### Concept
Wide prescription dose ranges indicate complex optimization objectives requiring more careful parameter tuning.

### Implementation
**Function:** `SigmaEstimator::EstimateFromPrescriptionRange()`

**Algorithm:**
1. Extract min/max prescribed doses from KLDivTerm
2. Calculate dose range: `range = max_dose - min_dose`
3. Map to sigma based on optimization difficulty

### Mapping to Sigma

```
normalized_range = dose_range / 50.0 Gy

sigma_prescription = base_sigma / (0.5 + 0.5 × normalized_range)
```

### Example Results

| Prescription | Min Dose (Gy) | Max Dose (Gy) | Range (Gy) | Estimated Sigma |
|--------------|---------------|---------------|------------|-----------------|
| Simple PTV | 45.0 | 50.0 | 5.0 | 0.48 |
| Complex PTV | 60.0 | 70.0 | 10.0 | 0.45 |
| OAR constraint | 0.0 | 30.0 | 30.0 | 0.36 |
| Wide range PTV | 50.0 | 80.0 | 30.0 | 0.36 |

**Interpretation:**
- Narrow range (tight constraints) → simpler optimization → coarser sigma OK
- Wide range (complex DVH objectives) → harder optimization → finer sigma helps

---

## Method 4: Volume-Based Estimation

### Concept
Larger structures can initially be optimized at coarser resolutions since small-scale details matter less for overall DVH matching.

### Implementation
**Function:** `SigmaEstimator::EstimateFromVolume()`

**Algorithm:**
1. Count voxels in structure region
2. Calculate volume: `V = voxel_count × voxel_spacing³`
3. Use log scale for mapping (volumes vary over orders of magnitude)

### Mapping to Sigma

```
log_volume = log₁₀(max(volume_cc, 1.0))

volume_factor = 0.5 + 0.25 × log_volume

sigma_volume = base_sigma × volume_factor
```

### Example Results

| Structure | Volume (cc) | log₁₀(V) | Volume Factor | Estimated Sigma |
|-----------|-------------|----------|---------------|-----------------|
| Spinal cord | 10 | 1.0 | 0.75 | 0.38 |
| Prostate PTV | 50 | 1.7 | 0.93 | 0.46 |
| Breast PTV | 500 | 2.7 | 1.18 | 0.59 |
| Lung | 2000 | 3.3 | 1.33 | 0.66 |

**Rationale:**
- Small structures: Details matter from the start → finer sigma
- Large structures: Can start coarse, refine later → coarser sigma OK

---

## Combined Estimation

### Weighted Average Approach

**Function:** `SigmaEstimator::CombineEstimates()`

```
sigma_combined = (w₁×σ_geometry + w₂×σ_gradient + w₃×σ_prescription + w₄×σ_volume)
                 / (w₁ + w₂ + w₃ + w₄)
```

**Default Weights:**
```cpp
WEIGHT_GEOMETRY = 1.0
WEIGHT_DOSE_GRADIENT = 2.0      // Most important!
WEIGHT_PRESCRIPTION = 1.5
WEIGHT_VOLUME = 0.8
```

**Rationale for Weights:**
1. **Dose Gradient (2.0)** - Highest weight because steep gradients absolutely require fine resolution
2. **Prescription (1.5)** - Second highest because complex objectives drive optimization difficulty
3. **Geometry (1.0)** - Baseline weight, shape complexity is important
4. **Volume (0.8)** - Lowest weight, volume is a weaker predictor

### Example: Combining Estimates

**Scenario:** Prostate PTV with adjacent rectum

| Method | Individual Sigma | Weight | Weighted Contribution |
|--------|------------------|--------|----------------------|
| Geometry (moderate) | 0.40 | 1.0 | 0.40 |
| Dose Gradient (high at rectum) | 0.25 | 2.0 | 0.50 |
| Prescription (standard) | 0.45 | 1.5 | 0.68 |
| Volume (medium) | 0.48 | 0.8 | 0.38 |
| **Combined** | **0.35** | **5.3** | **1.96 / 5.3 = 0.37** |

**Result:** Adaptive sigma = 0.37 (vs. default 0.25)

---

## Pyramid Sequence Generation

### From Base Sigma to Full Pyramid

**Function:** `SigmaEstimator::GeneratePyramidSequence()`

Given a base sigma (finest level), generate the full pyramid using a scale factor:

```
sigma[finest] = base_sigma
sigma[i] = sigma[i+1] × scale_factor

Default scale_factor = 2.5
```

### Example: Default vs. Adaptive

**Case:** Complex H&N plan → base_sigma = 0.30 (adaptive) vs. 0.25 (default)

| Level | Default Sigma | Adaptive Sigma | Scale Factor |
|-------|---------------|----------------|--------------|
| 4 (coarsest) | 8.0 | 11.7 | ← 2.5× |
| 3 | 3.2 | 4.7 | ← 2.5× |
| 2 | 1.3 | 1.9 | ← 2.6× |
| 1 | 0.5 | 0.75 | ← 2.5× |
| 0 (finest) | 0.25 | **0.30** | (base) |

**Impact:**
- Adaptive starts slightly coarser across all levels
- Maintains similar pyramid ratio structure
- Better matched to problem complexity

---

## Convergence-Based Adaptation

### Dynamic Adjustment During Optimization

**Function:** `SigmaEstimator::AdaptSigmaFromConvergence()`

**Idea:** Monitor optimization behavior and adjust sigma in real-time.

### Algorithm

```python
if cost_improvement < SLOW_THRESHOLD and gradient_norm < 0.01:
    # Stuck in local minimum
    sigma *= INCREASE_RATE  # Coarsen to escape (1.2×)

elif cost_improvement > FAST_THRESHOLD and gradient_norm > 0.1:
    # Converging rapidly
    sigma *= DECREASE_RATE  # Refine for precision (0.85×)
```

### Example Adaptive Trajectory

**Level 2 Optimization:**

| Iteration | Cost | Improvement | Gradient Norm | Action | New Sigma |
|-----------|------|-------------|---------------|--------|-----------|
| 0 | 1000 | - | 0.5 | Initialize | 1.30 |
| 10 | 850 | 0.15 | 0.45 | Converging fast → refine | 1.11 |
| 20 | 780 | 0.08 | 0.30 | Normal → maintain | 1.11 |
| 30 | 755 | 0.03 | 0.15 | Slowing → maintain | 1.11 |
| 40 | 748 | 0.009 | 0.08 | Slow + low gradient → escape | 1.33 |
| 50 | 730 | 0.024 | 0.12 | Escaped → maintain | 1.33 |
| 60 | 720 | 0.014 | 0.05 | Converged | Done |

**Benefit:** Automatically adjusts to problem difficulty without manual tuning.

---

## Expected Benefits

### 1. Improved Convergence Speed

**Hypothesis:** Better-matched sigma values reduce wasted iterations.

**Expected Improvements:**
- Simple cases: 5-10% faster (avoid over-refinement early)
- Complex cases: 10-25% faster (avoid under-refinement causing oscillation)

### 2. Better Plan Quality

**Hypothesis:** Adaptive sigma reduces risk of local minima and improves final DVH match.

**Expected Improvements:**
- Reduced objective function value (better KL-divergence match)
- Fewer DVH constraint violations
- More consistent results across diverse anatomies

### 3. Reduced Parameter Tuning

**Hypothesis:** Automatic estimation reduces need for manual registry tweaking.

**Expected Improvements:**
- Single "UseAdaptiveSigma" flag instead of 5 level-specific values
- Consistent behavior across institutions
- Less expert knowledge required

---

## Validation Strategy

### A. Unit Testing

1. **Sphericity Calculation**
   - Test: Perfect sphere → sphericity = 1.0
   - Test: Elongated cylinder → sphericity < 0.7

2. **Gradient Calculation**
   - Test: Uniform dose → gradient ≈ 0
   - Test: Step function → gradient >> 0

3. **Sigma Clamping**
   - Test: All outputs in [min_sigma, max_sigma] range

### B. Benchmark Cases

Run both default and adaptive sigma on standard test cases:

| Site | Complexity | Structures | Expected Outcome |
|------|-----------|------------|------------------|
| Prostate | Simple | 3-5 | Modest improvement (5-10%) |
| H&N | High | 15-25 | Significant improvement (15-25%) |
| Lung | Medium | 8-12 | Moderate improvement (10-15%) |
| Breast | Simple | 3-6 | Similar or slightly faster |

### C. Metrics to Track

1. **Convergence Speed**
   - Total iterations to convergence
   - Wall-clock time
   - Iterations per pyramid level

2. **Plan Quality**
   - Final objective function value
   - DVH constraint violations
   - Dose homogeneity indices

3. **Robustness**
   - Success rate (converged vs. failed)
   - Consistency across reruns
   - Sensitivity to initialization

---

## Implementation Roadmap

### Phase 1: Core Implementation (1-2 weeks)
- ✅ Create `SigmaEstimator` class
- ✅ Implement 4 estimation methods
- ✅ Write unit tests
- ⬜ Integrate into build system

### Phase 2: Integration (1 week)
- ⬜ Modify `PlanOptimizer::SetupPrescription()`
- ⬜ Add registry key "UseAdaptiveSigma"
- ⬜ Add logging/diagnostics
- ⬜ Test on benchmark cases

### Phase 3: Validation (2-3 weeks)
- ⬜ Run on diverse clinical case library
- ⬜ Compare default vs. adaptive performance
- ⬜ Statistical analysis of improvements
- ⬜ Identify failure modes

### Phase 4: Refinement (1-2 weeks)
- ⬜ Tune weights based on validation results
- ⬜ Add convergence-based adaptation
- ⬜ Implement safety fallbacks
- ⬜ Documentation and training materials

### Phase 5: Deployment
- ⬜ Enable by default (if validation successful)
- ⬜ Monitor clinical usage
- ⬜ Collect feedback
- ⬜ Iterate on edge cases

---

## Comparison to Alternative Methods

### vs. Fixed Sigmas (Current)
✅ **Adaptive:** Better for diverse cases
✅ **Adaptive:** Automatic tuning
❌ **Fixed:** Simpler, proven, zero overhead
❌ **Fixed:** Works well for standard cases

### vs. EM Algorithm
✅ **Structure-based:** Fast (no iterative search)
✅ **Structure-based:** No training data needed
❌ **EM:** More mathematically rigorous
❌ **EM:** Could achieve better optima

### vs. Q-Learning
✅ **Structure-based:** Works immediately (no training)
✅ **Structure-based:** Interpretable (human understanding)
❌ **Q-Learning:** Could learn complex patterns
❌ **Q-Learning:** Requires large training dataset

### Recommendation: Hybrid Approach

1. **Start:** Structure-based (this document)
2. **Refine:** Add EM-style adaptation per level
3. **Research:** Explore Q-learning offline for tuning weights

---

## Conclusion

Structure-based sigma estimation provides a practical, effective method to adapt Brimstone's multi-scale optimization to problem-specific characteristics. The approach:

✅ Requires no training data
✅ Works immediately on new cases
✅ Provides interpretable, physically-motivated estimates
✅ Can be gradually refined with convergence-based adaptation
✅ Serves as foundation for more sophisticated methods (EM, RL)

**Next Steps:**
1. Review implementation code (`SigmaEstimator.h/cpp`)
2. Run integration examples on test cases
3. Validate on clinical dataset
4. Deploy if validation successful

---

## References

- `RtModel/include/SigmaEstimator.h` - Class interface
- `RtModel/SigmaEstimator.cpp` - Implementation
- `RtModel/SigmaEstimator_Integration_Example.cpp` - Usage examples
- `RtModel/PlanOptimizer.cpp:284-329` - Current sigma usage
- `CLAUDE.md` - Brimstone algorithm overview
