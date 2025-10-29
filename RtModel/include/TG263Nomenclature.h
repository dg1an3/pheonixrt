// Copyright (C) 2nd Messenger Systems
// TG263Nomenclature.h - AAPM TG-263 standardized nomenclature translator
//
// Implements translation between common clinical structure names and
// standardized TG-263 nomenclature for radiotherapy planning.
//
// Reference: AAPM Task Group 263 - Standardizing Nomenclatures in Radiation Oncology
// https://www.aapm.org/pubs/reports/RPT_263.pdf

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

namespace dH {

///////////////////////////////////////////////////////////////////////////////
// TG-263 Structure Categories
///////////////////////////////////////////////////////////////////////////////
enum class TG263Category {
	UNKNOWN = 0,
	TARGET = 1,          // Planning target volumes
	OAR = 2,            // Organs at risk
	NORMAL_TISSUE = 3,  // Normal tissue structures
	LYMPH_NODE = 4,     // Lymph node regions
	CAVITY = 5,         // Body cavities
	BONE = 6,           // Skeletal structures
	MARKER = 7,         // Fiducial markers, etc.
	EXTERNAL = 8,       // External contour, skin
	AVOIDANCE = 9,      // Avoidance structures
	DOSE_REGION = 10    // Dose specification regions
};

///////////////////////////////////////////////////////////////////////////////
// Anatomical Site Categories
///////////////////////////////////////////////////////////////////////////////
enum class AnatomicalSite {
	UNKNOWN = 0,
	BRAIN = 1,
	HEAD_NECK = 2,
	THORAX = 3,
	ABDOMEN = 4,
	PELVIS = 5,
	EXTREMITY = 6,
	SPINE = 7,
	WHOLE_BODY = 8
};

///////////////////////////////////////////////////////////////////////////////
// Laterality
///////////////////////////////////////////////////////////////////////////////
enum class Laterality {
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	BILATERAL = 3
};

///////////////////////////////////////////////////////////////////////////////
// Structure Match Result
///////////////////////////////////////////////////////////////////////////////
struct TG263Match {
	std::string standardName;     // TG-263 standard name
	std::string alternativeName;  // Alternative TG-263 name (if applicable)
	TG263Category category;       // Structure category
	AnatomicalSite site;          // Anatomical site
	Laterality laterality;        // Left/Right/Bilateral
	float confidence;             // Match confidence [0.0, 1.0]
	std::string matchedAlias;     // Which alias matched (for debugging)

	TG263Match()
		: category(TG263Category::UNKNOWN)
		, site(AnatomicalSite::UNKNOWN)
		, laterality(Laterality::NONE)
		, confidence(0.0f)
	{}
};

///////////////////////////////////////////////////////////////////////////////
// TG-263 Structure Definition
///////////////////////////////////////////////////////////////////////////////
struct TG263Structure {
	std::string standardName;
	std::vector<std::string> aliases;  // Common alternative names
	TG263Category category;
	AnatomicalSite site;
	Laterality laterality;
	std::string description;

	TG263Structure()
		: category(TG263Category::UNKNOWN)
		, site(AnatomicalSite::UNKNOWN)
		, laterality(Laterality::NONE)
	{}

	TG263Structure(
		const std::string& name,
		const std::vector<std::string>& aliasList,
		TG263Category cat,
		AnatomicalSite s,
		Laterality lat = Laterality::NONE,
		const std::string& desc = "")
		: standardName(name)
		, aliases(aliasList)
		, category(cat)
		, site(s)
		, laterality(lat)
		, description(desc)
	{}
};

///////////////////////////////////////////////////////////////////////////////
// TG263 Nomenclature Translator
///////////////////////////////////////////////////////////////////////////////
class TG263Nomenclature {
public:
	TG263Nomenclature();
	virtual ~TG263Nomenclature();

	///////////////////////////////////////////////////////////////////////////
	// Primary Translation Methods
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Translate a clinical structure name to TG-263 standard
	 * @param clinicalName The input structure name
	 * @param minConfidence Minimum confidence threshold (default: 0.5)
	 * @return Match result with standard name and metadata
	 */
	TG263Match TranslateToStandard(
		const std::string& clinicalName,
		float minConfidence = 0.5f) const;

	/**
	 * Get all possible matches for a clinical name
	 * @param clinicalName The input structure name
	 * @param maxResults Maximum number of results to return
	 * @return Vector of matches sorted by confidence (highest first)
	 */
	std::vector<TG263Match> GetAllMatches(
		const std::string& clinicalName,
		int maxResults = 5) const;

	/**
	 * Check if a name is already TG-263 compliant
	 * @param name The structure name to check
	 * @return true if name exactly matches a TG-263 standard name
	 */
	bool IsStandardName(const std::string& name) const;

	/**
	 * Get all aliases for a TG-263 standard name
	 * @param standardName TG-263 standard name
	 * @return Vector of known aliases
	 */
	std::vector<std::string> GetAliases(const std::string& standardName) const;

	///////////////////////////////////////////////////////////////////////////
	// Bulk Translation
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Translate a list of structure names
	 * @param clinicalNames Vector of input names
	 * @param minConfidence Minimum confidence threshold
	 * @return Vector of match results (same order as input)
	 */
	std::vector<TG263Match> TranslateBatch(
		const std::vector<std::string>& clinicalNames,
		float minConfidence = 0.5f) const;

	///////////////////////////////////////////////////////////////////////////
	// Query Methods
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Get all structures in a category
	 * @param category The structure category
	 * @return Vector of standard names
	 */
	std::vector<std::string> GetStructuresByCategory(
		TG263Category category) const;

	/**
	 * Get all structures for an anatomical site
	 * @param site The anatomical site
	 * @return Vector of standard names
	 */
	std::vector<std::string> GetStructuresBySite(
		AnatomicalSite site) const;

	/**
	 * Get structure definition
	 * @param standardName TG-263 standard name
	 * @return Pointer to structure definition (NULL if not found)
	 */
	const TG263Structure* GetStructureDefinition(
		const std::string& standardName) const;

	/**
	 * Get all defined structures
	 * @return Vector of all TG-263 standard names
	 */
	std::vector<std::string> GetAllStandardNames() const;

	///////////////////////////////////////////////////////////////////////////
	// Statistics and Info
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Get total number of defined structures
	 */
	int GetStructureCount() const;

	/**
	 * Get total number of aliases
	 */
	int GetAliasCount() const;

	/**
	 * Get human-readable category name
	 */
	static std::string CategoryToString(TG263Category category);

	/**
	 * Get human-readable site name
	 */
	static std::string SiteToString(AnatomicalSite site);

	/**
	 * Get human-readable laterality
	 */
	static std::string LateralityToString(Laterality laterality);

protected:
	///////////////////////////////////////////////////////////////////////////
	// Matching Algorithms
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Exact match (case-insensitive)
	 */
	TG263Match ExactMatch(const std::string& name) const;

	/**
	 * Fuzzy match using edit distance and token matching
	 */
	std::vector<TG263Match> FuzzyMatch(
		const std::string& name,
		int maxResults = 5) const;

	/**
	 * Calculate match confidence score
	 */
	float CalculateConfidence(
		const std::string& clinicalName,
		const std::string& standardName,
		const std::string& matchedAlias) const;

	/**
	 * Normalize string for matching (lowercase, trim, remove special chars)
	 */
	std::string NormalizeName(const std::string& name) const;

	/**
	 * Calculate Levenshtein edit distance
	 */
	int EditDistance(const std::string& s1, const std::string& s2) const;

	/**
	 * Token-based similarity (Jaccard index of word sets)
	 */
	float TokenSimilarity(const std::string& s1, const std::string& s2) const;

	/**
	 * Detect laterality from name
	 */
	Laterality DetectLaterality(const std::string& name) const;

private:
	///////////////////////////////////////////////////////////////////////////
	// Internal Data Structures
	///////////////////////////////////////////////////////////////////////////

	// Map: standard name -> structure definition
	std::map<std::string, TG263Structure> m_structures;

	// Map: normalized alias -> standard name (for fast lookup)
	std::map<std::string, std::string> m_aliasMap;

	// Map: category -> list of standard names
	std::map<TG263Category, std::vector<std::string>> m_categoryMap;

	// Map: site -> list of standard names
	std::map<AnatomicalSite, std::vector<std::string>> m_siteMap;

	///////////////////////////////////////////////////////////////////////////
	// Initialization
	///////////////////////////////////////////////////////////////////////////

	/**
	 * Populate structure database with TG-263 definitions
	 */
	void InitializeStructures();

	/**
	 * Register a structure in the database
	 */
	void RegisterStructure(const TG263Structure& structure);

	/**
	 * Load structures by anatomical site
	 */
	void LoadBrainStructures();
	void LoadHeadNeckStructures();
	void LoadThoraxStructures();
	void LoadAbdomenStructures();
	void LoadPelvisStructures();
	void LoadSpineStructures();
	void LoadCommonTargets();
	void LoadCommonOARs();
};

} // namespace dH
