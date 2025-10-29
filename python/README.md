# pybrimstone - Python Wrapper for Brimstone Optimization

Python bindings for the Brimstone inverse planning algorithm for radiotherapy treatment planning.

## Overview

pybrimstone provides a Pythonic interface to the Brimstone optimization algorithm, enabling:

- **Modern tooling**: Use Python data science ecosystem (NumPy, matplotlib, Jupyter)
- **Visualization**: Real-time monitoring of optimization with pymedphys
- **Flexibility**: Easy integration with custom workflows and scripts
- **Performance**: Direct access to optimized C++ implementation

## Installation

### Prerequisites

- Python 3.8 or later
- C++ compiler (MSVC on Windows, GCC/Clang on Linux)
- NumPy
- Cython

### Build from Source

```bash
# Clone repository
cd pheonixrt/python

# Install in development mode
pip install -e .

# Or install with visualization dependencies
pip install -e ".[viz]"

# Or install everything
pip install -e ".[all]"
```

## Quick Start

```python
import pybrimstone as pb
import numpy as np

# Create a plan
plan = pb.Plan()

# Add beams
plan.add_beam(pb.Beam(gantry_angle=0.0))
plan.add_beam(pb.Beam(gantry_angle=90.0))
plan.add_beam(pb.Beam(gantry_angle=180.0))

# Create optimizer
optimizer = pb.PlanOptimizer(plan)

# Run optimization
result = optimizer.optimize()

print(f"Optimization completed successfully: {result['success']}")
print(f"Final weights shape: {result['final_weights'].shape}")
```

## Features

### Core Functionality (Phase 1 - Current)

- [x] Basic C++ class wrapping (Plan, Beam, Series, Structure)
- [x] PlanOptimizer wrapper with NumPy integration
- [x] State vector access and manipulation
- [x] Build system and packaging

### Coming Soon

- [ ] Callback mechanism for live monitoring
- [ ] ITK volume ↔ NumPy array conversion
- [ ] DICOM I/O integration
- [ ] Visualization utilities
- [ ] DVH plotting
- [ ] pymedphys integration

## Architecture

```
pybrimstone/
├── core.pyx           # Cython implementation
├── core.pxd           # C++ declarations
├── callbacks.py       # Python callback classes (Phase 3)
├── visualization.py   # Visualization utilities (Phase 4)
└── io.py             # DICOM/data I/O (Phase 4)
```

## Development Status

**Current Phase: Phase 1 - Core Data Structures**

See `CYTHON_WRAPPER_DESIGN.md` for detailed roadmap.

### Phase 1: Core Data Structures ✓
- Basic wrapping of Plan, Beam, Series, Structure
- PlanOptimizer with simple optimize() method
- NumPy array integration for state vectors
- Build system and packaging

### Phase 2: Optimization Core (Next)
- Full Prescription and objective term wrapping
- Callback mechanism (C++ → Python)
- Multi-scale pyramid access
- Comprehensive gradient handling

### Phase 3: Callbacks & Monitoring
- Python callback infrastructure
- VisualizationCallback class
- Convergence tracking
- Real-time DVH updates

### Phase 4: Visualization & Integration
- pymedphys integration
- DVH plotting
- Dose overlays
- Interactive Jupyter examples

## Examples

### Basic Optimization

```python
import pybrimstone as pb

# Load or create plan
plan = pb.Plan()

# Setup beams
for angle in [0, 45, 90, 135, 180, 225, 270, 315]:
    plan.add_beam(pb.Beam(gantry_angle=angle))

# Optimize
optimizer = pb.PlanOptimizer(plan)
result = optimizer.optimize(
    max_iterations=500,
    tolerance=1e-3
)

# Access results
weights = result['final_weights']
print(f"Optimized {len(weights)} beamlet weights")
```

### Working with State Vectors

```python
# Get current beamlet weights
weights = optimizer.get_state_vector()
print(f"Current weights: {weights.shape}")

# Modify weights
weights *= 1.1  # Scale up by 10%

# Set new weights
optimizer.set_state_vector(weights)

# Update plan
plan.update_histograms()
```

## Contributing

This is a work in progress. Contributions welcome!

See the design document for implementation details:
- `/pheonixrt/CYTHON_WRAPPER_DESIGN.md`

## License

Copyright (C) 2nd Messenger Systems
U.S. Patent 7,369,645

Proprietary - See LICENSE file

## References

- [Brimstone Algorithm Documentation](../CLAUDE.md)
- [Cython Documentation](https://cython.readthedocs.io)
- [pymedphys](https://docs.pymedphys.com)
