# pheonixrt

**Brimstone**: A variational inverse planning algorithm for [radiotherapy treatment planning](https://en.wikipedia.org/wiki/Radiation_treatment_planning).

pheonixrt is a research and development platform for radiation therapy optimization, implementing the Brimstone algorithm - a mathematically principled approach to inverse treatment planning based on information theory and variational methods.

## Overview

The Brimstone algorithm optimizes radiation beam intensities (beamlet weights) to deliver prescribed doses to tumor targets while minimizing exposure to healthy tissue. It employs:

- **Multi-scale pyramid optimization** for robust convergence
- **KL-divergence minimization** for dose-volume histogram (DVH) matching
- **Adaptive covariance optimization** with conjugate gradient methods
- **Implicit free energy representation** related to variational Bayes methods

## Key Features

- ✅ **Mathematically principled**: Information-theoretic cost functions (KL-divergence)
- ✅ **Robust optimization**: Multi-scale approach avoids local minima
- ✅ **Flexible prescriptions**: Arbitrary target DVH shapes supported
- ✅ **Python wrapper**: Modern interface via `pybrimstone` package
- ✅ **C++ core**: High-performance ITK-based implementation

## Project Structure

```
pheonixrt/
├── Brimstone/          # C++ core implementation (MFC application)
├── RtModel/            # Core radiotherapy models and algorithms
├── VecMat/             # Vector and matrix utilities
├── Graph/              # Visualization components
├── python/             # Python bindings (pybrimstone)
│   ├── pybrimstone/    # Python package
│   └── examples/       # Usage examples
├── notebook_zoo/       # Jupyter notebooks for research
├── docs/               # Additional documentation
├── CLAUDE.md           # Detailed algorithm documentation
└── CYTHON_WRAPPER_DESIGN.md  # Python wrapper design
```

## Getting Started

### Python Interface (Recommended)

The easiest way to use pheonixrt is through the Python wrapper:

```bash
cd python
pip install -e .
```

See [python/README.md](python/README.md) for detailed installation and usage instructions.

**Quick example:**
```python
import pybrimstone as pb

# Create treatment plan
plan = pb.Plan()
plan.add_beam(pb.Beam(gantry_angle=0.0))
plan.add_beam(pb.Beam(gantry_angle=180.0))

# Optimize
optimizer = pb.PlanOptimizer(plan)
result = optimizer.optimize()
```

### C++ Implementation

The C++ implementation requires:
- Visual Studio 2010 or later (Windows)
- ITK (Insight Toolkit) library
- MFC (Microsoft Foundation Classes)

Build using `Brimstone_src.sln` in Visual Studio.

## Documentation

- **[CLAUDE.md](CLAUDE.md)** - Comprehensive algorithm documentation with technical details
- **[python/README.md](python/README.md)** - Python wrapper documentation and examples
- **[CYTHON_WRAPPER_DESIGN.md](CYTHON_WRAPPER_DESIGN.md)** - Python binding architecture
- **[docs/](docs/)** - Additional technical documents and research notes

### Research Notebooks

The `notebook_zoo/` directory contains Jupyter notebooks exploring:
- Entropy maximization methods
- Free energy formulations
- Variational Bayes connections

## Algorithm Details

Brimstone uses a multi-level optimization approach:

1. **Hierarchical pyramid**: Optimization proceeds from coarse to fine resolution
2. **Cost function**: KL-divergence between target and calculated DVHs
3. **Optimizer**: Polak-Ribiere conjugate gradient with Brent line search
4. **Adaptive variance**: Dynamic covariance adjustment during search

For complete technical details, see [CLAUDE.md](CLAUDE.md).

## License

**[-MIND THE LICENSE-](LICENSE)**

U.S. Patent 7,369,645

Copyright (c) 2007-2021, Derek G. Lane  
All rights reserved.

This software is proprietary. See [LICENSE](LICENSE) file for terms.

## Citation

If you use this software in academic research, please cite:

```
Brimstone Inverse Planning Algorithm
Derek G. Lane
U.S. Patent 7,369,645
```

## Related Concepts

- **Variational Bayes**: Similar KL-divergence minimization framework
- **Free Energy Minimization**: Information-theoretic optimization
- **Inverse Problems**: General mathematical framework for inverse planning

## Contributing

This is an active research and development project. For questions or collaboration:
- See existing issues and discussions
- Review [CYTHON_WRAPPER_DESIGN.md](CYTHON_WRAPPER_DESIGN.md) for development roadmap

## References

- [Radiation Treatment Planning (Wikipedia)](https://en.wikipedia.org/wiki/Radiation_treatment_planning)
- [Variational Bayes Methods](https://en.wikipedia.org/wiki/Variational_Bayesian_methods)
- [ITK - Insight Toolkit](https://itk.org/)
