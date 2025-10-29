// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// SigmaEstimator_Integration_Example.cpp
//
// This file shows how to integrate SigmaEstimator into PlanOptimizer
// to enable adaptive sigma estimation based on structure characteristics.

#include "stdafx.h"
#include "PlanOptimizer.h"
#include "SigmaEstimator.h"

// NOTE: This is example code showing integration approach.
// Actual integration would modify PlanOptimizer.cpp directly.

namespace dH {

///////////////////////////////////////////////////////////////////////////////
// EXAMPLE 1: Basic integration in PlanOptimizer::SetupPrescription
///////////////////////////////////////////////////////////////////////////////

void PlanOptimizer_Example_SetupPrescription_WithAdaptiveSigma(
	CPlan* pPlan,
	Prescription* pPresc)
{
	// Create sigma estimator
	SigmaEstimator sigmaEst;

	// Configure estimator
	sigmaEst.SetSigmaRange(0.1, 16.0);  // Min/max allowed sigmas

	// Get voxel spacing from plan dose matrix
	VolumeReal* pDose = pPlan->GetDoseMatrix();
	if (pDose) {
		VolumeReal::SpacingType spacing = pDose->GetSpacing();
		sigmaEst.SetVoxelSpacing(spacing[0], spacing[1], spacing[2]);
	}

	// Enable all estimation methods
	sigmaEst.SetUseStructureComplexity(true);
	sigmaEst.SetUseDoseGradient(true);
	sigmaEst.SetUsePrescriptionRange(true);
	sigmaEst.SetUseVolumeSize(true);

	// Estimate pyramid sigmas based on plan characteristics
	std::vector<REAL> adaptiveSigmas = sigmaEst.EstimatePyramidSigmas(
		pPlan,
		pPresc,
		PlanPyramid::MAX_SCALES);

	// Use estimated sigmas in prescription setup
	const REAL GBinSigma = 0.2;  // Keep constant

	for (int nLevel = 0; nLevel < PlanPyramid::MAX_SCALES; nLevel++) {
		// Use adaptive sigma instead of hard-coded default
		const REAL sigma = adaptiveSigmas[nLevel];

		// Calculate variance range as before
		const REAL binVar = pow(GBinSigma / sigma, 2);
		const REAL varMin = binVar * 0.25;
		const REAL varMax = binVar;

		// Construct optimizer with calculated variances
		DynamicCovarianceOptimizer *pOptimizer =
			new DynamicCovarianceOptimizer(pPresc);
		pOptimizer->SetAdaptiveVariance(true, varMin, varMax);

		// Set convergence tolerances
		const REAL cgTol = 1e-3;  // From registry or default
		pOptimizer->set_f_tolerance(cgTol);

		// Apply slope variance correction (disabled at coarsest level)
		if (nLevel == PlanPyramid::MAX_SCALES-1)
			pPresc->SetTransformSlopeVariance(false);

		pPresc->SetGBinVar(varMin, varMax);

		// Log the estimated sigma
		TRACE("Level %d: Adaptive Sigma = %.3f (was %.3f)\n",
			nLevel,
			sigma,
			DEFAULT_LEVELSIGMA[nLevel]);
	}
}

///////////////////////////////////////////////////////////////////////////////
// EXAMPLE 2: Structure-specific sigma estimation
///////////////////////////////////////////////////////////////////////////////

void Example_PerStructureSigmaEstimation(CPlan* pPlan, Prescription* pPresc)
{
	SigmaEstimator sigmaEst;
	VolumeReal* pDose = pPlan->GetDoseMatrix();

	// Iterate through all VOI terms
	POSITION pos = pPresc->m_mapVOITs.GetStartPosition();
	while (pos != NULL) {
		Structure* pStruct = NULL;
		VOITerm* pVOIT = NULL;
		pPresc->m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		if (pStruct && pVOIT) {
			// Estimate optimal sigma for this specific structure
			REAL structSigma = sigmaEst.EstimateStructureSigma(
				pStruct,
				pDose,
				pVOIT);

			// Log structure-specific recommendations
			TRACE("Structure: %s, Recommended base sigma: %.3f\n",
				pStruct->GetName().c_str(),
				structSigma);

			// Could use this for structure-specific weighting or
			// optimization prioritization
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// EXAMPLE 3: Dynamic sigma adaptation during optimization
///////////////////////////////////////////////////////////////////////////////

class AdaptiveSigmaCallback : public vnl_least_squares_function
{
public:
	AdaptiveSigmaCallback(
		SigmaEstimator* pSigmaEst,
		Prescription* pPresc,
		int level)
		: m_pSigmaEst(pSigmaEst)
		, m_pPresc(pPresc)
		, m_level(level)
		, m_lastCost(1e10)
		, m_iteration(0)
	{
	}

	void OnIteration(REAL currentCost, REAL gradientNorm)
	{
		// Calculate cost improvement
		REAL costImprovement = (m_lastCost - currentCost) / (m_lastCost + 1e-6);

		// Get current sigma (would need to store in Prescription or optimizer)
		REAL currentSigma = GetCurrentSigma();

		// Adapt sigma based on convergence behavior
		REAL adaptedSigma = m_pSigmaEst->AdaptSigmaFromConvergence(
			currentSigma,
			m_iteration,
			costImprovement,
			gradientNorm);

		// If sigma changed significantly, update prescription
		if (fabs(adaptedSigma - currentSigma) > 0.01) {
			UpdatePrescriptionSigma(adaptedSigma);
			TRACE("Level %d, Iter %d: Adapted sigma %.3f -> %.3f\n",
				m_level, m_iteration, currentSigma, adaptedSigma);
		}

		m_lastCost = currentCost;
		m_iteration++;
	}

private:
	REAL GetCurrentSigma() {
		// Would need to extract from prescription or optimizer state
		// This is a placeholder
		return 0.5;
	}

	void UpdatePrescriptionSigma(REAL newSigma) {
		// Recalculate variance range with new sigma
		const REAL GBinSigma = 0.2;
		const REAL binVar = pow(GBinSigma / newSigma, 2);
		const REAL varMin = binVar * 0.25;
		const REAL varMax = binVar;

		// Update prescription
		m_pPresc->SetGBinVar(varMin, varMax);
	}

	SigmaEstimator* m_pSigmaEst;
	Prescription* m_pPresc;
	int m_level;
	REAL m_lastCost;
	int m_iteration;
};

///////////////////////////////////////////////////////////////////////////////
// EXAMPLE 4: Comparison mode - run with both default and adaptive sigmas
///////////////////////////////////////////////////////////////////////////////

void Example_CompareDefaultVsAdaptive(CPlan* pPlan, Prescription* pPresc)
{
	// Default sigmas (current hard-coded values)
	const REAL DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25};

	// Estimate adaptive sigmas
	SigmaEstimator sigmaEst;
	std::vector<REAL> adaptiveSigmas = sigmaEst.EstimatePyramidSigmas(
		pPlan,
		pPresc,
		PlanPyramid::MAX_SCALES);

	// Print comparison
	TRACE("\n===== Sigma Comparison =====\n");
	TRACE("Level | Default | Adaptive | Difference\n");
	TRACE("------|---------|----------|------------\n");

	for (int i = 0; i < PlanPyramid::MAX_SCALES; i++) {
		REAL diff = adaptiveSigmas[i] - DEFAULT_LEVELSIGMA[i];
		REAL pctDiff = 100.0 * diff / DEFAULT_LEVELSIGMA[i];

		TRACE("  %d   |  %.3f  |  %.3f   | %+.3f (%+.1f%%)\n",
			i,
			DEFAULT_LEVELSIGMA[i],
			adaptiveSigmas[i],
			diff,
			pctDiff);
	}
	TRACE("============================\n\n");

	// Could run optimization twice and compare results:
	// 1. With DEFAULT_LEVELSIGMA
	// 2. With adaptiveSigmas
	// Then compare convergence speed, final cost, etc.
}

///////////////////////////////////////////////////////////////////////////////
// EXAMPLE 5: Registry integration for enabling adaptive sigmas
///////////////////////////////////////////////////////////////////////////////

void Example_RegistryControlledAdaptiveSigma(CPlan* pPlan, Prescription* pPresc)
{
	// Check registry key to enable/disable adaptive sigma
	const CString ADAPTIVE_SIGMA_KEY = _T("UseAdaptiveSigma");
	BOOL bUseAdaptive = FALSE;

	// Read from registry (would use GetProfileInt or similar)
	// bUseAdaptive = GetProfileInt("Prescription", ADAPTIVE_SIGMA_KEY, FALSE);

	std::vector<REAL> sigmas;

	if (bUseAdaptive) {
		// Use adaptive estimation
		SigmaEstimator sigmaEst;
		sigmas = sigmaEst.EstimatePyramidSigmas(
			pPlan,
			pPresc,
			PlanPyramid::MAX_SCALES);

		TRACE("Using adaptive sigma estimation\n");
	} else {
		// Use default hard-coded values
		const REAL DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25};
		sigmas.assign(DEFAULT_LEVELSIGMA,
		              DEFAULT_LEVELSIGMA + PlanPyramid::MAX_SCALES);

		TRACE("Using default sigma values\n");
	}

	// Continue with prescription setup using chosen sigmas...
}

///////////////////////////////////////////////////////////////////////////////
// EXAMPLE 6: Detailed analysis and logging
///////////////////////////////////////////////////////////////////////////////

void Example_DetailedSigmaAnalysis(CPlan* pPlan, Prescription* pPresc)
{
	SigmaEstimator sigmaEst;
	VolumeReal* pDose = pPlan->GetDoseMatrix();

	TRACE("\n===== Structure-Based Sigma Analysis =====\n");

	// Analyze each structure
	POSITION pos = pPresc->m_mapVOITs.GetStartPosition();
	while (pos != NULL) {
		Structure* pStruct = NULL;
		VOITerm* pVOIT = NULL;
		pPresc->m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		if (pStruct) {
			TRACE("\nStructure: %s\n", pStruct->GetName().c_str());

			// Individual estimates
			REAL geomSigma = sigmaEst.EstimateFromGeometry(pStruct);
			TRACE("  Geometry-based sigma: %.3f\n", geomSigma);

			if (pDose) {
				REAL gradSigma = sigmaEst.EstimateFromDoseGradient(pStruct, pDose);
				TRACE("  Dose gradient sigma: %.3f\n", gradSigma);
			}

			if (pVOIT) {
				REAL prescSigma = sigmaEst.EstimateFromPrescriptionRange(pVOIT);
				TRACE("  Prescription sigma: %.3f\n", prescSigma);
			}

			REAL volSigma = sigmaEst.EstimateFromVolume(pStruct);
			TRACE("  Volume-based sigma: %.3f\n", volSigma);

			// Combined estimate
			REAL combinedSigma = sigmaEst.EstimateStructureSigma(
				pStruct, pDose, pVOIT);
			TRACE("  Combined estimate: %.3f\n", combinedSigma);

			// Additional metrics
			REAL sphericity = sigmaEst.CalculateSphericity(pStruct);
			REAL volume = sigmaEst.ApproximateVolume(pStruct);
			TRACE("  Sphericity: %.3f, Volume: %.1f cc\n",
				sphericity, volume);
		}
	}

	TRACE("\n==========================================\n");
}

///////////////////////////////////////////////////////////////////////////////
// EXAMPLE 7: Hybrid approach - use adaptive for initial levels only
///////////////////////////////////////////////////////////////////////////////

void Example_HybridSigmaStrategy(CPlan* pPlan, Prescription* pPresc)
{
	SigmaEstimator sigmaEst;

	// Estimate adaptive sigmas
	std::vector<REAL> adaptiveSigmas = sigmaEst.EstimatePyramidSigmas(
		pPlan, pPresc, PlanPyramid::MAX_SCALES);

	// Default sigmas
	const REAL DEFAULT_LEVELSIGMA[] = {8.0, 3.2, 1.3, 0.5, 0.25};

	// Hybrid: Use adaptive for coarse levels, default for fine levels
	std::vector<REAL> hybridSigmas(PlanPyramid::MAX_SCALES);

	for (int i = 0; i < PlanPyramid::MAX_SCALES; i++) {
		if (i < 3) {
			// Coarse levels: use adaptive (more important for global structure)
			hybridSigmas[i] = adaptiveSigmas[i];
		} else {
			// Fine levels: use proven defaults (less risk)
			hybridSigmas[i] = DEFAULT_LEVELSIGMA[i];
		}

		TRACE("Level %d: Hybrid sigma = %.3f\n", i, hybridSigmas[i]);
	}

	// Use hybridSigmas in prescription setup...
}

} // namespace dH

///////////////////////////////////////////////////////////////////////////////
// INTEGRATION CHECKLIST
///////////////////////////////////////////////////////////////////////////////
/*

To integrate SigmaEstimator into production code:

1. Add SigmaEstimator.h to RtModel/include/
2. Add SigmaEstimator.cpp to RtModel/
3. Add to project build files (vcxproj, CMakeLists.txt, etc.)

4. Modify PlanOptimizer.cpp:
   a) #include "SigmaEstimator.h" at top
   b) In SetupPrescription(), replace DEFAULT_LEVELSIGMA with
      adaptive estimation (see Example 1)
   c) Add registry key "UseAdaptiveSigma" to enable/disable

5. Optional enhancements:
   a) Add convergence-based adaptation (Example 3)
   b) Log sigma comparison for analysis (Example 4)
   c) Per-structure sigma reporting (Example 6)

6. Testing:
   a) Run with UseAdaptiveSigma=0 (default behavior)
   b) Run with UseAdaptiveSigma=1 (adaptive)
   c) Compare convergence speed and final plan quality
   d) Validate across diverse cases (prostate, H&N, lung, etc.)

7. Documentation:
   a) Update CLAUDE.md with adaptive sigma section
   b) Document registry keys
   c) Add to technical manual

*/
