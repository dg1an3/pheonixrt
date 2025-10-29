// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// SigmaEstimator.h - Adaptive sigma estimation for multi-scale optimization
#pragma once

#include <Structure.h>
#include <Plan.h>
#include <Prescription.h>
#include <VOITerm.h>
#include <VectorN.h>

namespace dH
{

///////////////////////////////////////////////////////////////////////////////
// class SigmaEstimator
//
// Provides adaptive estimation of sigma values for multi-scale pyramid
// optimization based on structure characteristics, dose complexity, and
// optimization behavior.
///////////////////////////////////////////////////////////////////////////////
class SigmaEstimator
{
public:
	// Constructor
	SigmaEstimator();
	virtual ~SigmaEstimator();

	///////////////////////////////////////////////////////////////////////////
	// Configuration Parameters
	///////////////////////////////////////////////////////////////////////////

	/** Set the default sigma values (fallback if estimation fails) */
	void SetDefaultSigmas(const std::vector<REAL>& defaultSigmas);

	/** Set the allowed sigma range */
	void SetSigmaRange(REAL minSigma, REAL maxSigma);

	/** Set voxel spacing (mm) for spatial calculations */
	void SetVoxelSpacing(REAL spacingX, REAL spacingY, REAL spacingZ);

	/** Enable/disable specific estimation methods */
	void SetUseStructureComplexity(bool bUse) { m_bUseStructureComplexity = bUse; }
	void SetUseDoseGradient(bool bUse) { m_bUseDoseGradient = bUse; }
	void SetUsePrescriptionRange(bool bUse) { m_bUsePrescriptionRange = bUse; }
	void SetUseVolumeSize(bool bUse) { m_bUseVolumeSize = bUse; }

	///////////////////////////////////////////////////////////////////////////
	// Primary Estimation Methods
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Estimate sigma values for all pyramid levels based on plan characteristics
	 * @param pPlan The treatment plan
	 * @param pPresc The prescription with VOI terms
	 * @param nLevels Number of pyramid levels
	 * @return Vector of estimated sigma values (coarsest to finest)
	 */
	std::vector<REAL> EstimatePyramidSigmas(
		CPlan* pPlan,
		Prescription* pPresc,
		int nLevels);

	/**
	 * Estimate sigma for a single structure based on its characteristics
	 * @param pStructure The structure to analyze
	 * @param pDose Current dose distribution (can be NULL)
	 * @param pVOITerm The VOI term with prescription info (can be NULL)
	 * @return Estimated base sigma value
	 */
	REAL EstimateStructureSigma(
		Structure* pStructure,
		VolumeReal* pDose = NULL,
		VOITerm* pVOITerm = NULL);

	/**
	 * Adjust sigma based on optimization convergence behavior
	 * @param currentSigma Current sigma value
	 * @param iteration Current iteration number
	 * @param costImprovement Recent cost function improvement
	 * @param gradientNorm Current gradient magnitude
	 * @return Adjusted sigma value
	 */
	REAL AdaptSigmaFromConvergence(
		REAL currentSigma,
		int iteration,
		REAL costImprovement,
		REAL gradientNorm);

	///////////////////////////////////////////////////////////////////////////
	// Individual Estimation Strategies
	///////////////////////////////////////////////////////////////////////////

	/** Estimate sigma from structure geometric complexity */
	REAL EstimateFromGeometry(Structure* pStructure);

	/** Estimate sigma from dose gradient within structure */
	REAL EstimateFromDoseGradient(Structure* pStructure, VolumeReal* pDose);

	/** Estimate sigma from prescription dose range */
	REAL EstimateFromPrescriptionRange(VOITerm* pVOITerm);

	/** Estimate sigma from structure volume (larger volumes → coarser initial sigma) */
	REAL EstimateFromVolume(Structure* pStructure);

	/** Combine multiple estimates using weighted average */
	REAL CombineEstimates(const std::vector<REAL>& estimates,
	                       const std::vector<REAL>& weights);

	///////////////////////////////////////////////////////////////////////////
	// Pyramid Scaling Methods
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Generate pyramid sigma sequence from base sigma
	 * @param baseSigma Finest level sigma
	 * @param nLevels Number of pyramid levels
	 * @param scaleFactor Multiplier per coarser level (default ~2.5)
	 * @return Vector of sigmas from coarsest to finest
	 */
	std::vector<REAL> GeneratePyramidSequence(
		REAL baseSigma,
		int nLevels,
		REAL scaleFactor = 2.5);

	///////////////////////////////////////////////////////////////////////////
	// Helper Functions
	///////////////////////////////////////////////////////////////////////////

	/** Calculate structure sphericity (1.0 = perfect sphere, lower = more complex) */
	REAL CalculateSphericity(Structure* pStructure);

	/** Calculate structure surface-to-volume ratio */
	REAL CalculateSurfaceToVolumeRatio(Structure* pStructure);

	/** Calculate average dose gradient magnitude within structure */
	REAL CalculateAvgDoseGradient(Structure* pStructure, VolumeReal* pDose);

	/** Calculate dose heterogeneity (standard deviation) within structure */
	REAL CalculateDoseHeterogeneity(Structure* pStructure, VolumeReal* pDose);

	/** Estimate structure volume in voxels */
	int EstimateStructureVoxelCount(Structure* pStructure, int level = 0);

	/** Clamp sigma to allowed range */
	REAL ClampSigma(REAL sigma) const;

private:
	// Default sigma values (fallback)
	std::vector<REAL> m_defaultSigmas;

	// Sigma range constraints
	REAL m_minSigma;
	REAL m_maxSigma;

	// Voxel spacing for spatial calculations
	REAL m_spacingX;
	REAL m_spacingY;
	REAL m_spacingZ;

	// Estimation method flags
	bool m_bUseStructureComplexity;
	bool m_bUseDoseGradient;
	bool m_bUsePrescriptionRange;
	bool m_bUseVolumeSize;

	// Adaptive convergence parameters
	REAL m_slowConvergenceThreshold;
	REAL m_fastConvergenceThreshold;
	REAL m_sigmaIncreaseRate;
	REAL m_sigmaDecreaseRate;

	///////////////////////////////////////////////////////////////////////////
	// Internal Helper Methods
	///////////////////////////////////////////////////////////////////////////

	/** Extract dose values within structure region */
	void ExtractStructureDoses(
		Structure* pStructure,
		VolumeReal* pDose,
		std::vector<REAL>& doses,
		int level = 0);

	/** Calculate spatial gradient at a voxel */
	void CalculateVoxelGradient(
		VolumeReal* pDose,
		int x, int y, int z,
		REAL& gradX, REAL& gradY, REAL& gradZ);

	/** Approximate structure volume from region */
	REAL ApproximateVolume(Structure* pStructure, int level = 0);

	/** Approximate structure surface area from region */
	REAL ApproximateSurfaceArea(Structure* pStructure, int level = 0);

};	// class SigmaEstimator

}	// namespace dH
