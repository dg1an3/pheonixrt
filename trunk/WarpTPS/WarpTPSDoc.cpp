// WarpTPSDoc.cpp : implementation of the CWarpTPSDoc class
//

#include "stdafx.h"
#include "WarpTPS.h"

#include "WarpTPSDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSDoc

IMPLEMENT_DYNCREATE(CWarpTPSDoc, CDocument)

BEGIN_MESSAGE_MAP(CWarpTPSDoc, CDocument)
	//{{AFX_MSG_MAP(CWarpTPSDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSDoc construction/destruction

CWarpTPSDoc::CWarpTPSDoc()
{
	for (int nAt = 0; nAt < 5; nAt++)
	{
		// initialize each image
		m_pImage[nAt] = NULL;
	}
}

CWarpTPSDoc::~CWarpTPSDoc()
{
	for (int nAt = 0; nAt < 5; nAt++)
	{
		// delete the existing images
		delete m_pImage[nAt];
		m_pImage[nAt] = NULL;
	}
}

BOOL CWarpTPSDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// load the images

	if (!LoadImages(_T(".\\TestData\\001.bmp"), _T(".\\TestData\\002.bmp")))
	{
		return FALSE;
	}

	// add some landmark points
	InitCornerLandmarks();

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CWarpTPSDoc serialization

void CWarpTPSDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_transform.GetLandmarkCount();
		for (int nAt = 0; nAt < m_transform.GetLandmarkCount(); nAt++)
		{
			ar << m_transform.GetLandmark(0, nAt);
			ar << m_transform.GetLandmark(1, nAt);
		}
	}
	else
	{
		int nCount = 0;
		ar >> nCount;
		for (int nAt = 0; nAt < nCount; nAt++)
		{
			CVectorD<3> vL1, vL2;
			ar >> vL1; 
			ar >> vL2;
			m_transform.AddLandmark(vL1, vL2);
			m_inversetransform.AddLandmark(vL2, vL1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSDoc diagnostics

#ifdef _DEBUG
void CWarpTPSDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWarpTPSDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSDoc commands

BOOL CWarpTPSDoc::LoadImages(LPCTSTR pszFilename1, LPCTSTR pszFilename2)
{
	for (int nAt = 0; nAt < 5; nAt++)
	{
		// delete the existing images
		delete m_pImage[nAt];
		m_pImage[nAt] = NULL;
	}

	// load the first image
	m_pImage[ImageRole::DestinationImage] = new CDib();
	BOOL bResult = m_pImage[ImageRole::DestinationImage]->Load(pszFilename1);
	if (!bResult)
	{
		goto ERROR_CLEANUP;
	}

	// load the warped image (initializes the size)
	m_pImage[ImageRole::WarpedSourceImage] = new CDib();
	bResult = m_pImage[ImageRole::WarpedSourceImage]->Load(pszFilename2);
	if (!bResult)
	{
		goto ERROR_CLEANUP;
	}

	// load the second image
	m_pImage[ImageRole::SourceImage] = new CDib();
	bResult = m_pImage[ImageRole::SourceImage]->Load(pszFilename2);
	if (!bResult)
	{
		goto ERROR_CLEANUP;
	}

	// load the warped image (initializes the size)
	m_pImage[ImageRole::WarpedDestinationImage] = new CDib();
	bResult = m_pImage[ImageRole::WarpedDestinationImage]->Load(pszFilename1);
	if (!bResult)
	{
		goto ERROR_CLEANUP;
	}
	
	// load the blended image (initializes size and pixel format)
	m_pImage[ImageRole::BlendedImage] = new CDib();
	bResult = m_pImage[ImageRole::BlendedImage]->Load(pszFilename1);
	if (!bResult)
	{
		goto ERROR_CLEANUP;
	}

	// check the size of the images
	if (m_pImage[ImageRole::DestinationImage]->GetSize() 
		!= m_pImage[ImageRole::SourceImage]->GetSize())
	{
		::AfxMessageBox(_T("Images must have same dimensions"));
		goto ERROR_CLEANUP;
	}

	// ensures the initial update sets things up
	SendInitialUpdate();

	// redraw the window
	UpdateAllViews(NULL);

	// success
	return TRUE;

ERROR_CLEANUP:
	// delete any loaded images
	for (int nAt = 0; nAt < 5; nAt++)
	{
		// delete the existing images
		delete m_pImage[nAt];
		m_pImage[nAt] = NULL;
	}

	// and return FALSE
	return FALSE;
}

CDib * CWarpTPSDoc::GetImage(int nIndex)
{
	return m_pImage[nIndex];
}

CTPSTransform * CWarpTPSDoc::GetTransform()
{
	return &m_transform;
}

CTPSTransform * CWarpTPSDoc::GetInverseTransform()
{
	return &m_inversetransform;
}

void CWarpTPSDoc::InitCornerLandmarks()
{
	// remove all landmarks from the transform
	GetTransform()->RemoveAllLandmarks();
	GetInverseTransform()->RemoveAllLandmarks();

	// add a landmark at <0.0, 0.0>
	GetTransform()->AddLandmark(CVectorD<3>(0.0, 0.0));
	GetInverseTransform()->AddLandmark(CVectorD<3>(0.0, 0.0));

	// compute the other four corners
	CSize size1 = GetImage(ImageRole::DestinationImage)->GetSize();
	CSize size2 = GetImage(ImageRole::SourceImage)->GetSize();

	// add the other four corners
	GetTransform()->AddLandmark(CVectorD<3>(0.0, size1.cy), 
		CVectorD<3>(0.0, size2.cy));
	GetTransform()->AddLandmark(CVectorD<3>(size1.cx, size1.cy), 
		CVectorD<3>(size2.cx, size2.cy));
	GetTransform()->AddLandmark(CVectorD<3>(size1.cx, 0.0), 
		CVectorD<3>(size2.cx, 0.0));

	GetInverseTransform()->AddLandmark(CVectorD<3>(0.0, size2.cy), 
		CVectorD<3>(0.0, size1.cy));
	GetInverseTransform()->AddLandmark(CVectorD<3>(size2.cx, size2.cy), 
		CVectorD<3>(size1.cx, size1.cy));
	GetInverseTransform()->AddLandmark(CVectorD<3>(size2.cx, 0.0), 
		CVectorD<3>(size1.cx, 0.0));

	// resample the images
	GetTransform()->Resample(m_pImage[ImageRole::SourceImage], 
		m_pImage[ImageRole::WarpedSourceImage], 1.0);
	GetInverseTransform()->Resample(m_pImage[ImageRole::DestinationImage], 
		m_pImage[ImageRole::WarpedDestinationImage], 0.5);
}

void CWarpTPSDoc::DeleteContents() 
{
	m_transform.RemoveAllLandmarks();	
	m_inversetransform.RemoveAllLandmarks();
	CDocument::DeleteContents();
}
