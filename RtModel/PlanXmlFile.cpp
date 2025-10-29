#include "StdAfx.h"
#include "PlanXmlFile.h"

#include <itksys/SystemTools.hxx>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

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
	else if (itksys::SystemTools::Strucmp(name, "BEAMLET") == 0)
	{
		assert(!m_pCurrentBeam.IsNull());
		const char *strPosition = FindAttributeValue(atts, "position");
		sscanf_s(strPosition, "%lf\\%lf", 
			m_currentBeamletPosition[0], m_currentBeamletPosition[1]);
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
		REAL resolution;
		sscanf_s(m_currentCharacterData.c_str(), "%lf", &resolution);
		GetOutputObject()->SetDoseResolution(resolution);
	}
	else if (itksys::SystemTools::Strucmp(name,"CONVOLUTION") == 0)
	{
	}
	else if (itksys::SystemTools::Strucmp(name,"PHIANGLES") == 0)
	{
	}
	if (itksys::SystemTools::Strucmp(name,"ISOCENTER") == 0)
	{
		itk::Vector<REAL> vIsocenter;
		sscanf_s(m_currentCharacterData.c_str(), "%lf\\%lf\\%lf", 
			&vIsocenter[0], &vIsocenter[1], &vIsocenter[2]);
		m_pCurrentBeam->SetIsocenter(vIsocenter);
	}
	else if (itksys::SystemTools::Strucmp(name, "ENERGY") == 0)
	{
		REAL energy;
		sscanf_s(m_currentCharacterData.c_str(), "%lf", &energy);
		// pCurrentBeam->SetEnergy(energy);
	}
	else if (itksys::SystemTools::Strucmp(name, "GANTRY") == 0)
	{
		REAL gantry;
		sscanf_s(m_currentCharacterData.c_str(), "%lf", &gantry);
		m_pCurrentBeam->SetGantryAngle(gantry);
	}
	else if (itksys::SystemTools::Strucmp(name, "INTENSITYMAP") == 0)
	{
		ImageFileReader<Beam::IntensityMap>::Pointer reader = 
			ImageFileReader<Beam::IntensityMap>::New();
		reader->SetFileName(m_currentCharacterData.c_str());
		reader->Update();
		// m_pCurrentBeam->SetIntensityMap(reader->GetOutput());
	}
	else if (itksys::SystemTools::Strucmp(name, "BEAMLET") == 0)
	{
		ImageFileReader<VolumeReal>::Pointer reader = 
			ImageFileReader<VolumeReal>::New();
		reader->SetFileName(m_currentCharacterData.c_str());
		reader->Update();
		// m_pCurrentBeam->InsertBeamlet(m_currentBeamletPosition, reader->GetOutput());
	}
	else if (itksys::SystemTools::Strucmp(name,"BEAM") == 0)
	{
		// add intensity map and beamlets to beam

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
	m_currentCharacterData = "";
	for(int i = 0; i < inLength; i++)
		m_currentCharacterData = m_currentCharacterData + inData[i];
}

//////////////////////////////////////////////////////////////////////////////
PlanXmlWriter::PlanXmlWriter()
{
	m_nLevel = 0;
	m_bEolBeforeEndElement = true;
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
	m_output.open(m_Filename.c_str());
	if(m_output.fail())
	{
		return FALSE;
	}

	XMLWriterBase<Plan>::WriteStartElement("?xml version=\"1.0\"?", m_output);
	m_output << std::endl;

	WriteStartElement("Plan");

		// image series path
		WriteElement("ImageSeries", m_strImageSeriesPath.c_str());

		// dose calculation parameters for the plan
		WriteDoseCalcParams(m_InputObject);

		// the plan's beams
		WriteStartElement("Beams");
		for (int nAtBeam = 0; nAtBeam < m_InputObject->GetBeamCount(); nAtBeam++)
		{
			WriteBeam(nAtBeam, m_InputObject->GetBeamAt(nAtBeam));
		}
		WriteEndElement("Beams");

		// prescription for the plan
		WritePrescription(NULL);

		// optimization parameters for the plan
		WriteOptimizationParameters(m_InputObject);

		// now write the plan dose
		WriteStartElement("PlanDose");

			const char *strExtension = "dcm";
			char strDoseFilePath[128];
			sprintf_s(strDoseFilePath, sizeof(strDoseFilePath),
				"%s\\total_plan_dose.%s", m_strPlanDataPath, strExtension);
			XMLWriterBase<Plan>::WriteCharacterData(strDoseFilePath, m_output);

		WriteEndElement("PlanDose");

		typedef itk::ImageFileWriter<VolumeReal> WriterType;
		WriterType::Pointer writer = WriterType::New();
		writer->SetFileName(strDoseFilePath);
		writer->SetInput(m_InputObject->GetDoseMatrix());
		writer->Update();

		// and finally the DVHs
		WriteDVHs(m_InputObject);

	WriteEndElement("Plan");
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteDoseCalcParams(Plan * pPlan)
{
	WriteStartElement("DoseCalcParams");

	WriteElement("Resolution", pPlan->GetDoseResolution());
	WriteElement("Convolution", "yes");
	WriteElement("PhiAngles", "64");

	WriteEndElement("DoseCalcParams");
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteBeam(int nBeam, Beam * pBeam)
{
	WriteStartElement("Beam");

	WriteStartElement("Isocenter");
	m_output << pBeam->GetIsocenter()[0] 
		<< '\\' << pBeam->GetIsocenter()[1]
		<< '\\' << pBeam->GetIsocenter()[2];
	WriteEndElement("Isocenter");

	WriteElement("Energy", 6.0 /*pBeam->GetEnergy()*/);
	WriteElement("Gantry", pBeam->GetGantryAngle());

	WriteIntensityMap(nBeam, pBeam->GetIntensityMap());

	WriteStartElement("Beamlets");
	for (int nAtBeamlet = 0; nAtBeamlet < pBeam->GetBeamletCount(); nAtBeamlet++)
	{
		Beam::IntensityMap::IndexType index;
		index[0] = nAtBeamlet;
		Beam::IntensityMap::PointType position;
		pBeam->GetIntensityMap()->TransformIndexToPhysicalPoint(index, position);
		WriteBeamlet(nBeam, position, nAtBeamlet, pBeam->GetBeamlet(nAtBeamlet));
	}
	WriteEndElement("Beamlets");

	WriteEndElement("Beam");
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteIntensityMap(int nBeam, Beam::IntensityMap * pIM)
{
	const char *strExtension = "dcm";
	char strIntensityMapFilePath[128];
	sprintf_s(strIntensityMapFilePath, sizeof(strIntensityMapFilePath),
		"%s\\beam_%i\\intensity_map.%s", m_strPlanDataPath, nBeam, strExtension);

	WriteElement("IntensityMap", strIntensityMapFilePath);

	ImageFileWriter<Beam::IntensityMap>::Pointer writer = 
		ImageFileWriter<Beam::IntensityMap>::New();
	writer->SetFileName(strIntensityMapFilePath);
	writer->SetInput(pIM);
	writer->Update();
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteBeamlet(int nBeam, 
			const Beam::IntensityMap::PointType& position, 
			int nBeamlet, VolumeReal * pBeamlet)
{
	char strPosition[128];
	sprintf_s(strPosition, sizeof(strPosition), "%lf\\%lf", position[0], position[1]);

	WriteStartElement("Beamlet", "position", strPosition);

	const char *strExtension = "dcm";
	char strBeamletFilePath[128];
	sprintf_s(strBeamletFilePath, sizeof(strBeamletFilePath),
		"%s\\beam_%i\\beamlet_%i.%s", m_strPlanDataPath, nBeam, nBeamlet, strExtension);
	XMLWriterBase<Plan>::WriteCharacterData(strBeamletFilePath, m_output);

	WriteEndElement("Beamlet");

	// now write out the beamlet to the file
	ImageFileWriter<VolumeReal>::Pointer writer = 
		ImageFileWriter<VolumeReal>::New();
	writer->SetFileName(strBeamletFilePath);
	writer->SetInput(pBeamlet);
	writer->Update();
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WritePrescription(Prescription * pPrescription)
{
	WriteStartElement("Prescription");

	Series *pImageSeries = pPrescription->GetPlan()->GetSeries();
	for (int nAt = 0; nAt < pImageSeries->GetStructureCount(); nAt++)
	{
		Structure *pStruct = pImageSeries->GetStructureAt(nAt);
		KLDivTerm *pKLDT = 
			dynamic_cast<KLDivTerm*>(pPrescription->GetStructureTerm(pStruct));
		if (pKLDT != NULL)
		{
			std::string strType;
			if (Structure::StructType::eTARGET == pStruct->GetType())
			{
				strType = "Target";
			}
			else if (Structure::StructType::eOAR == pStruct->GetType())
			{
				strType = "OAR";
			}
			else
			{
				continue;
			}

			WriteStartElement(strType.c_str(), "label", pStruct->GetName().c_str());

			WriteElement("Weight", pKLDT->GetWeight());
			WriteElement("Priority", pStruct->GetPriority());

			WriteStartElement("GoalDVH");
			const CMatrixNxM<>& mDVPs = pKLDT->GetDVPs();
			m_output << mDVPs[0][0] << '\\' << mDVPs[0][1];
			for (int nPoint = 1; nPoint < mDVPs.GetCols(); nPoint++)
			{
				m_output << '\\' << mDVPs[nPoint][0] << '\\' << mDVPs[nPoint][1];
			}
			WriteEndElement("GoalDVH");

			WriteEndElement(strType.c_str());
		}
	}

	WriteEndElement("Prescription");
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteOptimizationParameters(Plan * pPlan)
{
	WriteStartElement("OptimizationParams");

	WriteElement("Damping", "");
	WriteElement("KLDiv", "");
	WriteElement("VarMinMax", "25.0/50.0");
	WriteElement("Sigma", "0.5");
	WriteElement("SigmaMax", "0.5");

	WriteEndElement("OptimizationParams");
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteDVHs(Plan * pPlan)
{
	WriteStartElement("DVHs");

	Series *pImageSeries = pPlan->GetSeries();
	for (int nAt = 0; nAt < pImageSeries->GetStructureCount(); nAt++)
	{
		Structure *pStruct = pImageSeries->GetStructureAt(nAt);
		CHistogram *pHisto = pPlan->GetHistogram(pStruct, false);
		if (pHisto != NULL)
		{
			WriteStartElement("DVH", "label", pStruct->GetName().c_str());

			const CVectorN<>& vBinMeans = pHisto->GetBinMeans();
			const CVectorN<>& vCumBins = pHisto->GetCumBins();
			m_output << vBinMeans[0] << '\\' << vCumBins[0];
			for (int nBin = 1; nBin < vBinMeans.GetDim(); nBin++)
			{
				m_output << '\\' << vBinMeans[nBin] << '\\' << vCumBins[nBin];
			}

			WriteEndElement("DVH");
		}
	}

	WriteEndElement("DVHs");
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteStartElement(const char *name)
{
	WriteStartElement(name, "", "");
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteStartElement(const char *name, const char *attribute, 
			const char *attribute_value)
{
	// write tabs
	for (int nAt = 0; nAt < m_nLevel; nAt++)
		m_output << '\t';

	char strElement[128];
	if (strlen(attribute) == 0)
	{
		strcpy_s(strElement, sizeof(strElement), name);
	}
	else
	{
		sprintf_s(strElement, sizeof(strElement), 
			"%s %s=\"%s\"", name, attribute, attribute_value);
	}
	XMLWriterBase<Plan>::WriteStartElement(strElement, m_output);
	m_bEolBeforeEndElement = false;

	m_nLevel++;
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteEndElement(const char *name)
{
	if (m_bEolBeforeEndElement)
		m_output << std::endl;
	m_bEolBeforeEndElement = true;

	m_nLevel--;

	// write tabs
	for (int nAt = 0; nAt < m_nLevel; nAt++)
		m_output << '\t';

	XMLWriterBase<Plan>::WriteEndElement(name, m_output);
	m_output << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteElement(const char *name, const char *character_data)
{
	WriteStartElement(name);
	XMLWriterBase<Plan>::WriteCharacterData(character_data, m_output);
	WriteEndElement(name);
}

//////////////////////////////////////////////////////////////////////////////
void 
	PlanXmlWriter::WriteElement(const char *name, REAL real_data)
{
	char strData[32];
	sprintf_s(strData, sizeof(strData), "%lf", real_data);
	WriteElement(name, strData);
}

}