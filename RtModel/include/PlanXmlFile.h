#pragma once

#include <itkXMLFile.h>

#include <Plan.h>

namespace dH
{

/**
 *
 */
class PlanXmlReader 
	: public itk::XMLReader<CPlan>
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
	/** stores the current beam */
	CBeam::Pointer m_pCurrentBeam;
};

/**
 *
 */
class PlanXmlWriter
	: public itk::XMLWriterBase<CPlan>
{
public:
	/** Return non-zero if the filename given is writeable. */
	virtual int CanWriteFile(const char* name);

	/** Write the XML file, based on the Input Object */
	virtual int WriteFile();
};


}