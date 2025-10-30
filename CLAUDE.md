# Brimstone: Inverse Planning Algorithm for Radiotherapy

## Overview

Brimstone is a variational inverse planning algorithm for radiotherapy treatment planning developed by Derek G. Lane and protected under U.S. Patent 7,369,645. The algorithm is related to variational Bayes methods, though free energy is implicitly represented rather than explicitly calculated.

## Core Concept

Brimstone optimizes radiation beam intensities (beamlet weights) to deliver prescribed radiation doses to target volumes while minimizing dose to healthy tissue. It formulates treatment planning as an optimization problem where the goal is to match calculated dose-volume histograms (DVH) to clinically desired target distributions.

## Key Technical Components

### 1. Multi-Scale Pyramid Optimization

The algorithm employs a hierarchical pyramid approach with multiple resolution levels (default: 4 scales):

- **PlanPyramid**: Manages multiple resolution representations of the treatment plan
- Optimization proceeds from **coarsest to finest** resolution
- Each level refines the solution from the previous level through inverse filtering
- Scale-dependent sigma values control the resolution: {8.0, 3.2, 1.3, 0.5, 0.25}

**Benefits:**
- Faster convergence by solving simplified problems first
- Avoids local minima common in high-dimensional optimization
- Natural regularization through multi-scale representation

### 2. Dynamic Covariance Optimizer

Custom conjugate gradient optimizer (`DynamicCovarianceOptimizer`) with adaptive variance:

**Optimization Method:**
- **Polak-Ribiere conjugate gradient** for parameter updates
- **Brent line minimization** for 1D optimization along search directions
- Maximum 500 iterations per pyramid level
- Convergence tolerance: 1e-3 (configurable per level)

**Adaptive Variance Mechanism:**
- Dynamically adjusts search space weighting based on searched directions
- Uses **Gram-Schmidt orthogonalization** to maintain orthogonal basis
- Computes covariance matrix from search history
- Variance scales with search progress: `scale = 4^n / 4^iteration`
- Allows different parameters to adapt at different rates

### 3. Cost Function: Prescription

The `Prescription` class implements the objective function to be minimized:

**Components:**
- **KL-Divergence Terms** (`KLDivTerm`): Measure mismatch between calculated and target DVHs
- **VOI Terms** (`VOITerm`): Per-structure (Volume of Interest) optimization objectives
- **Gaussian Binning**: Histograms computed with adaptive Gaussian smoothing

**Mathematical Formulation:**
```
Cost = Σ KL(P_target || P_calc) over all structures

where:
- P_target: Target dose-volume probability distribution
- P_calc: Calculated dose-volume distribution from current beamlet weights
- KL divergence penalizes deviations from prescription
```

**Key Features:**
- Maintains dose-volume histograms with gradient information
- Supports multiple structures with individual weights
- Adaptive Gaussian bin variance (varMin to varMax range)
- Bin width: ~0.01 Gy (configurable)

### 4. Parameter Space Transformation

Uses sigmoid-based transformation to map between parameter spaces:

- **Linear space** (optimizer parameters) ↔ **Intensity space** (beamlet weights)
- Transformation ensures beamlet weights remain positive
- Scale parameter: 0.5 (configurable via "InputScale" registry)
- Sigmoid scale: 0.2 for smooth nonlinear mapping
- **Transform slope variance correction** accounts for Jacobian effects (disabled at coarsest level)

### 5. State Vector Management

The optimizer works with a unified state vector containing all beamlet weights:

- **StateVector**: Concatenated weights from all beams and beamlets
- Conversion utilities: `StateVectorToIntensityMap`, `IntensityMapToStateVector`
- Handles beam-specific beamlet counts and offsets
- Element inclusion flags prevent optimization of certain beamlets

## Algorithm Workflow

### Initialization
1. Create plan pyramid with multiple resolution levels
2. Set up prescription (cost function) for each pyramid level
3. Initialize state vector with small uniform weights (~0.001)
4. Configure adaptive variance ranges per level

### Optimization Loop (Coarse to Fine)
For each pyramid level (from MAX_SCALES-1 down to 0):

1. **Update histogram regions** for current resolution
2. **Inverse transform** initial vector to optimizer parameter space
3. **Run conjugate gradient optimization:**
   - Compute gradient at current point
   - Perform line minimization along conjugate direction
   - Update adaptive variance based on search directions
   - Check convergence criteria
   - Callback for progress monitoring
4. **Transform** result back to intensity space
5. **Inverse filter** to next finer level (if not at finest level)

### Convergence Criteria
- Change in function value < tolerance × (|f_old| + |f_new| + ε)
- Or gradient magnitude approaches zero
- Or maximum iterations reached

### Output
- Optimized beamlet intensity maps for each beam
- Updated dose distributions
- Final objective function value

## Implementation Details

### Histogram Calculation (`CalcSumSigmoid`)
- Accumulates dose contributions from all beamlets
- Applies sigmoid transformation during accumulation
- Tracks min/max variance fractions for adaptive Gaussian binning
- Computes gradients with respect to beamlet weights
- Supports 2D slice-based optimization for efficiency

### Gradient Computation
- Analytical gradients computed via chain rule through:
  1. Beamlet dose contributions
  2. Histogram bin populations
  3. KL-divergence formula
- Gradient information flows back through Gaussian convolution
- Enables efficient gradient-based optimization

### Registry Configuration
Key parameters controllable via Windows registry:

- `GBinSigma`: Gaussian bin smoothing (default: 0.2)
- `LevelSigma[i]`: Resolution scale per pyramid level
- `CGTolerance[i]`: Convergence tolerance per level
- `Tolerance[i]`: Line search tolerance per level
- `InputScale`: Parameter space transformation scale

## Advantages

1. **Mathematically principled**: Based on information theory (KL-divergence)
2. **Multi-scale robustness**: Avoids local minima through pyramid approach
3. **Adaptive optimization**: Dynamic covariance adapts search to problem structure
4. **Gradient-based**: Efficient compared to derivative-free methods
5. **Flexible prescriptions**: Supports arbitrary target DVH shapes via DVP matrices
6. **Implicit regularization**: Multi-scale approach naturally regularizes solutions

## Related Concepts

- **Variational Bayes**: Similar minimization of KL-divergence
- **Free Energy Minimization**: Connection to information-theoretic frameworks
- **Entropy Maximization**: Related to histogram matching objectives
- **Inverse Problems**: General framework for solving inverse planning

## Patent Information

**U.S. Patent 7,369,645**
Copyright (c) 2007-2021, Derek G. Lane (2nd Messenger Systems)
All rights reserved.

## Code Structure

```
Brimstone/
├── PlanOptimizer.{h,cpp}         # Multi-level optimization manager
├── ConjGradOptimizer.{h,cpp}     # Conjugate gradient with adaptive variance
├── Prescription.{h,cpp}          # Cost function implementation
├── PlanPyramid.{h,cpp}           # Multi-resolution plan representation
├── KLDivTerm.{h,cpp}             # KL-divergence objective terms
├── ObjectiveFunction.{h,cpp}     # Base cost function interface
└── BrimstoneDoc/View/App         # MFC application framework
```

## References

- pheonixrt README: Describes relationship to variational Bayes methods
- Related to free energy formulations in inverse planning
- See notebook_zoo for experimental analyses
