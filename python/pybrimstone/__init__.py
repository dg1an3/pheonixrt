"""
pybrimstone - Python wrapper for Brimstone radiotherapy inverse planning

Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645

This package provides a Python interface to the Brimstone inverse planning
algorithm for radiotherapy treatment planning.
"""

__version__ = "0.1.0"
__author__ = "Derek G. Lane"
__license__ = "Proprietary - See LICENSE"

# Import core components when Cython extension is built
try:
    from .core import (
        Plan,
        Beam,
        Series,
        Structure,
        PlanOptimizer,
        Prescription,
        KLDivergenceTerm,
    )

    __all__ = [
        "Plan",
        "Beam",
        "Series",
        "Structure",
        "PlanOptimizer",
        "Prescription",
        "KLDivergenceTerm",
    ]
except ImportError as e:
    import warnings

    warnings.warn(
        f"Cython extensions not built. Please run 'pip install -e .' to build. Error: {e}"
    )
    __all__ = []
