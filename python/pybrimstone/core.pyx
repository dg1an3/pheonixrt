# distutils: language = c++
# cython: language_level=3

"""
Cython implementation of Python wrappers for Brimstone C++ classes

This file implements the Python-facing classes that wrap the C++ implementation.
"""

import numpy as np
cimport numpy as cnp
from libc.stdlib cimport malloc, free
from libcpp cimport bool

# Import C++ declarations
from pybrimstone.core cimport (
    CPlan,
    CBeam,
    CSeries,
    CStructure,
    CVectorN,
    PlanOptimizer as CPlanOptimizer,
    Prescription as CPrescription,
    KLDivTerm as CKLDivTerm,
    DynamicCovarianceOptimizer,
    OptimizerCallback,
)

# Initialize numpy
cnp.import_array()


cdef class Plan:
    """
    Treatment plan containing beams and dose calculations

    A Plan represents a complete radiotherapy treatment plan including
    the CT/imaging series, treatment beams, and dose calculations.

    Examples:
        >>> plan = Plan()
        >>> beam = Beam(gantry_angle=0.0)
        >>> plan.add_beam(beam)
    """
    cdef CPlan* _c_plan
    cdef bool _owns_pointer

    def __cinit__(self):
        self._c_plan = new CPlan()
        self._owns_pointer = True

    def __dealloc__(self):
        if self._owns_pointer and self._c_plan != NULL:
            del self._c_plan

    @property
    def beam_count(self) -> int:
        """Number of beams in the plan"""
        if self._c_plan == NULL:
            return 0
        return self._c_plan.GetBeamCount()

    @property
    def total_beamlet_count(self) -> int:
        """Total number of beamlets across all beams"""
        if self._c_plan == NULL:
            return 0
        return self._c_plan.GetTotalBeamletCount()

    def update_histograms(self) -> None:
        """Update all dose-volume histograms"""
        if self._c_plan != NULL:
            self._c_plan.UpdateAllHisto()

    def __repr__(self) -> str:
        return f"Plan(beams={self.beam_count}, beamlets={self.total_beamlet_count})"


cdef class Beam:
    """
    Single treatment beam

    A Beam represents one radiation beam with a specific gantry angle,
    isocenter, and beamlet intensity pattern.

    Args:
        gantry_angle: Angle of gantry rotation in degrees

    Examples:
        >>> beam = Beam(gantry_angle=90.0)
        >>> print(beam.gantry_angle)
        90.0
    """
    cdef CBeam* _c_beam
    cdef bool _owns_pointer

    def __cinit__(self, double gantry_angle=0.0):
        self._c_beam = new CBeam()
        self._owns_pointer = True
        self._c_beam.SetGantryAngle(gantry_angle)

    def __dealloc__(self):
        if self._owns_pointer and self._c_beam != NULL:
            del self._c_beam

    @property
    def gantry_angle(self) -> float:
        """Gantry angle in degrees"""
        if self._c_beam == NULL:
            return 0.0
        return self._c_beam.GetGantryAngle()

    @gantry_angle.setter
    def gantry_angle(self, double angle):
        """Set gantry angle in degrees"""
        if self._c_beam != NULL:
            self._c_beam.SetGantryAngle(angle)

    @property
    def beamlet_count(self) -> int:
        """Number of beamlets in this beam"""
        if self._c_beam == NULL:
            return 0
        return self._c_beam.GetBeamletCount()

    def __repr__(self) -> str:
        return f"Beam(gantry_angle={self.gantry_angle:.1f}°, beamlets={self.beamlet_count})"


cdef class Series:
    """
    CT/imaging data series

    A Series contains the patient CT scan and associated anatomical structures.

    Examples:
        >>> series = Series()
    """
    cdef CSeries* _c_series
    cdef bool _owns_pointer

    def __cinit__(self):
        self._c_series = new CSeries()
        self._owns_pointer = True

    def __dealloc__(self):
        if self._owns_pointer and self._c_series != NULL:
            del self._c_series

    def __repr__(self) -> str:
        return "Series()"


cdef class Structure:
    """
    Anatomical structure (Volume of Interest)

    A Structure represents a contoured anatomical region such as a tumor
    or organ at risk.

    Examples:
        >>> structure = Structure(name="PTV")
    """
    cdef CStructure* _c_structure
    cdef bool _owns_pointer

    def __cinit__(self):
        self._c_structure = new CStructure()
        self._owns_pointer = True

    def __dealloc__(self):
        if self._owns_pointer and self._c_structure != NULL:
            del self._c_structure

    def __repr__(self) -> str:
        return "Structure()"


# Helper function to convert numpy array to CVectorN
cdef CVectorN* numpy_to_cvectorn(cnp.ndarray[cnp.float64_t, ndim=1] arr):
    """Convert numpy array to C++ CVectorN"""
    cdef CVectorN* vec = new CVectorN(len(arr))
    cdef int i
    for i in range(len(arr)):
        vec[0][i] = arr[i]
    return vec


# Helper function to convert CVectorN to numpy array
cdef cnp.ndarray cvectorn_to_numpy(CVectorN* vec):
    """Convert C++ CVectorN to numpy array"""
    cdef int dim = vec.GetDim()
    cdef cnp.ndarray[cnp.float64_t, ndim=1] arr = np.empty(dim, dtype=np.float64)
    cdef int i
    for i in range(dim):
        arr[i] = vec[0][i]
    return arr


cdef class PlanOptimizer:
    """
    Multi-scale optimizer for treatment planning

    The PlanOptimizer manages the hierarchical optimization process,
    progressing from coarse to fine resolution levels.

    Args:
        plan: The treatment plan to optimize

    Examples:
        >>> plan = Plan()
        >>> optimizer = PlanOptimizer(plan)
        >>> result = optimizer.optimize()
    """
    cdef CPlanOptimizer* _c_optimizer
    cdef Plan _plan
    cdef object _callback

    def __cinit__(self, Plan plan):
        self._plan = plan
        self._c_optimizer = new CPlanOptimizer(plan._c_plan)
        self._callback = None

    def __dealloc__(self):
        if self._c_optimizer != NULL:
            del self._c_optimizer

    def optimize(
        self,
        initial_weights=None,
        callback=None,
        max_iterations: int = 500,
        tolerance: float = 1e-3
    ):
        """
        Run the optimization

        Args:
            initial_weights: Initial beamlet weights (optional)
            callback: Callback function called at each iteration (optional)
            max_iterations: Maximum number of iterations per pyramid level
            tolerance: Convergence tolerance

        Returns:
            Dictionary containing optimization results
        """
        # Store callback for use in C++ callback
        self._callback = callback

        # Create initial state vector
        cdef CVectorN vInit
        if initial_weights is None:
            # Use default initialization from optimizer
            self._c_optimizer.GetStateVectorFromPlan(vInit)
        else:
            # Convert numpy array to CVectorN
            weights_array = np.asarray(initial_weights, dtype=np.float64)
            vInit.SetDim(len(weights_array))
            for i in range(len(weights_array)):
                vInit[i] = weights_array[i]

        # Run optimization
        # Note: Callback mechanism needs more sophisticated implementation
        cdef bool success = self._c_optimizer.Optimize(vInit, NULL, NULL)

        # Get final weights
        final_weights = cvectorn_to_numpy(&vInit)

        return {
            'success': success,
            'final_weights': final_weights,
            'iterations': 0,  # TODO: get from optimizer
            'final_cost': 0.0,  # TODO: get from optimizer
        }

    def get_state_vector(self) -> cnp.ndarray:
        """Get current beamlet weights as numpy array"""
        cdef CVectorN vState
        self._c_optimizer.GetStateVectorFromPlan(vState)
        return cvectorn_to_numpy(&vState)

    def set_state_vector(self, cnp.ndarray[cnp.float64_t, ndim=1] weights):
        """Set beamlet weights from numpy array"""
        cdef CVectorN vState
        vState.SetDim(len(weights))
        for i in range(len(weights)):
            vState[i] = weights[i]
        self._c_optimizer.SetStateVectorToPlan(vState)

    def __repr__(self) -> str:
        return f"PlanOptimizer(plan={self._plan})"


cdef class Prescription:
    """
    Cost function for optimization

    The Prescription defines the optimization objectives by combining
    multiple structure-specific terms.

    Args:
        plan: The treatment plan

    Examples:
        >>> prescription = Prescription(plan)
        >>> prescription.add_kl_divergence_term(structure, weight=1.0)
    """
    cdef CPrescription* _c_prescription
    cdef Plan _plan

    def __cinit__(self, Plan plan):
        self._plan = plan
        self._c_prescription = new CPrescription(plan._c_plan)

    def __dealloc__(self):
        if self._c_prescription != NULL:
            del self._c_prescription

    def __repr__(self) -> str:
        return "Prescription()"


cdef class KLDivergenceTerm:
    """
    KL-divergence DVH matching objective

    This objective term minimizes the Kullback-Leibler divergence between
    the calculated dose-volume histogram and a target distribution.

    Args:
        structure: The anatomical structure
        weight: Relative weight of this objective (default: 1.0)

    Examples:
        >>> term = KLDivergenceTerm(structure, weight=1.0)
        >>> term.set_interval(dose_min=50.0, dose_max=60.0, fraction=0.95)
    """
    cdef CKLDivTerm* _c_term
    cdef Structure _structure

    def __cinit__(self, Structure structure, double weight=1.0):
        self._structure = structure
        self._c_term = new CKLDivTerm(structure._c_structure, weight)

    def __dealloc__(self):
        if self._c_term != NULL:
            del self._c_term

    def set_interval(
        self,
        double dose_min,
        double dose_max,
        double fraction,
        bool use_midpoint=False
    ):
        """
        Set target dose interval

        Args:
            dose_min: Minimum dose (Gy)
            dose_max: Maximum dose (Gy)
            fraction: Volume fraction (0-1)
            use_midpoint: Use midpoint of interval (default: False)
        """
        if self._c_term != NULL:
            self._c_term.SetInterval(dose_min, dose_max, fraction, use_midpoint)

    @property
    def min_dose(self) -> float:
        """Minimum dose in target interval"""
        if self._c_term == NULL:
            return 0.0
        return self._c_term.GetMinDose()

    @property
    def max_dose(self) -> float:
        """Maximum dose in target interval"""
        if self._c_term == NULL:
            return 0.0
        return self._c_term.GetMaxDose()

    def __repr__(self) -> str:
        return f"KLDivergenceTerm(dose_range=[{self.min_dose:.1f}, {self.max_dose:.1f}] Gy)"
