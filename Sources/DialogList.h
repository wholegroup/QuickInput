#pragma once

#define TRANSPARENT_RGB RGB(0x00, 0x00, 0x00)

typedef CArray<CDialog*, CDialog*> CListDialog;
typedef CArray<CString, CString>   CListCaption;

//////////////////////////////////////////////////////////////////////////
// описание класса списка диалогов
//
class CDialogList : public CListBox
{
	DECLARE_MESSAGE_MAP()

	public:

		//
		CDialogList();

		//
		virtual ~CDialogList();

	protected:

		UINT         m_uiItemHeight;    // высота ячеек
		DWORD        m_dwBmpHeight;     // высота битмапа
		DWORD        m_dwBmpWidth;      // ширина битмапа
		DWORD        m_dwBackground;    // цвет фона элемента
		DWORD        m_dwColorItem;     // цвет текста элемента
		DWORD        m_dwColorItemHlt;  // цвет текста элемента
		DWORD        m_dwSelected;      // цвет фона выделенного элемента
		CBrush       m_cBackground;     // кисть для закрашивания общего фона
		CImageList   m_listImg;         // список битмапов
		CListDialog  m_listDlg;         // список диалогов
		CListCaption m_listCaption;     // список заголовков
		CWnd         *m_wndCaption;     // указатель на окно с названием диалога
	
	public:

		//
		VOID AddItem(TCHAR *tcItem, CDialog *cDlg = NULL, TCHAR *tcCaption = NULL, 
			DWORD dwResID = NULL, UINT uiType = IMAGE_ICON);

		//
		VOID SetItemsHeight(UINT uiItemHeight) {m_uiItemHeight = uiItemHeight;}

		//
		VOID SetCaptionWnd(CWnd *wndCaption) {m_wndCaption = wndCaption;}

		//
		afx_msg VOID SelectDialog();

	protected:

		//
		virtual VOID DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

		//
		virtual VOID MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

		//
		virtual VOID PreSubclassWindow();

		//
		afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
};


//////////////////////////////////////////////////////////////////////////
// враппер для прозрачной работы с MemDC
//
class CMyMemDC : public CDC
{
	private:	

		CBitmap  m_bitmap;    // Offscreen bitmap
		CBitmap* m_oldBitmap; // bitmap originally found in CMemDC
		CDC*     m_pDC;       // Saves CDC passed in constructor
		CRect    m_rect;      // Rectangle of drawing area.
		BOOL     m_bMemDC;    // TRUE if CDC really is a Memory DC.

	public:

		//
		CMyMemDC(CDC* pDC, const CRect* pRect = NULL):CDC()
		{
			ASSERT(pDC != NULL); 

			m_pDC       = pDC;
			m_oldBitmap = NULL;
			m_bMemDC    = !pDC->IsPrinting();

			if (NULL == pRect)
			{
				pDC->GetClipBox(&m_rect);
			}
			else
			{
				m_rect = *pRect;
			}

			if (m_bMemDC)
			{
				CreateCompatibleDC(pDC);
				pDC->LPtoDP(&m_rect);
	
				m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
				m_oldBitmap = SelectObject(&m_bitmap);

				SetMapMode(pDC->GetMapMode());

				SetWindowExt(pDC->GetWindowExt());
				SetViewportExt(pDC->GetViewportExt());

				pDC->DPtoLP(&m_rect);
				SetWindowOrg(m_rect.left, m_rect.top);
			}
			else
			{
				m_bPrinting = pDC->m_bPrinting;
				m_hDC       = pDC->m_hDC;
				m_hAttribDC = pDC->m_hAttribDC;
			}
		}

		//
		virtual ~CMyMemDC()	
		{		
			// Copy the offscreen bitmap onto the screen.
			if (m_bMemDC)
			{
				m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
					this, m_rect.left, m_rect.top, SRCCOPY);			

				SelectObject(m_oldBitmap);		
			}
			else
			{
				m_hDC = m_hAttribDC = NULL;
			}	
		}

		//
		CMyMemDC* operator->() 
		{
			return this;
		}	

		//
		operator CMyMemDC*() 
		{
			return this;
		}
};
