#include "StdAfx.h"
#include "PlanXmlFile.h"

#include <itksys/SystemTools.hxx>

namespace dH
{

//////////////////////////////////////////////////////////////////////////////
int 
	PlanXmlReader::CanReadFile(const char* name)
{
	return itksys::SystemTools::FileExists(name) 
		&& !itksys::SystemTools::FileIsDirectory(name)
		&& itksys::SystemTools::FileLength(name) != 0;
}

//////////////////////////////////////////////////////////////////////////////
const char *
	FindAttributeValue(const char **atts, const char * name)
{
	for (const char **currentAttribute = atts; 
		(*currentAttribute) != NULL; currentAttribute++)
	{
		if (itksys::SystemTools::Strucmp((*currentAttribute),name))
			return (*currentAttribute);
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlReader::StartElement(const char * name,const char **atts)
{
	if (itksys::SystemTools::Strucmp(name,"BEAM") == 0)
	{
		m_pCurrentBeam = CBeam::New();
		GetOutputObject()->AddBeam(m_pCurrentBeam);
	}
	if (itksys::SystemTools::Strucmp(name, "BEAMLET") == 0)
	{
		assert(!m_pCurrentBeam.IsNull());
		// store the index
	}
	else if (itksys::SystemTools::Strucmp(name, "ROW") == 0)
	{
		assert(!m_pCurrentBeam.IsNull());
		// data is row data to be added to intensity map
	}
	else if (itksys::SystemTools::Strucmp(name, "TARGET") == 0)
	{
		// assert(!m_pCurrentPrescription.IsNull());
	}
	else if (itksys::SystemTools::Strucmp(name, "OAR") == 0)
	{
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlReader::EndElement(const char *name)
{
	if (itksys::SystemTools::Strucmp(name,"RESOLUTION") == 0)
	{
	}
	else if (itksys::SystemTools::Strucmp(name,"CONVOLUTION") == 0)
	{
	}
	else if (itksys::SystemTools::Strucmp(name,"PHIANGLES") == 0)
	{
	}
	if (itksys::SystemTools::Strucmp(name,"ISOCENTER") == 0)
	{
	}
	else if (itksys::SystemTools::Strucmp(name, "ENERGY") == 0)
	{
	}
	else if (itksys::SystemTools::Strucmp(name, "GANTRY") == 0)
	{
		// data is gantry angle
	}
	else if (itksys::SystemTools::Strucmp(name, "BEAMLET") == 0)
	{
		// data is beamlet file
	}
	else if (itksys::SystemTools::Strucmp(name, "ROW") == 0)
	{
		// data is row data to be added to intensity map
	}
	else if (itksys::SystemTools::Strucmp(name,"BEAM") == 0)
	{
		m_pCurrentBeam = NULL;
	}
	else if (itksys::SystemTools::Strucmp(name,"WEIGHT") == 0)
	{
	}
	else if (itksys::SystemTools::Strucmp(name,"PRIORITY") == 0)
	{
	}
	else if (itksys::SystemTools::Strucmp(name,"GOALDVH") == 0)
	{
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlReader::CharacterDataHandler(const char *inData, int inLength)
{
	// store character data for subsequent processing
}

//////////////////////////////////////////////////////////////////////////////
int 
	PlanXmlWriter::CanWriteFile(const char* name)
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
int 
	PlanXmlWriter::WriteFile()
{
}

}