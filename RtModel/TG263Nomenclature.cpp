// Copyright (C) 2nd Messenger Systems
// TG263Nomenclature.cpp - Implementation of TG-263 nomenclature translator

#include "stdafx.h"
#include "TG263Nomenclature.h"
#include <sstream>
#include <cctype>
#include <cmath>

using namespace dH;

///////////////////////////////////////////////////////////////////////////////
TG263Nomenclature::TG263Nomenclature()
{
	InitializeStructures();
}

///////////////////////////////////////////////////////////////////////////////
TG263Nomenclature::~TG263Nomenclature()
{
}

///////////////////////////////////////////////////////////////////////////////
// Primary Translation Methods
///////////////////////////////////////////////////////////////////////////////

TG263Match TG263Nomenclature::TranslateToStandard(
	const std::string& clinicalName,
	float minConfidence) const
{
	// Try exact match first
	TG263Match exactMatch = ExactMatch(clinicalName);
	if (exactMatch.confidence >= minConfidence) {
		return exactMatch;
	}

	// Try fuzzy match
	std::vector<TG263Match> fuzzyMatches = FuzzyMatch(clinicalName, 1);
	if (!fuzzyMatches.empty() && fuzzyMatches[0].confidence >= minConfidence) {
		return fuzzyMatches[0];
	}

	// No good match found
	return TG263Match();
}

///////////////////////////////////////////////////////////////////////////////
std::vector<TG263Match> TG263Nomenclature::GetAllMatches(
	const std::string& clinicalName,
	int maxResults) const
{
	std::vector<TG263Match> matches;

	// Try exact match first
	TG263Match exactMatch = ExactMatch(clinicalName);
	if (exactMatch.confidence > 0.0f) {
		matches.push_back(exactMatch);
	}

	// Add fuzzy matches
	std::vector<TG263Match> fuzzyMatches = FuzzyMatch(clinicalName, maxResults);
	for (const auto& match : fuzzyMatches) {
		// Avoid duplicates from exact match
		if (matches.empty() || match.standardName != matches[0].standardName) {
			matches.push_back(match);
		}
	}

	// Limit results
	if (matches.size() > static_cast<size_t>(maxResults)) {
		matches.resize(maxResults);
	}

	return matches;
}

///////////////////////////////////////////////////////////////////////////////
bool TG263Nomenclature::IsStandardName(const std::string& name) const
{
	return m_structures.find(name) != m_structures.end();
}

///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> TG263Nomenclature::GetAliases(
	const std::string& standardName) const
{
	auto it = m_structures.find(standardName);
	if (it != m_structures.end()) {
		return it->second.aliases;
	}
	return std::vector<std::string>();
}

///////////////////////////////////////////////////////////////////////////////
std::vector<TG263Match> TG263Nomenclature::TranslateBatch(
	const std::vector<std::string>& clinicalNames,
	float minConfidence) const
{
	std::vector<TG263Match> results;
	results.reserve(clinicalNames.size());

	for (const auto& name : clinicalNames) {
		results.push_back(TranslateToStandard(name, minConfidence));
	}

	return results;
}

///////////////////////////////////////////////////////////////////////////////
// Query Methods
///////////////////////////////////////////////////////////////////////////////

std::vector<std::string> TG263Nomenclature::GetStructuresByCategory(
	TG263Category category) const
{
	auto it = m_categoryMap.find(category);
	if (it != m_categoryMap.end()) {
		return it->second;
	}
	return std::vector<std::string>();
}

///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> TG263Nomenclature::GetStructuresBySite(
	AnatomicalSite site) const
{
	auto it = m_siteMap.find(site);
	if (it != m_siteMap.end()) {
		return it->second;
	}
	return std::vector<std::string>();
}

///////////////////////////////////////////////////////////////////////////////
const TG263Structure* TG263Nomenclature::GetStructureDefinition(
	const std::string& standardName) const
{
	auto it = m_structures.find(standardName);
	if (it != m_structures.end()) {
		return &it->second;
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> TG263Nomenclature::GetAllStandardNames() const
{
	std::vector<std::string> names;
	names.reserve(m_structures.size());

	for (const auto& pair : m_structures) {
		names.push_back(pair.first);
	}

	return names;
}

///////////////////////////////////////////////////////////////////////////////
int TG263Nomenclature::GetStructureCount() const
{
	return static_cast<int>(m_structures.size());
}

///////////////////////////////////////////////////////////////////////////////
int TG263Nomenclature::GetAliasCount() const
{
	return static_cast<int>(m_aliasMap.size());
}

///////////////////////////////////////////////////////////////////////////////
// Enum to String Conversions
///////////////////////////////////////////////////////////////////////////////

std::string TG263Nomenclature::CategoryToString(TG263Category category)
{
	switch (category) {
		case TG263Category::TARGET: return "Target";
		case TG263Category::OAR: return "Organ at Risk";
		case TG263Category::NORMAL_TISSUE: return "Normal Tissue";
		case TG263Category::LYMPH_NODE: return "Lymph Node";
		case TG263Category::CAVITY: return "Cavity";
		case TG263Category::BONE: return "Bone";
		case TG263Category::MARKER: return "Marker";
		case TG263Category::EXTERNAL: return "External";
		case TG263Category::AVOIDANCE: return "Avoidance";
		case TG263Category::DOSE_REGION: return "Dose Region";
		default: return "Unknown";
	}
}

///////////////////////////////////////////////////////////////////////////////
std::string TG263Nomenclature::SiteToString(AnatomicalSite site)
{
	switch (site) {
		case AnatomicalSite::BRAIN: return "Brain";
		case AnatomicalSite::HEAD_NECK: return "Head & Neck";
		case AnatomicalSite::THORAX: return "Thorax";
		case AnatomicalSite::ABDOMEN: return "Abdomen";
		case AnatomicalSite::PELVIS: return "Pelvis";
		case AnatomicalSite::EXTREMITY: return "Extremity";
		case AnatomicalSite::SPINE: return "Spine";
		case AnatomicalSite::WHOLE_BODY: return "Whole Body";
		default: return "Unknown";
	}
}

///////////////////////////////////////////////////////////////////////////////
std::string TG263Nomenclature::LateralityToString(Laterality laterality)
{
	switch (laterality) {
		case Laterality::LEFT: return "Left";
		case Laterality::RIGHT: return "Right";
		case Laterality::BILATERAL: return "Bilateral";
		default: return "None";
	}
}

///////////////////////////////////////////////////////////////////////////////
// Matching Algorithms
///////////////////////////////////////////////////////////////////////////////

TG263Match TG263Nomenclature::ExactMatch(const std::string& name) const
{
	TG263Match match;

	// Normalize input
	std::string normalized = NormalizeName(name);

	// Check alias map
	auto it = m_aliasMap.find(normalized);
	if (it != m_aliasMap.end()) {
		const std::string& standardName = it->second;
		auto structIt = m_structures.find(standardName);
		if (structIt != m_structures.end()) {
			const TG263Structure& structure = structIt->second;

			match.standardName = structure.standardName;
			match.category = structure.category;
			match.site = structure.site;
			match.laterality = structure.laterality;
			match.confidence = 1.0f;  // Exact match
			match.matchedAlias = name;

			return match;
		}
	}

	match.confidence = 0.0f;
	return match;
}

///////////////////////////////////////////////////////////////////////////////
std::vector<TG263Match> TG263Nomenclature::FuzzyMatch(
	const std::string& name,
	int maxResults) const
{
	std::vector<TG263Match> matches;

	std::string normalizedInput = NormalizeName(name);

	// Calculate similarity to all structures
	for (const auto& pair : m_structures) {
		const TG263Structure& structure = pair.second;

		float bestConfidence = 0.0f;
		std::string bestMatchedAlias;

		// Check standard name
		std::string normalizedStandard = NormalizeName(structure.standardName);
		float confidence = CalculateConfidence(normalizedInput,
		                                        normalizedStandard,
		                                        structure.standardName);
		if (confidence > bestConfidence) {
			bestConfidence = confidence;
			bestMatchedAlias = structure.standardName;
		}

		// Check all aliases
		for (const auto& alias : structure.aliases) {
			std::string normalizedAlias = NormalizeName(alias);
			float aliasConfidence = CalculateConfidence(normalizedInput,
			                                             normalizedAlias,
			                                             alias);
			if (aliasConfidence > bestConfidence) {
				bestConfidence = aliasConfidence;
				bestMatchedAlias = alias;
			}
		}

		// Create match if confidence is reasonable
		if (bestConfidence > 0.3f) {
			TG263Match match;
			match.standardName = structure.standardName;
			match.category = structure.category;
			match.site = structure.site;
			match.laterality = structure.laterality;
			match.confidence = bestConfidence;
			match.matchedAlias = bestMatchedAlias;

			matches.push_back(match);
		}
	}

	// Sort by confidence (highest first)
	std::sort(matches.begin(), matches.end(),
		[](const TG263Match& a, const TG263Match& b) {
			return a.confidence > b.confidence;
		});

	// Limit results
	if (matches.size() > static_cast<size_t>(maxResults)) {
		matches.resize(maxResults);
	}

	return matches;
}

///////////////////////////////////////////////////////////////////////////////
float TG263Nomenclature::CalculateConfidence(
	const std::string& clinicalName,
	const std::string& standardName,
	const std::string& matchedAlias) const
{
	// Combined metric using edit distance and token similarity
	float editSimilarity = 0.0f;
	int maxLen = std::max(clinicalName.length(), standardName.length());
	if (maxLen > 0) {
		int distance = EditDistance(clinicalName, standardName);
		editSimilarity = 1.0f - (static_cast<float>(distance) / maxLen);
	}

	float tokenSim = TokenSimilarity(clinicalName, standardName);

	// Weighted combination
	float confidence = 0.4f * editSimilarity + 0.6f * tokenSim;

	// Boost confidence if laterality matches
	Laterality inputLat = DetectLaterality(clinicalName);
	Laterality standardLat = DetectLaterality(standardName);
	if (inputLat != Laterality::NONE && inputLat == standardLat) {
		confidence = std::min(1.0f, confidence * 1.1f);
	}

	return confidence;
}

///////////////////////////////////////////////////////////////////////////////
std::string TG263Nomenclature::NormalizeName(const std::string& name) const
{
	std::string normalized;
	normalized.reserve(name.length());

	for (char c : name) {
		if (std::isalnum(c)) {
			normalized += std::tolower(c);
		} else if (c == ' ' || c == '_' || c == '-') {
			normalized += ' ';
		}
	}

	// Trim whitespace
	size_t start = normalized.find_first_not_of(" ");
	size_t end = normalized.find_last_not_of(" ");
	if (start != std::string::npos && end != std::string::npos) {
		return normalized.substr(start, end - start + 1);
	}

	return normalized;
}

///////////////////////////////////////////////////////////////////////////////
int TG263Nomenclature::EditDistance(
	const std::string& s1,
	const std::string& s2) const
{
	// Levenshtein distance using dynamic programming
	std::vector<std::vector<int>> dp(s1.length() + 1,
	                                  std::vector<int>(s2.length() + 1));

	for (size_t i = 0; i <= s1.length(); i++) {
		dp[i][0] = static_cast<int>(i);
	}
	for (size_t j = 0; j <= s2.length(); j++) {
		dp[0][j] = static_cast<int>(j);
	}

	for (size_t i = 1; i <= s1.length(); i++) {
		for (size_t j = 1; j <= s2.length(); j++) {
			int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
			dp[i][j] = std::min({
				dp[i-1][j] + 1,      // deletion
				dp[i][j-1] + 1,      // insertion
				dp[i-1][j-1] + cost  // substitution
			});
		}
	}

	return dp[s1.length()][s2.length()];
}

///////////////////////////////////////////////////////////////////////////////
float TG263Nomenclature::TokenSimilarity(
	const std::string& s1,
	const std::string& s2) const
{
	// Tokenize both strings
	auto tokenize = [](const std::string& s) -> std::set<std::string> {
		std::set<std::string> tokens;
		std::istringstream iss(s);
		std::string token;
		while (iss >> token) {
			tokens.insert(token);
		}
		return tokens;
	};

	std::set<std::string> tokens1 = tokenize(s1);
	std::set<std::string> tokens2 = tokenize(s2);

	if (tokens1.empty() && tokens2.empty()) {
		return 1.0f;
	}

	// Jaccard similarity: |intersection| / |union|
	std::set<std::string> intersection;
	std::set_intersection(tokens1.begin(), tokens1.end(),
	                      tokens2.begin(), tokens2.end(),
	                      std::inserter(intersection, intersection.begin()));

	std::set<std::string> unionSet;
	std::set_union(tokens1.begin(), tokens1.end(),
	               tokens2.begin(), tokens2.end(),
	               std::inserter(unionSet, unionSet.begin()));

	if (unionSet.empty()) {
		return 0.0f;
	}

	return static_cast<float>(intersection.size()) / unionSet.size();
}

///////////////////////////////////////////////////////////////////////////////
Laterality TG263Nomenclature::DetectLaterality(const std::string& name) const
{
	std::string lower = NormalizeName(name);

	// Check for left indicators
	if (lower.find("left") != std::string::npos ||
	    lower.find(" l ") != std::string::npos ||
	    lower.find("_l") != std::string::npos ||
	    lower.find("lt") != std::string::npos) {
		return Laterality::LEFT;
	}

	// Check for right indicators
	if (lower.find("right") != std::string::npos ||
	    lower.find(" r ") != std::string::npos ||
	    lower.find("_r") != std::string::npos ||
	    lower.find("rt") != std::string::npos) {
		return Laterality::RIGHT;
	}

	// Check for bilateral
	if (lower.find("bilateral") != std::string::npos ||
	    lower.find("both") != std::string::npos) {
		return Laterality::BILATERAL;
	}

	return Laterality::NONE;
}

///////////////////////////////////////////////////////////////////////////////
// Structure Registration
///////////////////////////////////////////////////////////////////////////////

void TG263Nomenclature::RegisterStructure(const TG263Structure& structure)
{
	// Add to main map
	m_structures[structure.standardName] = structure;

	// Add standard name to alias map
	std::string normalizedStandard = NormalizeName(structure.standardName);
	m_aliasMap[normalizedStandard] = structure.standardName;

	// Add all aliases
	for (const auto& alias : structure.aliases) {
		std::string normalizedAlias = NormalizeName(alias);
		m_aliasMap[normalizedAlias] = structure.standardName;
	}

	// Add to category map
	m_categoryMap[structure.category].push_back(structure.standardName);

	// Add to site map
	m_siteMap[structure.site].push_back(structure.standardName);
}

///////////////////////////////////////////////////////////////////////////////
// Structure Database Initialization
///////////////////////////////////////////////////////////////////////////////

void TG263Nomenclature::InitializeStructures()
{
	LoadBrainStructures();
	LoadHeadNeckStructures();
	LoadThoraxStructures();
	LoadAbdomenStructures();
	LoadPelvisStructures();
	LoadSpineStructures();
	LoadCommonTargets();
	LoadCommonOARs();
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadBrainStructures()
{
	// Brain/CNS structures
	RegisterStructure(TG263Structure(
		"Brain",
		{"whole brain", "brain stem", "cerebrum", "encephalon"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN
	));

	RegisterStructure(TG263Structure(
		"BrachialPlex_L",
		{"left brachial plexus", "brachial plexus left", "BP left"},
		TG263Category::OAR,
		AnatomicalSite::HEAD_NECK,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"BrachialPlex_R",
		{"right brachial plexus", "brachial plexus right", "BP right"},
		TG263Category::OAR,
		AnatomicalSite::HEAD_NECK,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"Brainstem",
		{"brain stem", "BS"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN
	));

	RegisterStructure(TG263Structure(
		"Chiasm",
		{"optic chiasm", "chiasma"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN
	));

	RegisterStructure(TG263Structure(
		"Cochlea_L",
		{"left cochlea", "cochlea left", "inner ear left"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"Cochlea_R",
		{"right cochlea", "cochlea right", "inner ear right"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"Lens_L",
		{"left lens", "lens left", "LT lens"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"Lens_R",
		{"right lens", "lens right", "RT lens"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"OpticNrv_L",
		{"left optic nerve", "optic nerve left", "ON left"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"OpticNrv_R",
		{"right optic nerve", "optic nerve right", "ON right"},
		TG263Category::OAR,
		AnatomicalSite::BRAIN,
		Laterality::RIGHT
	));
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadHeadNeckStructures()
{
	RegisterStructure(TG263Structure(
		"Parotid_L",
		{"left parotid", "parotid left", "LT parotid", "parotid gland left"},
		TG263Category::OAR,
		AnatomicalSite::HEAD_NECK,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"Parotid_R",
		{"right parotid", "parotid right", "RT parotid", "parotid gland right"},
		TG263Category::OAR,
		AnatomicalSite::HEAD_NECK,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"Submand_L",
		{"left submandibular", "submandibular left", "submandibular gland left"},
		TG263Category::OAR,
		AnatomicalSite::HEAD_NECK,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"Submand_R",
		{"right submandibular", "submandibular right", "submandibular gland right"},
		TG263Category::OAR,
		AnatomicalSite::HEAD_NECK,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"SpinalCord",
		{"spinal cord", "cord", "SC"},
		TG263Category::OAR,
		AnatomicalSite::SPINE
	));

	RegisterStructure(TG263Structure(
		"Esophagus",
		{"esophagus", "oesophagus", "eso"},
		TG263Category::OAR,
		AnatomicalSite::THORAX
	));

	RegisterStructure(TG263Structure(
		"Larynx",
		{"voice box", "glottis"},
		TG263Category::OAR,
		AnatomicalSite::HEAD_NECK
	));

	RegisterStructure(TG263Structure(
		"Mandible",
		{"jaw", "lower jaw", "mandibular bone"},
		TG263Category::BONE,
		AnatomicalSite::HEAD_NECK
	));
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadThoraxStructures()
{
	RegisterStructure(TG263Structure(
		"Heart",
		{"whole heart", "cardiac"},
		TG263Category::OAR,
		AnatomicalSite::THORAX
	));

	RegisterStructure(TG263Structure(
		"Lung_L",
		{"left lung", "lung left", "LT lung"},
		TG263Category::OAR,
		AnatomicalSite::THORAX,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"Lung_R",
		{"right lung", "lung right", "RT lung"},
		TG263Category::OAR,
		AnatomicalSite::THORAX,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"Lungs",
		{"both lungs", "bilateral lungs", "total lung"},
		TG263Category::OAR,
		AnatomicalSite::THORAX,
		Laterality::BILATERAL
	));

	RegisterStructure(TG263Structure(
		"Trachea",
		{"windpipe"},
		TG263Category::OAR,
		AnatomicalSite::THORAX
	));
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadAbdomenStructures()
{
	RegisterStructure(TG263Structure(
		"Liver",
		{"whole liver", "hepatic"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN
	));

	RegisterStructure(TG263Structure(
		"Stomach",
		{"gastric"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN
	));

	RegisterStructure(TG263Structure(
		"Kidney_L",
		{"left kidney", "kidney left", "LT kidney"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"Kidney_R",
		{"right kidney", "kidney right", "RT kidney"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"Kidneys",
		{"both kidneys", "bilateral kidneys"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN,
		Laterality::BILATERAL
	));

	RegisterStructure(TG263Structure(
		"Spleen",
		{"splenic"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN
	));

	RegisterStructure(TG263Structure(
		"Bowel_Small",
		{"small bowel", "small intestine", "SB"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN
	));

	RegisterStructure(TG263Structure(
		"Bowel_Large",
		{"large bowel", "colon", "large intestine"},
		TG263Category::OAR,
		AnatomicalSite::ABDOMEN
	));
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadPelvisStructures()
{
	RegisterStructure(TG263Structure(
		"Bladder",
		{"urinary bladder", "UB"},
		TG263Category::OAR,
		AnatomicalSite::PELVIS
	));

	RegisterStructure(TG263Structure(
		"Rectum",
		{"rectal"},
		TG263Category::OAR,
		AnatomicalSite::PELVIS
	));

	RegisterStructure(TG263Structure(
		"FemoralHead_L",
		{"left femoral head", "femoral head left", "LT femur head"},
		TG263Category::BONE,
		AnatomicalSite::PELVIS,
		Laterality::LEFT
	));

	RegisterStructure(TG263Structure(
		"FemoralHead_R",
		{"right femoral head", "femoral head right", "RT femur head"},
		TG263Category::BONE,
		AnatomicalSite::PELVIS,
		Laterality::RIGHT
	));

	RegisterStructure(TG263Structure(
		"Prostate",
		{"prostate gland"},
		TG263Category::OAR,
		AnatomicalSite::PELVIS
	));

	RegisterStructure(TG263Structure(
		"Uterus",
		{"womb"},
		TG263Category::OAR,
		AnatomicalSite::PELVIS
	));

	RegisterStructure(TG263Structure(
		"PenileBulb",
		{"bulb of penis", "penile bulb"},
		TG263Category::OAR,
		AnatomicalSite::PELVIS
	));
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadSpineStructures()
{
	RegisterStructure(TG263Structure(
		"SpinalCanal",
		{"spinal canal", "thecal sac"},
		TG263Category::OAR,
		AnatomicalSite::SPINE
	));

	RegisterStructure(TG263Structure(
		"Cauda",
		{"cauda equina", "horse tail"},
		TG263Category::OAR,
		AnatomicalSite::SPINE
	));
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadCommonTargets()
{
	RegisterStructure(TG263Structure(
		"GTV",
		{"gross tumor volume", "GTV primary", "primary GTV"},
		TG263Category::TARGET,
		AnatomicalSite::UNKNOWN
	));

	RegisterStructure(TG263Structure(
		"CTV",
		{"clinical target volume", "CTV primary", "primary CTV"},
		TG263Category::TARGET,
		AnatomicalSite::UNKNOWN
	));

	RegisterStructure(TG263Structure(
		"PTV",
		{"planning target volume", "PTV primary", "primary PTV"},
		TG263Category::TARGET,
		AnatomicalSite::UNKNOWN
	));

	RegisterStructure(TG263Structure(
		"ITV",
		{"internal target volume"},
		TG263Category::TARGET,
		AnatomicalSite::UNKNOWN
	));
}

///////////////////////////////////////////////////////////////////////////////
void TG263Nomenclature::LoadCommonOARs()
{
	RegisterStructure(TG263Structure(
		"Body",
		{"external", "skin", "patient outline", "contour"},
		TG263Category::EXTERNAL,
		AnatomicalSite::WHOLE_BODY
	));

	RegisterStructure(TG263Structure(
		"BoneMarrow",
		{"bone marrow", "marrow"},
		TG263Category::OAR,
		AnatomicalSite::WHOLE_BODY
	));
}
