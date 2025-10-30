# distutils: language = c++
# cython: language_level=3

"""
Cython declarations for Brimstone C++ classes

This file declares the C++ classes and methods that will be wrapped
for Python access.
"""

from libcpp cimport bool
from libcpp.vector cimport vector
from libcpp.string cimport string

# Forward declarations
cdef extern from "Plan.h" namespace "dH":
    cdef cppclass CPlan "dH::Plan":
        CPlan() except +
        int GetBeamCount()
        int GetTotalBeamletCount()
        void UpdateAllHisto()

cdef extern from "Beam.h" namespace "dH":
    cdef cppclass CBeam "dH::Beam":
        CBeam() except +
        double GetGantryAngle()
        void SetGantryAngle(double angle)
        int GetBeamletCount()

cdef extern from "Series.h" namespace "dH":
    cdef cppclass CSeries "dH::Series":
        CSeries() except +

cdef extern from "Structure.h" namespace "dH":
    cdef cppclass CStructure "dH::Structure":
        CStructure() except +
        # const char* GetName()
        # void SetName(const char* name)

cdef extern from "VectorN.h":
    cdef cppclass CVectorN "CVectorN<>":
        CVectorN() except +
        CVectorN(int dim) except +
        int GetDim()
        void SetDim(int dim)
        double& operator[](int index)

cdef extern from "PlanOptimizer.h" namespace "dH":
    # Callback function type
    ctypedef bool (*OptimizerCallback)(void* pOpt, void* pParam)

    cdef cppclass PlanOptimizer:
        PlanOptimizer(CPlan* pPlan) except +
        bool Optimize(CVectorN& vInit, OptimizerCallback pFunc, void* pParam)
        void GetStateVectorFromPlan(CVectorN& vState)
        void SetStateVectorToPlan(const CVectorN& vState)

cdef extern from "Prescription.h" namespace "dH":
    cdef cppclass Prescription:
        Prescription(CPlan* pPlan) except +
        void AddStructureTerm(void* pST)  # VOITerm pointer
        void RemoveStructureTerm(CStructure* pStruct)

cdef extern from "KLDivTerm.h" namespace "dH":
    cdef cppclass KLDivTerm:
        KLDivTerm(CStructure* pStructure, double weight) except +
        void SetInterval(double low, double high, double fraction, bool bMid)
        double GetMinDose()
        double GetMaxDose()

cdef extern from "ConjGradOptimizer.h":
    cdef cppclass DynamicCovarianceOptimizer:
        DynamicCovarianceOptimizer(void* pFunc) except +  # DynamicCovarianceCostFunction*
        void SetAdaptiveVariance(bool bCalcVar, double varMin, double varMax)
        double GetFinalValue()
