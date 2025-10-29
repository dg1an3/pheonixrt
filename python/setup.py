"""
Setup script for pybrimstone - Python wrapper for Brimstone optimization algorithm

Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
"""

import os
import sys
from pathlib import Path

import numpy
from Cython.Build import cythonize
from setuptools import Extension, setup

# Determine platform-specific settings
is_windows = sys.platform.startswith("win")
is_linux = sys.platform.startswith("linux")

# Base paths
PROJECT_ROOT = Path(__file__).parent.parent
RTMODEL_DIR = PROJECT_ROOT / "RtModel"
RTMODEL_INCLUDE = RTMODEL_DIR / "include"
VECMAT_DIR = PROJECT_ROOT / "VecMat"
GRAPH_DIR = PROJECT_ROOT / "Graph"

# Common include directories
include_dirs = [
    str(RTMODEL_INCLUDE),
    str(VECMAT_DIR),
    str(GRAPH_DIR),
    numpy.get_include(),
]

# Common library directories
library_dirs = []

# Common libraries to link
libraries = []

# Compiler flags
extra_compile_args = []
extra_link_args = []

if is_windows:
    # Windows-specific settings
    extra_compile_args = [
        "/std:c++14",
        "/EHsc",
        "/D_USE_MATH_DEFINES",
        "/DUSE_RTOPT",
        "/DWIN32",
        "/D_WINDOWS",
    ]
    # Add ITK paths (adjust as needed for your system)
    # include_dirs.append("C:/Program Files/ITK/include")
    # library_dirs.append("C:/Program Files/ITK/lib")

elif is_linux:
    # Linux-specific settings
    extra_compile_args = [
        "-std=c++14",
        "-DUSE_RTOPT",
        "-fPIC",
    ]
    extra_link_args = ["-Wl,-rpath,$ORIGIN"]

# Define extensions
extensions = [
    Extension(
        name="pybrimstone.core",
        sources=[
            "pybrimstone/core.pyx",
            # C++ source files to compile
            str(RTMODEL_DIR / "PlanOptimizer.cpp"),
            str(RTMODEL_DIR / "ConjGradOptimizer.cpp"),
            str(RTMODEL_DIR / "Prescription.cpp"),
            str(RTMODEL_DIR / "KLDivTerm.cpp"),
            str(RTMODEL_DIR / "VOITerm.cpp"),
            str(RTMODEL_DIR / "ObjectiveFunction.cpp"),
            str(RTMODEL_DIR / "PlanPyramid.cpp"),
            str(RTMODEL_DIR / "Plan.cpp"),
            str(RTMODEL_DIR / "Beam.cpp"),
            str(RTMODEL_DIR / "Structure.cpp"),
            str(RTMODEL_DIR / "Series.cpp"),
            str(RTMODEL_DIR / "Histogram.cpp"),
            str(RTMODEL_DIR / "HistogramGradient.cpp"),
        ],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
        language="c++",
    ),
]

# Cythonize extensions
ext_modules = cythonize(
    extensions,
    compiler_directives={
        "language_level": "3",
        "embedsignature": True,
        "boundscheck": False,
        "wraparound": False,
    },
)

# Run setup
setup(
    ext_modules=ext_modules,
    zip_safe=False,
)
