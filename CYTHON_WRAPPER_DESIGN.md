# Brimstone Cython Wrapper Design

## Overview

This document outlines the design for a Cython wrapper around the Brimstone inverse planning algorithm, enabling Python-based radiotherapy treatment planning with modern visualization tools like pymedphys.

## Goals

1. **Python API**: Provide a Pythonic interface to Brimstone optimization
2. **Intermediate Results**: Enable access to optimization state during runtime
3. **Visualization**: Integrate with pymedphys, matplotlib, and other Python tools
4. **Performance**: Minimize overhead while maintaining C++ performance
5. **Extensibility**: Allow custom callbacks and objective functions from Python

## Architecture

### Core Components to Wrap

#### 1. Data Structures
```python
# Core planning objects
class Plan:
    """Treatment plan containing beams and dose calculations"""
    series: Series                    # CT/imaging data
    beams: List[Beam]                 # Treatment beams
    dose: np.ndarray                  # Computed dose distribution
    structures: List[Structure]       # Anatomical structures

    def add_beam(self, gantry_angle: float, isocenter: Tuple[float, float, float])
    def get_dose(self) -> np.ndarray
    def update_histograms(self)

class Beam:
    """Single treatment beam"""
    gantry_angle: float
    isocenter: np.ndarray
    beamlets: np.ndarray              # Beamlet dose distributions
    intensity_map: np.ndarray         # Beamlet weights

    def get_dose(self) -> np.ndarray

class Series:
    """CT/imaging data"""
    ct_volume: np.ndarray
    mass_density: np.ndarray
    structures: List[Structure]

    @staticmethod
    def from_dicom(path: str) -> Series

class Structure:
    """Anatomical structure (VOI)"""
    name: str
    mask: np.ndarray                  # Binary mask
    histogram: Optional[np.ndarray]    # DVH

    def get_dvh(self) -> Tuple[np.ndarray, np.ndarray]
```

#### 2. Optimization Components
```python
class PlanOptimizer:
    """Multi-scale optimizer for treatment planning"""
    plan: Plan
    prescription: Prescription

    def __init__(self, plan: Plan)

    def optimize(
        self,
        initial_weights: Optional[np.ndarray] = None,
        callback: Optional[Callable] = None,
        max_iterations: int = 500,
        tolerance: float = 1e-3
    ) -> OptimizationResult

    def add_objective(self, structure: Structure, objective: ObjectiveTerm)

class Prescription:
    """Cost function for optimization"""
    objectives: List[ObjectiveTerm]

    def add_kl_divergence_term(
        self,
        structure: Structure,
        target_dvh: np.ndarray,
        weight: float = 1.0
    )

    def evaluate(
        self,
        beamlet_weights: np.ndarray
    ) -> Tuple[float, np.ndarray]  # cost, gradient

class ObjectiveTerm:
    """Base class for optimization objectives"""
    structure: Structure
    weight: float

    def evaluate(self, dose: np.ndarray) -> Tuple[float, np.ndarray]

class KLDivergenceTerm(ObjectiveTerm):
    """KL-divergence DVH matching objective"""
    target_dvh: np.ndarray

    def set_interval(
        self,
        dose_min: float,
        dose_max: float,
        volume_fraction: float
    )
```

#### 3. Callback and Monitoring
```python
class OptimizationCallback:
    """Base class for optimization callbacks"""

    def on_iteration(
        self,
        iteration: int,
        level: int,
        cost: float,
        gradient_norm: float,
        beamlet_weights: np.ndarray,
        dose: np.ndarray
    ) -> bool:  # Return False to terminate
        pass

class VisualizationCallback(OptimizationCallback):
    """Callback for live visualization with pymedphys"""

    def __init__(self, update_interval: int = 10):
        self.update_interval = update_interval
        self.history = {
            'iteration': [],
            'cost': [],
            'gradient_norm': [],
            'doses': [],
            'dvhs': {}
        }

    def on_iteration(self, **kwargs) -> bool:
        # Store history
        self.history['iteration'].append(kwargs['iteration'])
        self.history['cost'].append(kwargs['cost'])
        # ... update visualizations
        return True  # Continue optimization

    def plot_convergence(self):
        """Plot cost vs iteration"""
        pass

    def plot_dose_slice(self, slice_idx: int):
        """Plot dose distribution"""
        pass

    def plot_dvh(self, structures: List[Structure]):
        """Plot dose-volume histograms"""
        pass

class OptimizationResult:
    """Result of optimization"""
    success: bool
    message: str
    final_cost: float
    iterations: int
    beamlet_weights: np.ndarray
    dose: np.ndarray
    convergence_history: Dict
```

### Cython Implementation Structure

```
python/
├── setup.py                      # Build configuration
├── pyproject.toml               # Modern Python packaging
├── pybrimstone/
│   ├── __init__.py
│   ├── core.pyx                 # Core Cython wrapper
│   ├── core.pxd                 # Cython declarations
│   ├── callbacks.py             # Python callback classes
│   ├── visualization.py         # Visualization utilities
│   ├── io.py                    # DICOM/data I/O
│   └── examples/
│       ├── basic_optimization.py
│       ├── live_visualization.py
│       └── pymedphys_integration.py
└── cpp_headers/
    ├── plan_wrapper.h           # C++ wrapper headers
    └── optimizer_wrapper.h      # Bridge between C++ and Python
```

## Implementation Phases

### Phase 1: Core Data Structures (Weeks 1-2)
- [ ] Wrap Plan, Beam, Series, Structure classes
- [ ] Implement ITK volume ↔ NumPy array conversion
- [ ] Basic I/O for loading plans and CT data
- [ ] Unit tests for data structure wrapping

**Key Challenges:**
- ITK smart pointers and memory management
- Numpy array data ownership and lifetime
- Windows MFC dependencies (CTypedPtrMap, CString, etc.)

### Phase 2: Optimization Core (Weeks 3-4)
- [ ] Wrap PlanOptimizer, Prescription, KLDivergenceTerm
- [ ] Implement callback mechanism (C++ → Python)
- [ ] Wrap ConjGradOptimizer and PlanPyramid
- [ ] State vector ↔ NumPy array conversion
- [ ] Unit tests for optimization components

**Key Challenges:**
- Callback function pointer management
- Gradient array passing and memory safety
- Multi-scale pyramid state management

### Phase 3: Callbacks & Monitoring (Week 5)
- [ ] Implement Python callback infrastructure
- [ ] Create VisualizationCallback class
- [ ] Add convergence tracking and history
- [ ] Real-time dose and DVH updates
- [ ] Integration tests with callbacks

### Phase 4: Visualization & Integration (Week 6)
- [ ] pymedphys integration examples
- [ ] DVH plotting with matplotlib
- [ ] Dose overlay on CT images
- [ ] Interactive Jupyter notebook examples
- [ ] Performance benchmarking

### Phase 5: Advanced Features (Weeks 7-8)
- [ ] Custom objective functions from Python
- [ ] Advanced visualization dashboards
- [ ] Parallel plan evaluation
- [ ] Export to DICOM RT format
- [ ] Documentation and tutorials

## Technical Considerations

### Memory Management
- Use `unique_ptr` for C++ object ownership transfer
- Wrap ITK SmartPointers carefully in Cython
- NumPy arrays: use memoryviews for efficiency
- RAII wrappers for callback context

### Performance
- Minimize Python ↔ C++ boundary crossings
- Use memoryviews instead of Python lists for arrays
- Batch operations when possible
- Profile callback overhead (target < 1% of total time)

### Platform Support
- **Windows**: Primary development platform (Visual Studio)
- **Linux**: Secondary target
- **macOS**: Future consideration
- Handle MFC dependencies (consider platform abstraction)

### Dependencies
```python
# Required Python packages
dependencies = [
    "numpy>=1.20",
    "cython>=0.29",
    "itk>=5.2",           # ITK Python wrappers
    "pydicom>=2.3",       # DICOM I/O
    "matplotlib>=3.5",     # Basic plotting
    "pymedphys>=0.39",    # Radiotherapy visualization (optional)
    "ipywidgets>=8.0",    # Jupyter integration (optional)
]
```

## Example Usage

### Basic Optimization
```python
import pybrimstone as pb
import numpy as np

# Load patient data
series = pb.Series.from_dicom("path/to/dicom")

# Create plan
plan = pb.Plan(series)

# Add beams
plan.add_beam(gantry_angle=0.0, isocenter=(0, 0, 0))
plan.add_beam(gantry_angle=90.0, isocenter=(0, 0, 0))
plan.add_beam(gantry_angle=180.0, isocenter=(0, 0, 0))

# Setup optimizer
optimizer = pb.PlanOptimizer(plan)

# Define objectives
ptv = series.structures["PTV"]
optimizer.add_objective(
    structure=ptv,
    objective=pb.KLDivergenceTerm(
        dose_range=(50, 60),  # Gy
        volume_fraction=0.95,
        weight=1.0
    )
)

bladder = series.structures["Bladder"]
optimizer.add_objective(
    structure=bladder,
    objective=pb.KLDivergenceTerm(
        dose_range=(0, 40),   # Keep dose < 40 Gy
        volume_fraction=1.0,
        weight=0.5
    )
)

# Run optimization
result = optimizer.optimize(
    max_iterations=500,
    tolerance=1e-3
)

print(f"Optimization completed: {result.message}")
print(f"Final cost: {result.final_cost:.4f}")
print(f"Iterations: {result.iterations}")

# Get final dose
dose = plan.get_dose()
```

### Live Visualization
```python
import pybrimstone as pb
from pybrimstone.visualization import VisualizationCallback
import matplotlib.pyplot as plt

# Setup as before...
plan = pb.Plan(series)
optimizer = pb.PlanOptimizer(plan)

# Create visualization callback
viz = pb.VisualizationCallback(update_interval=10)

# Run with live updates
result = optimizer.optimize(callback=viz)

# Plot results
fig, axes = plt.subplots(2, 2, figsize=(12, 10))

# Convergence curve
viz.plot_convergence(ax=axes[0, 0])

# Dose distribution
viz.plot_dose_slice(slice_idx=50, ax=axes[0, 1])

# DVH
viz.plot_dvh(structures=[ptv, bladder], ax=axes[1, 0])

# Beamlet weights
viz.plot_beamlet_weights(beam_idx=0, ax=axes[1, 1])

plt.tight_layout()
plt.show()
```

### pymedphys Integration
```python
import pybrimstone as pb
from pymedphys import gamma_analysis, dose_from_dataset
import matplotlib.pyplot as plt

# Optimize as before
result = optimizer.optimize()

# Compare with reference plan
ref_dose = dose_from_dataset("reference_plan.dcm")
calc_dose = result.dose

# Gamma analysis
gamma = gamma_analysis(
    reference=ref_dose,
    evaluation=calc_dose,
    dose_threshold=0.1,
    distance_mm=3,
    dose_percent=3
)

# Visualize
plt.imshow(gamma[50, :, :], cmap='RdYlGn_r', vmin=0, vmax=2)
plt.colorbar(label='Gamma Index')
plt.title('Gamma Analysis (3mm/3%)')
plt.show()
```

## Testing Strategy

### Unit Tests
- Data structure wrapping and conversion
- Memory management and lifetime
- Callback mechanisms
- Gradient calculations

### Integration Tests
- End-to-end optimization workflows
- Multi-beam planning
- Various objective function combinations
- Callback interruption and resumption

### Performance Tests
- Optimization runtime benchmarks
- Callback overhead measurement
- Memory usage profiling
- Comparison with pure C++ implementation

### Validation Tests
- Reproduce known planning results
- Compare with TPS (Treatment Planning System) outputs
- DVH accuracy checks
- Dose calculation validation

## Documentation

### API Documentation
- Sphinx-generated API reference
- Docstrings for all public classes/methods
- Type hints throughout

### User Guide
- Getting started tutorial
- Optimization workflow guide
- Callback development guide
- Visualization examples

### Developer Guide
- Building from source
- Cython wrapping patterns
- Adding new objective functions
- Contributing guidelines

## Future Enhancements

1. **GPU Acceleration**: Offload dose calculations to GPU
2. **Distributed Computing**: Multi-node optimization
3. **Web Interface**: Browser-based planning tool
4. **Cloud Integration**: AWS/Azure deployment
5. **Machine Learning**: Learned objective functions
6. **DICOM RT Export**: Full treatment plan export

## References

- Cython documentation: https://cython.readthedocs.io
- ITK Python: https://itkpythonpackage.readthedocs.io
- pymedphys: https://docs.pymedphys.com
- NumPy C API: https://numpy.org/doc/stable/reference/c-api
