// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// SigmaEstimator.cpp - Implementation of adaptive sigma estimation
#include "stdafx.h"
#include "SigmaEstimator.h"
#include "KLDivTerm.h"

#include <cmath>
#include <algorithm>
#include <numeric>

using namespace dH;

// Constants for estimation heuristics
namespace {
	const REAL DEFAULT_MIN_SIGMA = 0.1;
	const REAL DEFAULT_MAX_SIGMA = 16.0;
	const REAL DEFAULT_VOXEL_SPACING = 2.0;  // mm
	const REAL PYRAMID_SCALE_FACTOR = 2.5;

	// Thresholds for convergence-based adaptation
	const REAL SLOW_CONVERGENCE_THRESHOLD = 0.001;
	const REAL FAST_CONVERGENCE_THRESHOLD = 0.1;
	const REAL SIGMA_INCREASE_RATE = 1.2;
	const REAL SIGMA_DECREASE_RATE = 0.85;

	// Weights for combined estimation
	const REAL WEIGHT_GEOMETRY = 1.0;
	const REAL WEIGHT_DOSE_GRADIENT = 2.0;
	const REAL WEIGHT_PRESCRIPTION = 1.5;
	const REAL WEIGHT_VOLUME = 0.8;
}

///////////////////////////////////////////////////////////////////////////////
SigmaEstimator::SigmaEstimator()
	: m_minSigma(DEFAULT_MIN_SIGMA)
	, m_maxSigma(DEFAULT_MAX_SIGMA)
	, m_spacingX(DEFAULT_VOXEL_SPACING)
	, m_spacingY(DEFAULT_VOXEL_SPACING)
	, m_spacingZ(DEFAULT_VOXEL_SPACING)
	, m_bUseStructureComplexity(true)
	, m_bUseDoseGradient(true)
	, m_bUsePrescriptionRange(true)
	, m_bUseVolumeSize(true)
	, m_slowConvergenceThreshold(SLOW_CONVERGENCE_THRESHOLD)
	, m_fastConvergenceThreshold(FAST_CONVERGENCE_THRESHOLD)
	, m_sigmaIncreaseRate(SIGMA_INCREASE_RATE)
	, m_sigmaDecreaseRate(SIGMA_DECREASE_RATE)
{
	// Initialize default sigmas matching current Brimstone values
	m_defaultSigmas = {8.0, 3.2, 1.3, 0.5, 0.25};
}

///////////////////////////////////////////////////////////////////////////////
SigmaEstimator::~SigmaEstimator()
{
}

///////////////////////////////////////////////////////////////////////////////
void SigmaEstimator::SetDefaultSigmas(const std::vector<REAL>& defaultSigmas)
{
	m_defaultSigmas = defaultSigmas;
}

///////////////////////////////////////////////////////////////////////////////
void SigmaEstimator::SetSigmaRange(REAL minSigma, REAL maxSigma)
{
	m_minSigma = minSigma;
	m_maxSigma = maxSigma;
}

///////////////////////////////////////////////////////////////////////////////
void SigmaEstimator::SetVoxelSpacing(REAL spacingX, REAL spacingY, REAL spacingZ)
{
	m_spacingX = spacingX;
	m_spacingY = spacingY;
	m_spacingZ = spacingZ;
}

///////////////////////////////////////////////////////////////////////////////
std::vector<REAL> SigmaEstimator::EstimatePyramidSigmas(
	CPlan* pPlan,
	Prescription* pPresc,
	int nLevels)
{
	if (!pPlan || !pPresc) {
		// Return defaults if no plan/prescription
		return m_defaultSigmas;
	}

	// Collect structure-specific sigma estimates
	std::vector<REAL> structureSigmas;
	std::vector<REAL> structureWeights;

	VolumeReal* pDose = pPlan->GetDoseMatrix();

	// Iterate through all VOI terms
	POSITION pos = pPresc->m_mapVOITs.GetStartPosition();
	while (pos != NULL) {
		Structure* pStruct = NULL;
		VOITerm* pVOIT = NULL;
		pPresc->m_mapVOITs.GetNextAssoc(pos, pStruct, pVOIT);

		if (pStruct && pVOIT) {
			// Estimate sigma for this structure
			REAL structSigma = EstimateStructureSigma(pStruct, pDose, pVOIT);
			REAL weight = pVOIT->GetWeight();

			structureSigmas.push_back(structSigma);
			structureWeights.push_back(weight);
		}
	}

	// Combine structure estimates using weighted average
	REAL baseSigma = 0.5;  // Default finest-level sigma
	if (!structureSigmas.empty()) {
		baseSigma = CombineEstimates(structureSigmas, structureWeights);
	}

	// Generate pyramid sequence from base sigma
	return GeneratePyramidSequence(baseSigma, nLevels);
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::EstimateStructureSigma(
	Structure* pStructure,
	VolumeReal* pDose,
	VOITerm* pVOITerm)
{
	if (!pStructure) {
		return 0.5;  // Default
	}

	std::vector<REAL> estimates;
	std::vector<REAL> weights;

	// 1. Geometric complexity estimate
	if (m_bUseStructureComplexity) {
		REAL geomSigma = EstimateFromGeometry(pStructure);
		if (geomSigma > 0) {
			estimates.push_back(geomSigma);
			weights.push_back(WEIGHT_GEOMETRY);
		}
	}

	// 2. Dose gradient estimate (if dose available)
	if (m_bUseDoseGradient && pDose) {
		REAL gradSigma = EstimateFromDoseGradient(pStructure, pDose);
		if (gradSigma > 0) {
			estimates.push_back(gradSigma);
			weights.push_back(WEIGHT_DOSE_GRADIENT);
		}
	}

	// 3. Prescription range estimate (if VOI term available)
	if (m_bUsePrescriptionRange && pVOITerm) {
		REAL prescSigma = EstimateFromPrescriptionRange(pVOITerm);
		if (prescSigma > 0) {
			estimates.push_back(prescSigma);
			weights.push_back(WEIGHT_PRESCRIPTION);
		}
	}

	// 4. Volume-based estimate
	if (m_bUseVolumeSize) {
		REAL volSigma = EstimateFromVolume(pStructure);
		if (volSigma > 0) {
			estimates.push_back(volSigma);
			weights.push_back(WEIGHT_VOLUME);
		}
	}

	// Combine estimates
	if (estimates.empty()) {
		return 0.5;  // Default
	}

	return CombineEstimates(estimates, weights);
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::EstimateFromGeometry(Structure* pStructure)
{
	if (!pStructure) return -1.0;

	// Calculate geometric complexity metrics
	REAL sphericity = CalculateSphericity(pStructure);
	REAL svRatio = CalculateSurfaceToVolumeRatio(pStructure);

	// Complex structures (low sphericity, high SV ratio) need finer resolution
	// Simple structures can use coarser resolution

	// Sphericity: 1.0 (sphere) → can use larger sigma
	//             0.0 (complex) → need smaller sigma
	REAL complexityFromShape = 1.0 - sphericity;

	// Surface-to-volume ratio (normalized)
	// Higher ratio → more complex boundary → finer sigma needed
	REAL avgSVRatio = 1.0;  // Approximate typical value
	REAL complexityFromSV = svRatio / avgSVRatio;

	// Combined complexity score
	REAL complexity = 0.6 * complexityFromShape + 0.4 * complexityFromSV;
	complexity = std::max(0.0, std::min(1.0, complexity));  // Clamp to [0,1]

	// Map complexity to sigma: complex → small sigma, simple → large sigma
	// Use exponential mapping for better scale distribution
	REAL baseSigma = 0.5;
	REAL complexityFactor = 1.0 - 0.7 * complexity;  // Range [0.3, 1.0]
	REAL sigma = baseSigma * complexityFactor;

	return ClampSigma(sigma);
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::EstimateFromDoseGradient(Structure* pStructure, VolumeReal* pDose)
{
	if (!pStructure || !pDose) return -1.0;

	// Calculate average dose gradient magnitude
	REAL avgGradient = CalculateAvgDoseGradient(pStructure, pDose);

	// Also calculate dose heterogeneity
	REAL doseStdDev = CalculateDoseHeterogeneity(pStructure, pDose);

	// High gradient → need fine resolution → small sigma
	// Low gradient → can use coarse resolution → large sigma

	// Normalize gradient (typical range ~0-10 Gy/mm)
	REAL normalizedGrad = avgGradient / 5.0;
	normalizedGrad = std::min(normalizedGrad, 2.0);  // Cap at 2x typical

	// Normalize heterogeneity (typical range ~0-10 Gy)
	REAL normalizedHet = doseStdDev / 5.0;
	normalizedHet = std::min(normalizedHet, 2.0);

	// Combine gradient and heterogeneity
	REAL doseComplexity = 0.7 * normalizedGrad + 0.3 * normalizedHet;

	// Map to sigma using inverse relationship
	REAL baseSigma = 0.5;
	REAL sigma = baseSigma / (1.0 + doseComplexity);

	return ClampSigma(sigma);
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::EstimateFromPrescriptionRange(VOITerm* pVOITerm)
{
	if (!pVOITerm) return -1.0;

	// Try to cast to KLDivTerm to access dose range
	KLDivTerm* pKLTerm = dynamic_cast<KLDivTerm*>(pVOITerm);
	if (!pKLTerm) return -1.0;

	// Get prescription dose range
	REAL minDose = pKLTerm->GetMinDose();
	REAL maxDose = pKLTerm->GetMaxDose();
	REAL doseRange = maxDose - minDose;

	// Wide dose range → more complex optimization → finer sigma
	// Narrow range → simpler → coarser sigma acceptable

	// Normalize dose range (typical range ~20-80 Gy)
	REAL normalizedRange = doseRange / 50.0;
	normalizedRange = std::min(normalizedRange, 2.0);

	// Map to sigma
	REAL baseSigma = 0.5;
	REAL sigma = baseSigma / (0.5 + 0.5 * normalizedRange);

	return ClampSigma(sigma);
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::EstimateFromVolume(Structure* pStructure)
{
	if (!pStructure) return -1.0;

	// Estimate structure volume
	REAL volume = ApproximateVolume(pStructure);  // cc

	// Larger volumes can tolerate coarser initial resolution
	// Smaller volumes need finer resolution

	// Log scale for volume (typical range 1-1000 cc)
	REAL logVolume = log10(std::max(volume, 1.0));  // Range ~0-3

	// Map to sigma: small volume → fine, large volume → coarse
	REAL baseSigma = 0.5;
	REAL volumeFactor = 0.5 + 0.25 * logVolume;  // Range [0.5, 1.25]
	REAL sigma = baseSigma * volumeFactor;

	return ClampSigma(sigma);
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::CombineEstimates(
	const std::vector<REAL>& estimates,
	const std::vector<REAL>& weights)
{
	if (estimates.empty()) return 0.5;

	REAL weightSum = 0.0;
	REAL weightedSum = 0.0;

	for (size_t i = 0; i < estimates.size(); ++i) {
		REAL weight = (i < weights.size()) ? weights[i] : 1.0;
		weightedSum += estimates[i] * weight;
		weightSum += weight;
	}

	if (weightSum > 0) {
		return weightedSum / weightSum;
	}
	return estimates[0];
}

///////////////////////////////////////////////////////////////////////////////
std::vector<REAL> SigmaEstimator::GeneratePyramidSequence(
	REAL baseSigma,
	int nLevels,
	REAL scaleFactor)
{
	std::vector<REAL> sigmas(nLevels);

	// Generate from finest (baseSigma) to coarsest
	// sigmas[0] = coarsest, sigmas[nLevels-1] = finest
	sigmas[nLevels - 1] = baseSigma;

	for (int i = nLevels - 2; i >= 0; --i) {
		sigmas[i] = sigmas[i + 1] * scaleFactor;
		sigmas[i] = ClampSigma(sigmas[i]);
	}

	return sigmas;
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::AdaptSigmaFromConvergence(
	REAL currentSigma,
	int iteration,
	REAL costImprovement,
	REAL gradientNorm)
{
	REAL adaptedSigma = currentSigma;

	// If converging too slowly, increase sigma to escape local minimum
	if (costImprovement < m_slowConvergenceThreshold && gradientNorm < 0.01) {
		adaptedSigma *= m_sigmaIncreaseRate;
	}
	// If converging rapidly, decrease sigma for better precision
	else if (costImprovement > m_fastConvergenceThreshold && gradientNorm > 0.1) {
		adaptedSigma *= m_sigmaDecreaseRate;
	}

	return ClampSigma(adaptedSigma);
}

///////////////////////////////////////////////////////////////////////////////
// Helper Functions
///////////////////////////////////////////////////////////////////////////////

REAL SigmaEstimator::CalculateSphericity(Structure* pStructure)
{
	if (!pStructure) return 0.5;

	REAL volume = ApproximateVolume(pStructure);
	REAL surfaceArea = ApproximateSurfaceArea(pStructure);

	if (volume <= 0 || surfaceArea <= 0) return 0.5;

	// Sphericity = (π^(1/3) * (6*V)^(2/3)) / A
	// For perfect sphere, sphericity = 1.0
	const REAL PI = 3.14159265358979323846;
	REAL sphericity = pow(PI, 1.0/3.0) * pow(6.0 * volume, 2.0/3.0) / surfaceArea;

	// Clamp to [0, 1]
	return std::max(0.0, std::min(1.0, sphericity));
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::CalculateSurfaceToVolumeRatio(Structure* pStructure)
{
	if (!pStructure) return 1.0;

	REAL volume = ApproximateVolume(pStructure);
	REAL surfaceArea = ApproximateSurfaceArea(pStructure);

	if (volume <= 0) return 1.0;

	return surfaceArea / volume;
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::CalculateAvgDoseGradient(Structure* pStructure, VolumeReal* pDose)
{
	if (!pStructure || !pDose) return 0.0;

	std::vector<REAL> gradientMagnitudes;

	// Get structure region at finest level
	const VolumeReal* pRegion = pStructure->GetRegion(0);
	if (!pRegion) return 0.0;

	// Get region iterator
	typedef itk::ImageRegionConstIteratorWithIndex<VolumeReal> RegionIterator;
	RegionIterator regionIt(pRegion, pRegion->GetLargestPossibleRegion());

	// Get dose iterator
	typedef itk::ImageRegionConstIteratorWithIndex<VolumeReal> DoseIterator;
	DoseIterator doseIt(pDose, pDose->GetLargestPossibleRegion());

	// Iterate through voxels in structure
	for (regionIt.GoToBegin(); !regionIt.IsAtEnd(); ++regionIt) {
		REAL regionValue = regionIt.Get();
		if (regionValue > 0.5) {  // Inside structure
			VolumeReal::IndexType idx = regionIt.GetIndex();

			// Calculate gradient at this voxel
			REAL gradX = 0, gradY = 0, gradZ = 0;
			CalculateVoxelGradient(pDose, idx[0], idx[1], idx[2], gradX, gradY, gradZ);

			// Gradient magnitude
			REAL gradMag = sqrt(gradX*gradX + gradY*gradY + gradZ*gradZ);
			gradientMagnitudes.push_back(gradMag);
		}
	}

	// Calculate average
	if (gradientMagnitudes.empty()) return 0.0;

	REAL sum = std::accumulate(gradientMagnitudes.begin(), gradientMagnitudes.end(), 0.0);
	return sum / gradientMagnitudes.size();
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::CalculateDoseHeterogeneity(Structure* pStructure, VolumeReal* pDose)
{
	if (!pStructure || !pDose) return 0.0;

	std::vector<REAL> doses;
	ExtractStructureDoses(pStructure, pDose, doses);

	if (doses.size() < 2) return 0.0;

	// Calculate mean
	REAL mean = std::accumulate(doses.begin(), doses.end(), 0.0) / doses.size();

	// Calculate standard deviation
	REAL variance = 0.0;
	for (REAL dose : doses) {
		variance += (dose - mean) * (dose - mean);
	}
	variance /= doses.size();

	return sqrt(variance);
}

///////////////////////////////////////////////////////////////////////////////
int SigmaEstimator::EstimateStructureVoxelCount(Structure* pStructure, int level)
{
	if (!pStructure) return 0;

	const VolumeReal* pRegion = pStructure->GetRegion(level);
	if (!pRegion) return 0;

	// Count voxels with region value > 0.5
	typedef itk::ImageRegionConstIterator<VolumeReal> Iterator;
	Iterator it(pRegion, pRegion->GetLargestPossibleRegion());

	int count = 0;
	for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
		if (it.Get() > 0.5) {
			++count;
		}
	}

	return count;
}

///////////////////////////////////////////////////////////////////////////////
void SigmaEstimator::ExtractStructureDoses(
	Structure* pStructure,
	VolumeReal* pDose,
	std::vector<REAL>& doses,
	int level)
{
	doses.clear();

	if (!pStructure || !pDose) return;

	const VolumeReal* pRegion = pStructure->GetRegion(level);
	if (!pRegion) return;

	// Iterate through region and dose simultaneously
	typedef itk::ImageRegionConstIterator<VolumeReal> Iterator;
	Iterator regionIt(pRegion, pRegion->GetLargestPossibleRegion());
	Iterator doseIt(pDose, pDose->GetLargestPossibleRegion());

	for (regionIt.GoToBegin(), doseIt.GoToBegin();
	     !regionIt.IsAtEnd() && !doseIt.IsAtEnd();
	     ++regionIt, ++doseIt) {
		if (regionIt.Get() > 0.5) {  // Inside structure
			doses.push_back(doseIt.Get());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void SigmaEstimator::CalculateVoxelGradient(
	VolumeReal* pDose,
	int x, int y, int z,
	REAL& gradX, REAL& gradY, REAL& gradZ)
{
	gradX = gradY = gradZ = 0.0;

	if (!pDose) return;

	VolumeReal::IndexType idx;
	VolumeReal::SizeType size = pDose->GetLargestPossibleRegion().GetSize();

	// X gradient (central difference)
	if (x > 0 && x < (int)size[0] - 1) {
		idx[0] = x + 1; idx[1] = y; idx[2] = z;
		REAL doseXPlus = pDose->GetPixel(idx);
		idx[0] = x - 1;
		REAL doseXMinus = pDose->GetPixel(idx);
		gradX = (doseXPlus - doseXMinus) / (2.0 * m_spacingX);
	}

	// Y gradient
	if (y > 0 && y < (int)size[1] - 1) {
		idx[0] = x; idx[1] = y + 1; idx[2] = z;
		REAL doseYPlus = pDose->GetPixel(idx);
		idx[1] = y - 1;
		REAL doseYMinus = pDose->GetPixel(idx);
		gradY = (doseYPlus - doseYMinus) / (2.0 * m_spacingY);
	}

	// Z gradient
	if (z > 0 && z < (int)size[2] - 1) {
		idx[0] = x; idx[1] = y; idx[2] = z + 1;
		REAL doseZPlus = pDose->GetPixel(idx);
		idx[2] = z - 1;
		REAL doseZMinus = pDose->GetPixel(idx);
		gradZ = (doseZPlus - doseZMinus) / (2.0 * m_spacingZ);
	}
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::ApproximateVolume(Structure* pStructure, int level)
{
	if (!pStructure) return 0.0;

	const VolumeReal* pRegion = pStructure->GetRegion(level);
	if (!pRegion) return 0.0;

	// Count voxels and multiply by voxel volume
	int voxelCount = EstimateStructureVoxelCount(pStructure, level);

	// Get voxel spacing
	VolumeReal::SpacingType spacing = pRegion->GetSpacing();
	REAL voxelVolume = spacing[0] * spacing[1] * spacing[2];  // mm^3

	// Convert to cc (cm^3)
	REAL volumeCC = (voxelCount * voxelVolume) / 1000.0;

	return volumeCC;
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::ApproximateSurfaceArea(Structure* pStructure, int level)
{
	if (!pStructure) return 0.0;

	const VolumeReal* pRegion = pStructure->GetRegion(level);
	if (!pRegion) return 0.0;

	// Get voxel spacing
	VolumeReal::SpacingType spacing = pRegion->GetSpacing();

	// Count boundary voxels (voxels with at least one neighbor outside structure)
	typedef itk::ImageRegionConstIteratorWithIndex<VolumeReal> Iterator;
	Iterator it(pRegion, pRegion->GetLargestPossibleRegion());

	int boundaryCount = 0;
	VolumeReal::SizeType size = pRegion->GetLargestPossibleRegion().GetSize();

	for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
		if (it.Get() > 0.5) {  // Inside structure
			VolumeReal::IndexType idx = it.GetIndex();
			bool isBoundary = false;

			// Check 6-connected neighbors
			for (int d = 0; d < 3; ++d) {
				for (int delta = -1; delta <= 1; delta += 2) {
					VolumeReal::IndexType neighborIdx = idx;
					neighborIdx[d] += delta;

					// Check bounds
					if (neighborIdx[d] >= 0 && neighborIdx[d] < (long)size[d]) {
						REAL neighborValue = pRegion->GetPixel(neighborIdx);
						if (neighborValue < 0.5) {  // Neighbor outside structure
							isBoundary = true;
							break;
						}
					} else {
						isBoundary = true;  // Edge of volume
						break;
					}
				}
				if (isBoundary) break;
			}

			if (isBoundary) {
				++boundaryCount;
			}
		}
	}

	// Approximate surface area (rough estimate)
	// Each boundary voxel contributes approximately one face area
	REAL avgFaceArea = (spacing[0] * spacing[1] + spacing[1] * spacing[2] +
	                    spacing[0] * spacing[2]) / 3.0;  // mm^2

	// Convert to cm^2
	REAL surfaceAreaCM2 = (boundaryCount * avgFaceArea) / 100.0;

	return surfaceAreaCM2;
}

///////////////////////////////////////////////////////////////////////////////
REAL SigmaEstimator::ClampSigma(REAL sigma) const
{
	return std::max(m_minSigma, std::min(m_maxSigma, sigma));
}
