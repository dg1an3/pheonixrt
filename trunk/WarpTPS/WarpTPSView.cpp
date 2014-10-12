// WarpTPSView.cpp : implementation of the CWarpTPSView class
//

#include "stdafx.h"
#include "WarpTPS.h"

#include "WarpTPSDoc.h"
#include "WarpTPSView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSView

IMPLEMENT_DYNCREATE(CWarpTPSView, CView)

BEGIN_MESSAGE_MAP(CWarpTPSView, CView)
	//{{AFX_MSG_MAP(CWarpTPSView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(ID_SHOWMARKS, OnShowmarks)
	ON_UPDATE_COMMAND_UI(ID_SHOWMARKS, OnUpdateShowmarks)
	ON_COMMAND(ID_FILE_OPEN_IMAGES, OnFileOpenImages)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSView construction/destruction

CWarpTPSView::CWarpTPSView()
	: m_pTransform(NULL)
	, m_pInverseTransform(NULL)
{
}

CWarpTPSView::~CWarpTPSView()
{
}

BOOL CWarpTPSView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSView drawing

void CWarpTPSView::OnDraw(CDC* pDC)
{
	CWarpTPSDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSView printing

BOOL CWarpTPSView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CWarpTPSView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CWarpTPSView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSView diagnostics

#ifdef _DEBUG
void CWarpTPSView::AssertValid() const
{
	CView::AssertValid();
}

void CWarpTPSView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWarpTPSDoc* CWarpTPSView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWarpTPSDoc)));
	return (CWarpTPSDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSView message handlers

void CWarpTPSView::OnInitialUpdate() 
{
	// set the document for the views
	m_arrViews[0].SetDocument(GetDocument());
	m_arrViews[0].SetDib(GetDocument()->GetImage(ImageRole::DestinationImage));
	m_arrViews[1].SetDocument(GetDocument());
	m_arrViews[1].SetDib(GetDocument()->GetImage(ImageRole::BlendedImage));
	m_arrViews[2].SetDocument(GetDocument());
	m_arrViews[2].SetDib(GetDocument()->GetImage(ImageRole::SourceImage));

	CView::OnInitialUpdate();	
}

CRect CWarpTPSView::GetRect(int nArea)
{
	CRect rectClient;
	GetClientRect(&rectClient);
	rectClient.DeflateRect(10, 10, 10, 10);
	//rectClient.right = rectClient.Height();

	// compute the rectangle
	CRect rect(rectClient);
	rect.left = 5 + (nArea % 2) * (rectClient.Width() / 2);
	rect.right = rect.left + (rectClient.Width() / 2 - 5);
	rect.top = 5 + nArea / 2 * rectClient.Height() / 2;
	rect.bottom = rect.top + rect.Width();	// assume square image

	return rect;
}

void CWarpTPSView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	CRect rect = GetRect(0);
	m_arrViews[0].MoveWindow(&rect);

	rect = GetRect(1);
	m_arrViews[1].MoveWindow(&rect);

	rect = GetRect(3);
	m_arrViews[2].MoveWindow(&rect);

	rect = GetRect(2);
	m_morphSlider.MoveWindow(&rect);
}

int CWarpTPSView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rect = GetRect(0);
	if (!m_arrViews[0].Create(NULL, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, rect, this, 0))
		return -1;

	rect = GetRect(1);
	if (!m_arrViews[1].Create(NULL, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, rect, this, 0))
		return -1;

	rect = GetRect(3);
	if (!m_arrViews[2].Create(NULL, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, rect, this, 0))
		return -1;

	if (!m_morphSlider.Create(IDD_MORPHSLIDER, this))
		return -1;

	rect = GetRect(2);
	m_morphSlider.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), 
      SWP_SHOWWINDOW);

	return 0;
}

void CWarpTPSView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// set the pointer to transform
	m_pTransform = GetDocument()->GetTransform();
	m_pInverseTransform = GetDocument()->GetInverseTransform();	

	if (m_pTransform)
	{
		CWaitCursor *pWaitCursor = NULL;
		if (m_pTransform->GetLandmarkCount() > 10)
		{
			pWaitCursor = new CWaitCursor;
		}

		// now resample the center image
		TRACE("OnUpdate/Resample");
		m_pTransform->Resample(GetDocument()->GetImage(ImageRole::SourceImage), 
			GetDocument()->GetImage(ImageRole::WarpedSourceImage), m_morphSlider.m_morphPercent);

		// update the inverse transform to document image 3
		m_pInverseTransform->Resample(GetDocument()->GetImage(ImageRole::DestinationImage), 
			GetDocument()->GetImage(ImageRole::WarpedDestinationImage), 1.0);
		
		GetDocument()->GetImage(ImageRole::BlendedImage)->CopyPixels(
			GetDocument()->GetImage(ImageRole::WarpedSourceImage));
		GetDocument()->GetImage(ImageRole::BlendedImage)->BlendPixels(
			GetDocument()->GetImage(ImageRole::WarpedDestinationImage), m_morphSlider.m_blendPercent);

		delete pWaitCursor;
	}

	ASSERT(m_arrViews[0].GetDib() == GetDocument()->GetImage(ImageRole::DestinationImage));
	m_arrViews[0].SetDib(GetDocument()->GetImage(ImageRole::DestinationImage));
	m_arrViews[0].SetTransform(m_pTransform, m_pInverseTransform, 0);

	ASSERT(m_arrViews[1].GetDib() == GetDocument()->GetImage(ImageRole::BlendedImage));
	m_arrViews[1].SetDib(GetDocument()->GetImage(ImageRole::BlendedImage));
	m_arrViews[1].SetTransform(m_pTransform, m_pInverseTransform, 0);

	ASSERT(m_arrViews[2].GetDib() == GetDocument()->GetImage(ImageRole::SourceImage));
	m_arrViews[2].SetDib(GetDocument()->GetImage(ImageRole::SourceImage));
	m_arrViews[2].SetTransform(m_pTransform, m_pInverseTransform, 1);

	m_morphSlider.m_pTransform = m_pTransform;
	m_morphSlider.m_pInverseTransform = m_pInverseTransform;
	m_morphSlider.m_pWarpedView = &m_arrViews[1];

	// set up the images
	//   TODO: give these more meaningful access via an enum
	m_morphSlider.m_pSourceImage = GetDocument()->GetImage(ImageRole::SourceImage);
	m_morphSlider.m_pWarpedSourceImage = GetDocument()->GetImage(ImageRole::WarpedSourceImage);
	m_morphSlider.m_pDestinationImage = GetDocument()->GetImage(ImageRole::DestinationImage);
	m_morphSlider.m_pWarpedDestinationImage = GetDocument()->GetImage(ImageRole::WarpedDestinationImage);
	m_morphSlider.m_pBlendedImage = GetDocument()->GetImage(ImageRole::BlendedImage);

	Invalidate(FALSE);
}

void CWarpTPSView::OnShowmarks() 
{
	m_arrViews[0].m_bDrawMarks = !m_arrViews[0].m_bDrawMarks;	
	m_arrViews[0].Invalidate(FALSE);
	m_arrViews[1].m_bDrawMarks = m_arrViews[0].m_bDrawMarks;	
	m_arrViews[1].Invalidate(FALSE);
	m_arrViews[2].m_bDrawMarks = m_arrViews[0].m_bDrawMarks;	
	m_arrViews[2].Invalidate(FALSE);
}

void CWarpTPSView::OnUpdateShowmarks(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_arrViews[0].m_bDrawMarks ? 1 : 0);
}

void CWarpTPSView::OnFileOpenImages() 
{
	CFileDialog imageFileOpen1(TRUE, "bmp", NULL, OFN_HIDEREADONLY, NULL, 
		::AfxGetMainWnd());

	CFileDialog imageFileOpen2(TRUE, "bmp", NULL, OFN_HIDEREADONLY, NULL, 
		::AfxGetMainWnd());

	if (IDOK == imageFileOpen1.DoModal() && IDOK == imageFileOpen2.DoModal())
	{
		// open the files
		if (GetDocument()->LoadImages(imageFileOpen1.GetPathName(), 
			imageFileOpen2.GetPathName()))
		{
			// set up some corner landmarks
			GetDocument()->InitCornerLandmarks();

			// invalidate everything
			Invalidate(TRUE);

			// redraw the window
			GetDocument()->UpdateAllViews(NULL);
		}
	}	
}
