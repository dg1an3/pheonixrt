// Copyright (C) 2nd Messenger Systems
// $Id: SeriesDicomImporter.h 609 2008-09-14 18:34:53Z dglane001 $
#if !defined(AFX_SERIESDICOMIMPORTER_H__718F0516_B419_4DF9_BFF5_5A5578C638AD__INCLUDED_)
#define AFX_SERIESDICOMIMPORTER_H__718F0516_B419_4DF9_BFF5_5A5578C638AD__INCLUDED_

#include <dcmtk/dcmdata/dcfilefo.h>

//#include <VectorD.h>

#include <vector>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace dH
{
class Series;
}

//////////////////////////////////////////////////////////////////////////////
class CSeriesDicomImporter  
{
public:
	CSeriesDicomImporter(dH::Series *pSeries, CFileDialog *pDlg);
	virtual ~CSeriesDicomImporter();

	// processes next file / image
	int ProcessNext();

	// formats the volume, once all files are read
	void FormatVolume();

	// resamples next image (i.e. puts in to proper slice in volume)
	void ResampleNextDicomImage();

	// imports the sset
	void ImportDicomStructureSet(DcmFileFormat *pFileFormat);

private:
	// the series for the importer
	dH::Series *m_pSeries;

	// the file dialog and position of the file
	CFileDialog *m_pDlg;
	POSITION m_posFile;

	// internal count of processed items
	int m_nCount;

	// private helper class to process image items
	class CDicomImageItem
	{
	public:
		CDicomImageItem(DcmFileFormat *pFileFormat);
		virtual ~CDicomImageItem() 
		{
			delete m_pFileFormat;
		}

		// helper for sorting the slices
		static bool SlicePositionLower(CDicomImageItem *pFirst, 
			CDicomImageItem *pSecond);

		// the contained format item (contains dicom data)
		DcmFileFormat *m_pFileFormat;

		// image height and width
		int m_nHeight;
		int m_nWidth;

		// image geometry
		VolumeReal::PointType m_vOrigin;
		VolumeReal::DirectionType m_mDirections; 
		VolumeReal::SpacingType m_vSpacing;
	};

	// the list of parsed dicom datasets
	vector< CDicomImageItem* > m_arrImageItems;

	// flag to indicate if the volume is formatted
	BOOL m_bVolumeFormatted;
};

#endif // !defined(AFX_SERIESDICOMIMPORTER_H__718F0516_B419_4DF9_BFF5_5A5578C638AD__INCLUDED_)
