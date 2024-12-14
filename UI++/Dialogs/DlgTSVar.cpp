// TSVarDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UI++.h"
#include "DlgTSVar.h"
#include "afxdialogex.h"
#include "TSVar.h"

UINT CTSVarGridCtrl::UWM_GRID_UPDATED = ::RegisterWindowMessage(_T("UWM_GRID_UPDATED-{ECD3A240-AB0B-47EF-83E4-78DA2D68DDF4}"));

// CTSVarDlg dialog

IMPLEMENT_DYNAMIC(CDlgTSVar, CDialog)

CDlgTSVar::CDlgTSVar(FTWCMLOG::ICMLogPtr pLog, CWnd* pParent /*=NULL*/)
	: m_pCMLog(pLog),
	CDialog(CDlgTSVar::IDD, pParent)
{

}

CDlgTSVar::~CDlgTSVar()
{
	//v_variableGrid.RemoveAll();
	//	
	//delete m_pEditableGroup;
	//delete m_pReadOnlyGroup;
}

void CDlgTSVar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_READONLYGRID, v_readonlyVarGrid);
	DDX_Control(pDX, IDC_EDITABLEGRID, v_editableVarGrid);
	DDX_Control(pDX, IDC_VARIABLETAB, m_varTabCtrl);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_GRIDRESET, m_resetAll);
}


BEGIN_MESSAGE_MAP(CDlgTSVar, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_VARIABLETAB, &CDlgTSVar::OnSelchangeVariabletab)
	ON_BN_CLICKED(IDOK, &CDlgTSVar::OnBnClickedOk)
	ON_REGISTERED_MESSAGE(CTSVarGridCtrl::UWM_GRID_UPDATED, OnGridValueChange)
	ON_BN_CLICKED(IDC_GRIDRESET, &CDlgTSVar::OnClickedGridreset)
	ON_BN_CLICKED(IDCANCEL, &CDlgTSVar::OnBnClickedCancel)
END_MESSAGE_MAP()


// CTSVarDlg message handlers


BOOL CDlgTSVar::OnInitDialog()
{
	CDialog::OnInitDialog();

	BOOL ok = m_resizer.Hook(this);

	m_resizer.SetShowResizeGrip();

	ASSERT(ok == TRUE);

	ok = m_resizer.SetAnchor(IDOK, ANCHOR_RIGHT | ANCHOR_BOTTOM);
	ASSERT(ok == TRUE);

	ok = m_resizer.SetAnchor(IDCANCEL, ANCHOR_RIGHT | ANCHOR_BOTTOM);
	ASSERT(ok == TRUE); 

	ok = m_resizer.SetAnchor(IDC_VARIABLETAB, ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_LEFT | ANCHOR_BOTTOM);
	ASSERT(ok == TRUE); 

	ok = m_resizer.SetAnchor(IDC_READONLYGRID, ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_LEFT | ANCHOR_BOTTOM);
	ASSERT(ok == TRUE); 

	ok = m_resizer.SetAnchor(IDC_EDITABLEGRID, ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_LEFT | ANCHOR_BOTTOM);
	ASSERT(ok == TRUE); 

	ok = m_resizer.SetAnchor(IDC_GRIDRESET, ANCHOR_LEFT | ANCHOR_BOTTOM);
	ASSERT(ok == TRUE);
		
	m_varTabCtrl.InsertItem(0, _T("Read-only"));
	m_varTabCtrl.InsertItem(1, _T("Editable"));

	v_readonlyVarGrid.EnableHeaderCtrl(TRUE, _T("Variable Name"), _T("Variable Value"));
	v_readonlyVarGrid.SetLeftColumnWidth();

	v_editableVarGrid.EnableHeaderCtrl(TRUE, _T("Variable Name"), _T("Variable Value"));
	v_editableVarGrid.SetLeftColumnWidth();

	//m_pReadOnlyGroup = new CMFCPropertyGridProperty(m_readOnly, 0, FALSE);
	//m_pEditableGroup = new CMFCPropertyGridProperty(m_editable, 0, FALSE);

	//v_variableGrid.AddProperty(m_pReadOnlyGroup, FALSE, TRUE);
	//v_variableGrid.AddProperty(m_pEditableGroup, FALSE, TRUE);

	v_readonlyVarGrid.PopulateGrid();
	v_editableVarGrid.PopulateGrid(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgTSVar::OnSelchangeVariabletab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int sel = m_varTabCtrl.GetCurSel();

	if(sel == 0)
	{
		v_readonlyVarGrid.ShowWindow(SW_SHOW);
		v_editableVarGrid.ShowWindow(SW_HIDE);
	}
	else
	{
		v_readonlyVarGrid.ShowWindow(SW_HIDE);
		v_editableVarGrid.ShowWindow(SW_SHOW);
	}

	*pResult = 0;
}

void CDlgTSVar::OnClickedGridreset()
{
	m_OK.EnableWindow(FALSE);
	m_resetAll.EnableWindow(FALSE);

	v_readonlyVarGrid.ResetOriginalValues();
	v_editableVarGrid.ResetOriginalValues();

}

void CDlgTSVar::OnBnClickedCancel()
{
	DeleteAllProperties();

	CDialog::OnCancel();
}

void CDlgTSVar::OnBnClickedOk()
{
	int count = v_editableVarGrid.GetPropertyCount();
	CMFCPropertyGridProperty* prop;

	for(int i = 0; i < count; i++)
	{
		prop = v_editableVarGrid.GetProperty(i);

		if (prop->IsModified())
			CTSEnv::Instance().Set(m_pCMLog, prop->GetName(), (_bstr_t)prop->GetValue());
	}

	DeleteAllProperties();

	CDialog::OnOK();
}

void CDlgTSVar::DeleteAllProperties(void)
{
	//v_readonlyVarGrid.GetPropertyCount();

	CMFCPropertyGridProperty* prop;
	
	while (v_readonlyVarGrid.GetPropertyCount() > 0)
	{
		prop = v_readonlyVarGrid.GetProperty(0);
		
		if (prop)
		{
			v_readonlyVarGrid.DeleteProperty(prop, FALSE, FALSE);

			delete prop;
		}

	}

	while (v_editableVarGrid.GetPropertyCount() > 0)
	{
		prop = v_editableVarGrid.GetProperty(0);
		
		if (prop)
		{
			v_editableVarGrid.DeleteProperty(prop, FALSE, FALSE);

			delete prop;
		}

	}

}

LRESULT CDlgTSVar::OnGridValueChange(WPARAM, LPARAM lParam)
{
	m_OK.EnableWindow();
	m_resetAll.EnableWindow();

	return 0;
}

void CTSVarGridCtrl::OnPropertyChanged(CMFCPropertyGridProperty* pProp) const
{
	GetParent()->SendMessage(UWM_GRID_UPDATED);

	return CMFCPropertyGridCtrl::OnPropertyChanged(pProp);
}

CTSVarGridCtrl::CTSVarGridCtrl()
{
}

CTSVarGridCtrl::~CTSVarGridCtrl()
{
}

void CTSVarGridCtrl::PopulateGrid(bool readOnly)
{
	CMFCPropertyGridProperty* pNewProperty;

	if(!CTSEnv::Instance().InTS() && CTSEnv::Instance().m_pNonTSVars != NULL)
	{
		if (CTSEnv::Instance().m_pNonTSVars->IsEmpty())
			return;
		
		CMapStringToString::CPair* pCurrent = CTSEnv::Instance().m_pNonTSVars->PGetFirstAssoc();

		while(pCurrent != NULL)
		{
			if(readOnly && pCurrent->key.Left(1) == _T("_"))
			{
				pNewProperty = new CMFCPropertyGridProperty(pCurrent->key, pCurrent->value);
				pNewProperty->AllowEdit(FALSE);
				pNewProperty->Enable(FALSE);
				this->AddProperty(pNewProperty);
			}
			else if(!readOnly && pCurrent->key.Left(1) != _T("_"))
			{
				pNewProperty = new CMFCPropertyGridProperty(pCurrent->key, pCurrent->value);
				pNewProperty->AllowEdit(TRUE);
				pNewProperty->Enable(TRUE);
				this->AddProperty(pNewProperty);
			}		

			pCurrent = CTSEnv::Instance().m_pNonTSVars->PGetNextAssoc(pCurrent);
		}
	}
	else if (CTSEnv::Instance().InTS())
	{
		_variant_t vars = CTSEnv::Instance().m_tsEnv->GetVariables();

		CComSafeArray<VARIANT> varArray(*(vars.parray));

		CString name, value;

		for (ULONG count = 0; count < varArray.GetCount(); count++)
		{
			_variant_t v = varArray.GetAt(count);
			name = v.bstrVal;
			value = (LPCTSTR)CTSEnv::Instance().m_tsEnv->GetValue(v.bstrVal);

			if(readOnly && name.Left(1) == _T("_"))
			{
				pNewProperty = new CMFCPropertyGridProperty(name, value);
				pNewProperty->AllowEdit(FALSE);
				pNewProperty->Enable(FALSE);
				this->AddProperty(pNewProperty);
			}
			else if(!readOnly && name.Left(1) != _T("_"))
			{
				pNewProperty = new CMFCPropertyGridProperty(name, value);
				pNewProperty->AllowEdit(TRUE);
				pNewProperty->Enable(TRUE);
				this->AddProperty(pNewProperty);
			}	

		}
	}

}

