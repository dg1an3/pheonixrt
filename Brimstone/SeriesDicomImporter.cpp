// Copyright (C) 2nd Messenger Systems
// $Id: SeriesDicomImporter.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"
#include "SeriesDicomImporter.h"

#include <Series.h>
#include <Structure.h>
//#include <Volumep.h>
//#include <Polygon.h>

#include <dcmtk/dcmdata/dcmetinf.h>
#include <dcmtk/dcmdata/dcdeftag.h>

#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dcpixel.h>

#include <dcmtk/dcmimgle/didocu.h>
#include <dcmtk/dcmimgle/dimo1img.h>
#include <dcmtk/dcmimgle/dimo2img.h>

#include <algorithm>

#define CHK_DCM(x) \
if (!(x.good()))	\
{					\
	ASSERT(FALSE);	\
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
CSeriesDicomImporter::CSeriesDicomImporter(dH::Series *pSeries, CFileDialog *pDlg)
: m_pSeries(pSeries),
	m_pDlg(pDlg),
	m_posFile(NULL),
	m_bVolumeFormatted(FALSE),
	m_nCount(0)
{
   m_posFile = m_pDlg->GetStartPosition();
}

//////////////////////////////////////////////////////////////////////////////
CSeriesDicomImporter::~CSeriesDicomImporter()
{
	// delete objects
	while (m_arrImageItems.size() > 0)
	{
		delete m_arrImageItems.back();
		m_arrImageItems.pop_back();
	}
}

//////////////////////////////////////////////////////////////////////////////
int CSeriesDicomImporter::ProcessNext()
{
	USES_CONVERSION;

	if (m_posFile != NULL)
	{
		// get next filename
		CString strFilepath = m_pDlg->GetNextPathName(m_posFile);

		// open file
		DcmFileFormat *pFileFormat = new DcmFileFormat();
		if (pFileFormat->loadFile(T2A(strFilepath)).good())
		{
			OFString strModality;
			CHK_DCM(pFileFormat->getDataset()->findAndGetOFString(DCM_Modality, strModality));

			if (strModality == "CT")
			{
				CDicomImageItem *pItem = new CDicomImageItem(pFileFormat);
				m_arrImageItems.push_back(pItem);
			}
			else if (strModality == "MR")
			{
				CDicomImageItem *pItem = new CDicomImageItem(pFileFormat);
				m_arrImageItems.push_back(pItem);
			}
			else if (strModality == "RTSTRUCT")
			{
				ImportDicomStructureSet(pFileFormat);
				delete pFileFormat;
			}
		}
		else
		{
			delete pFileFormat;
		}
	}
	else if (m_arrImageItems.size() > 0)
	{
		ResampleNextDicomImage();
	}
	else
	{
		return -1;
	}

	return ++m_nCount;
}

//////////////////////////////////////////////////////////////////////////////
void CSeriesDicomImporter::FormatVolume()
{
	if (!m_bVolumeFormatted)
	{
		// sort by slice position, in ascending order
		sort(m_arrImageItems.begin(), m_arrImageItems.end(), 
			CDicomImageItem::SlicePositionLower);

		// get first item for geometry info
		CDicomImageItem *pItem = m_arrImageItems[0];

		// determine dimensions
		itk::Size<3> vSize;
		vSize[0] = pItem->m_nWidth;
		vSize[1] = pItem->m_nHeight;
		vSize[2] = m_arrImageItems.size();
		m_pSeries->GetDensity()->SetRegions(vSize);
		m_pSeries->GetDensity()->Allocate();

		// set origin
		m_pSeries->GetDensity()->SetOrigin(pItem->m_vOrigin); 

		// determine slice spacing
		VolumeReal::SpacingType vSpacing = pItem->m_vSpacing;
		CDicomImageItem *pItem2 = m_arrImageItems[1];
		/// TEMP: make isotropic
		vSpacing[2] = // vSpacing[0];
			pItem2->m_vOrigin[2] - pItem->m_vOrigin[2];
		m_pSeries->GetDensity()->SetSpacing(vSpacing); 

		// TODO: set direction
		// m_pSeries->GetDensity()->SetDirections(mDirections);

		m_bVolumeFormatted = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
BOOL convertVoxels(VOXEL_TYPE *pOrigVoxels, int nSize, 
				   VOXEL_REAL *pNewVoxels, Float64 slope = 1.0, Float64 intercept = 0.0)
{
	for (int nAt = 0; nAt < nSize; nAt++)
	{
		pNewVoxels[nAt] = (VOXEL_REAL) (slope * pOrigVoxels[nAt] + intercept);
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
void CSeriesDicomImporter::ResampleNextDicomImage()
{
	// consume the image items, starting at the end of the list
	DcmDataset *pDataset = m_arrImageItems.back()->m_pFileFormat->getDataset();
	DcmMetaInfo *pMetaInfo = m_arrImageItems.back()->m_pFileFormat->getMetaInfo();

	FormatVolume();

	// calculate current slice position in the volume
	int nSlice = (int) m_arrImageItems.size()-1;

	// now open image
	DicomImage *pImage = new DicomImage(pDataset, pMetaInfo->getOriginalXfer());
	
	if (pImage != NULL)
	{
		if (pImage->getStatus() == EIS_Normal)
		{
			DcmElement *pElem = NULL;
			CHK_DCM(pDataset->findAndGetElement(DCM_PixelData, pElem));

			DcmPixelData *pDPD = OFstatic_cast(DcmPixelData *, pElem);
			CHK_DCM(pDPD->loadAllDataIntoMemory());

			Uint16 nPR = 0;
			CHK_DCM(pDataset->findAndGetUint16(DCM_PixelRepresentation, nPR));

			Float64 slope = 1.0;
			if (!(pDataset->findAndGetFloat64(DCM_RescaleSlope, slope).good()))
			{
				slope = 1.0;
			}

			Float64 intercept = 0.0;
			if (!(pDataset->findAndGetFloat64(DCM_RescaleIntercept, intercept).good()))
			{
				intercept = 0.0;
			}

			// check that slice position calculation will be OK
			ASSERT(pImage->getWidth() % 4 == 0);
			ASSERT(pImage->getWidth() == m_pSeries->GetDensity()->GetBufferedRegion().GetSize()[0]);

			int nSize = pImage->getHeight() * pImage->getWidth();
			int nSliceOffset = nSlice * nSize;
			if (pDPD->getVR() == EVR_OW)
			{
				Uint16 *wordVals = NULL;
				CHK_DCM(pDPD->getUint16Array(wordVals));

				if (nPR == 0)	// unsigned
				{
					convertVoxels(wordVals, nSize, m_pSeries->GetDensity()->GetBufferPointer() + nSliceOffset, 
						slope, intercept);
				}
				else
				{
					Sint16 *swordVals = (Sint16 *) wordVals;
					convertVoxels(swordVals, nSize, m_pSeries->GetDensity()->GetBufferPointer() + nSliceOffset, 
						slope, intercept);
				}
			}
			else if (pDPD->getVR() == EVR_OB)
			{
				Uint8 *byteVals = NULL;
				CHK_DCM(pDPD->getUint8Array(byteVals));

				if (nPR == 0)	// unsigned
				{
					convertVoxels(byteVals, nSize, m_pSeries->GetDensity()->GetBufferPointer() + nSliceOffset, 
						slope, intercept);
				}
				else
				{
					Sint8 *sbyteVals = (Sint8 *) byteVals;
					convertVoxels(sbyteVals, nSize, m_pSeries->GetDensity()->GetBufferPointer() + nSliceOffset, 
						slope, intercept);
				}
			}
		} 
		else
		{
			cerr << "Error: cannot load DICOM image (" << DicomImage::getString(pImage->getStatus()) << ")" << endl;
		}

		delete pImage;
	}

	// now remove image from vector
	delete m_arrImageItems.back();
	m_arrImageItems.pop_back();
}

//////////////////////////////////////////////////////////////////////////////
CSeriesDicomImporter::CDicomImageItem::CDicomImageItem(DcmFileFormat *pFileFormat)
	: m_pFileFormat(pFileFormat)
{
	DcmDataset *pDataset = m_pFileFormat->getDataset();

	// extract width and height
	OFString strHeight;
	CHK_DCM(pDataset->findAndGetOFString(DCM_Rows, strHeight));
	sscanf_s(strHeight.c_str(), "%i", &m_nHeight);

	OFString strWidth;
	CHK_DCM(pDataset->findAndGetOFString(DCM_Rows, strWidth));
	sscanf_s(strWidth.c_str(), "%i", &m_nWidth);

	// extract pixel origin
	OFString strImgPositionPatient;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_ImagePositionPatient, strImgPositionPatient));
	sscanf_s(strImgPositionPatient.c_str(), "%lf\\%lf\\%lf",
		&m_vOrigin[0], &m_vOrigin[1], &m_vOrigin[2]);

	// extract pixel spacing
	OFString strPixelSpacing;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_PixelSpacing, strPixelSpacing));
	sscanf_s(strPixelSpacing.c_str(), "%lf\\%lf", 
		&m_vSpacing[0], &m_vSpacing[1]);
	m_vSpacing[2] = 0.0;	// can't tell slice spacing here

	// extract directions
	OFString strImgOrientPatient;
	CHK_DCM(pDataset->findAndGetOFStringArray(DCM_ImageOrientationPatient, strImgOrientPatient));
	sscanf_s(strImgOrientPatient.c_str(), "%lf\\%lf\\%lf\\%lf\\%lf\\%lf",
		&m_mDirections(0, 0), &m_mDirections(1, 0), &m_mDirections(2, 0),
		&m_mDirections(2, 0), &m_mDirections(1, 1), &m_mDirections(2, 1));
}

//////////////////////////////////////////////////////////////////////////////
bool CSeriesDicomImporter::CDicomImageItem::SlicePositionLower(CDicomImageItem *pFirst, 
			CDicomImageItem *pSecond)
	// return true if first has lower slice position than second
{
	REAL pos1 = pFirst->m_vOrigin[2];
	REAL pos2 = pSecond->m_vOrigin[2];

	return (pos1 < pos2); 
}

//////////////////////////////////////////////////////////////////////////////
void CSeriesDicomImporter::ImportDicomStructureSet(DcmFileFormat *pFileFormat)
{
	DcmDataset *pDataset = pFileFormat->getDataset();

	// CTypedPtrArray< CObArray, CStructure * > arrROIs;
	std::map<int, dH::Structure::Pointer > mapROIs;

	DcmSequenceOfItems *pSSROISequence = NULL;
	CHK_DCM(pDataset->findAndGetSequence(DCM_StructureSetROISequence, pSSROISequence));
	for (int nAtROI = 0; nAtROI < (int) pSSROISequence->card(); nAtROI++)
	{
		DcmItem *pROIItem = pSSROISequence->getItem(nAtROI);

		long nROINumber;
		CHK_DCM(pROIItem->findAndGetSint32(DCM_ROINumber, nROINumber));

		OFString strName;
		CHK_DCM(pROIItem->findAndGetOFString(DCM_ROIName, strName));

		dH::Structure::Pointer pStruct = dH::Structure::New();
		pStruct->SetName(strName.c_str());

		// add to ROI list
		// arrROIs.SetAtGrow(nROINumber, pStruct);
		mapROIs[nROINumber] = pStruct;

		// add structure to series
		m_pSeries->AddStructure(pStruct);
	}

	DcmSequenceOfItems *pROIContourSequence = NULL;
	CHK_DCM(pDataset->findAndGetSequence(DCM_ROIContourSequence, pROIContourSequence));
	for (int nAtROIContour = 0; nAtROIContour < (int) pROIContourSequence->card(); nAtROIContour++)
	{
		DcmItem *pROIContourItem = pROIContourSequence->getItem(nAtROIContour);

		long nRefROINumber = 0;
		CHK_DCM(pROIContourItem->findAndGetSint32(DCM_ReferencedROINumber, nRefROINumber));
		dH::Structure *pStruct = mapROIs[nRefROINumber];
		ASSERT(pStruct != NULL);

		OFString strColor;
		CHK_DCM(pROIContourItem->findAndGetOFStringArray(DCM_ROIDisplayColor, strColor));
		
		int nRed, nGrn, nBlu;
		sscanf_s(strColor.c_str(), "%i\\%i\\%i", &nRed, &nGrn, &nBlu);
		pStruct->SetColor(RGB(nRed, nGrn, nBlu));

		DcmSequenceOfItems *pContourSequence = NULL;
		CHK_DCM(pROIContourItem->findAndGetSequence(DCM_ContourSequence, pContourSequence));
		for (int nAtContour = 0; nAtContour < (int) pContourSequence->card(); nAtContour++)
		{
			DcmItem *pContourItem = pContourSequence->getItem(nAtContour);

			OFString strContourGeometricType;
			CHK_DCM(pContourItem->findAndGetOFString(DCM_ContourGeometricType, strContourGeometricType));
			if (strContourGeometricType == "CLOSED_PLANAR")
			{
				long nContourPoints;
				CHK_DCM(pContourItem->findAndGetSint32(DCM_NumberOfContourPoints, nContourPoints));

				OFString strContourData;
				CHK_DCM(pContourItem->findAndGetOFStringArray(DCM_ContourData, strContourData));

				dH::Structure::PolygonType::Pointer pPoly = dH::Structure::PolygonType::New();

				int nStart = 0;
				int nNext = 0;
				float slice_z = 0.0;
				for (int nAtPoint = 0; nAtPoint < nContourPoints; nAtPoint++)
				{
					double coord[3];
					// sscanf_s(strContourData.c_str(), "%lf\\%lf\\%lf", &coord[0], &coord[1], &coord[2]);

					// now parse contour data, one vertex at a time
					nNext = (int) strContourData.find('\\', nStart);
					sscanf_s(strContourData.substr(nStart, nNext-nStart).c_str(), "%lf", &coord[0]);
					nStart = nNext+1;

					nNext = (int) strContourData.find('\\', nStart);
					sscanf_s(strContourData.substr(nStart, nNext-nStart).c_str(), "%lf", &coord[1]);
					nStart = nNext+1;

					nNext = (int) strContourData.find('\\', nStart);
					sscanf_s(strContourData.substr(nStart, nNext-nStart).c_str(), "%lf", &coord[2]);
					nStart = nNext+1;

					if (nAtPoint == 0)
					{
						slice_z = coord[2];
					}
					// ASSERT(IsApproxEqual(coord_z, slice_z));
					pPoly->AddPoint(dH::Structure::PolygonType::PointType(coord)); // AddVertex(MakeVector<2>(coord_x, coord_y)); // (vVert);
				}
				pStruct->AddContour(pPoly, slice_z);
			}
		}
	}
}


