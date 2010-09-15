#pragma once

#include <itkXMLFile.h>

#include <Plan.h>
#include <Prescription.h>

namespace dH
{

/**
 *
 */
class PlanXmlReader 
	: public XMLReader<Plan>
{
public:
	/** determine whether a file can be opened and read */
	virtual int CanReadFile(const char* name);

	/** called from XML parser with start-of-element information. */
	virtual void StartElement(const char * name,const char **atts);

	/** called from XML parser when ending tag encountered */
	virtual void EndElement(const char *name);

	/** called from XML parser with the character data for an XML element */
	virtual void CharacterDataHandler(const char *inData, int inLength);

private:
	/** current character data for the open element */
	std::string m_currentCharacterData;

	/** stores the current beam */
	CBeam::Pointer m_pCurrentBeam;

	/** stores current beamlet's position */
	Beam::IntensityMap::PointType m_currentBeamletPosition;
};

/**
 *
 */
class PlanXmlWriter
	: public XMLWriterBase<Plan>
{
public:
	PlanXmlWriter();

	/** Return non-zero if the filename given is writeable. */
	virtual int CanWriteFile(const char* name);

	/** accessors for data path */
	void SetImageSeriesPath(const std::string& strPath);

	/** accessors for data path */
	void SetPlanDataPath(const std::string& strPath);

	/** Write the XML file, based on the Input Object */
	virtual int WriteFile();

	/** write out the dose calculation parameters for the plan */
	void WriteDoseCalcParams(Plan * pPlan);

	/** write out individual beam elements */
	void WriteBeam(int nBeam, Beam * pBeam);
	void WriteBeamlet(int nBeam, 
		const Beam::IntensityMap::PointType& position, 
		int nBeamlet, VolumeReal * pBeamlet);
	void WriteIntensityMap(int nBeam, Beam::IntensityMap * pIM);

	/** write out individual prescription elements */
	void WritePrescription(Prescription *pPrescription);

	/** write out optimization parameters */
	void WriteOptimizationParameters(Plan * pPlan);

	/** write out calculated DVHs */
	void WriteDVHs(Plan * pPlan);

	/** write a start element with an attribute  */
	void WriteStartElement(const char *name);
	void WriteStartElement(const char *name, const char *attribute, const char *attribute_value);
	void WriteEndElement(const char *name);

	/** write a single element with character data */
	void WriteElement(const char *name, const char *character_data);

	/** write a single element with character data */
	void WriteElement(const char *name, REAL real_data);

private:
	std::ofstream m_output;
	std::string m_strImageSeriesPath;
	std::string m_strPlanDataPath;

	int m_nLevel;
	bool m_bEolBeforeEndElement;
};

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::SetImageSeriesPath(const std::string& strPath)
{
	m_strImageSeriesPath = strPath;
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::SetPlanDataPath(const std::string& strPath)
{
	m_strPlanDataPath = strPath;
}

}