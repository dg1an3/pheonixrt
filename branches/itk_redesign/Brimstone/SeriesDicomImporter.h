// SeriesDicomImporter.h: interface for the CSeriesDicomImporter class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <dcmtk/dcmdata/dcfilefo.h>

#include <vector>

#include <Series.h>


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
