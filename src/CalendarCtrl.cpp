/* Copyright (C) 2013 Webyog Inc

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA

*/

#include "CalendarCtrl.h"
#include "FrameWindowHelper.h"
#ifndef COMMUNITY
#include "formview.h"
#endif


/* ----------------------------------------------------------------------------------------
	Implementation of Calendar Control dialog box. 
	This dialog box is used when the user edits a 
	datetime, timestamp, date, or time field.
   --------------------------------------------------------------------------------------*/

CalendarCtrl::CalendarCtrl()
{
	m_hwnd = NULL;
	m_isForm = wyFalse;
	m_isDate = wyFalse;
	m_isResult = wyFalse;
}

CalendarCtrl::~CalendarCtrl()
{
}

#ifndef COMMUNITY
//Sets the parameters of the control in Formview
HWND
CalendarCtrl::CreateForm(htmlayout::dom::element hwndedit, FormView *pfv, wyBool isDate)
{
	m_edit = hwndedit;
	m_hwndparent = m_edit.get_element_hwnd(true);
	m_button = m_edit.next_sibling();
	
	m_pfv = pfv;
	
	if(pfv->m_pdv)
	{
		m_dv = pfv->m_pdv;
	}
	
	m_isDate = isDate;
	m_orgdata.SetAs(m_edit.text());
	m_isForm = wyTrue;
	m_rectCell = m_button.get_location();
	
	while(m_edit.get_ctl_type() != CTL_EDIT)
	{
		m_edit = m_edit.prev_sibling();
	}
	
	m_hwnd = Create();
	
	if(isDate)
	{
		ShowWindow(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),SW_HIDE);
	}
	
	PositionWindow(&m_rectCell);
	ShowWindow(m_hwnd,SW_SHOW);
    m_dv->m_hwndcal = m_hwnd;
    //m_dvb->m_hwndcal = m_hwnd;
	
	//set focus to the edit box
	m_edit.set_state(STATE_FOCUS);
	
	return m_hwnd;
}

#endif

//Sets the parameters of the control in Gridview
HWND
CalendarCtrl::CreateGrid(HWND hwndparent, wyWChar* olddat, DataViewBase *dvb, wyBool isDate, wyBool isResult)
{
	m_hwndparent= hwndparent;
	m_orgdata.SetAs(olddat);
	m_isDate = isDate;
	m_date.SetAs(olddat);
	m_dvb = dvb;
	m_isResult = isResult;

	m_hwnd = Create();
	
	if(isDate)
		ShowWindow(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),SW_HIDE);

	CustomGrid_GetSubItemRect(m_hwndparent, m_row, m_col, &m_rectCell);
 
	PositionWindow(&m_rectCell);
	ShowWindow(m_hwnd,SW_SHOW);
	
	return m_hwnd;
}

HWND
CalendarCtrl::CreateGrid(HWND hwndparent, DataView *dv, wyBool isDate, wyBool isResult)
{
	m_hwndparent= hwndparent;
    wyChar* temp = dv->m_data->m_rowarray->GetRowExAt(CustomGrid_GetCurSelRow(dv->m_hwndgrid))->m_row[CustomGrid_GetCurSelCol(dv->m_hwndgrid)];
	if(temp)
		m_orgdata.SetAs(temp);
	else
		m_orgdata.SetAs(L"(NULL)");
	m_isDate = isDate;
	m_date.SetAs(m_orgdata);
	m_dv	= dv;
	m_isResult = isResult;

	m_hwnd = Create();
	
	if(isDate)
		ShowWindow(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),SW_HIDE);

	CustomGrid_GetSubItemRect(m_hwndparent, m_row, m_col, &m_rectCell);
 
	PositionWindow(&m_rectCell);
	ShowWindow(m_hwnd,SW_SHOW);
	
	return m_hwnd;
}


//Function to create the dialogbox
HWND
CalendarCtrl::Create()
{
	 HWND hwnd = CreateDialogParam(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DATETIME), m_hwndparent, CalendarCtrl::CalendarCtrlProc, (LPARAM)this);
	 m_calproc = (WNDPROC)SetWindowLongPtr(GetDlgItem(m_hwnd,IDC_MONTHCALENDAR1),GWLP_WNDPROC,(LONG_PTR)CalendarCtrl::CalendarProc);
	 m_timeproc = (WNDPROC)SetWindowLongPtr(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),GWLP_WNDPROC,(LONG_PTR)CalendarCtrl::TimeProc);
	 SetWindowLongPtr(GetDlgItem(m_hwnd,IDC_MONTHCALENDAR1),GWLP_USERDATA,(LONG_PTR)this);
	 SetWindowLongPtr(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),GWLP_USERDATA,(LONG_PTR)this);
	 return hwnd;
}


// Calendar control Main Procedure

INT_PTR
CalendarCtrl::CalendarCtrlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CalendarCtrl* pcc = (CalendarCtrl*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(message)
	{
		case WM_INITDIALOG:
			pcc = (CalendarCtrl*)lParam;
			pcc->m_hwnd = hwnd;
			SetWindowLongPtr(hwnd, GWLP_USERDATA,lParam);
			LocalizeWindow(hwnd);
			pcc->InitCalendarValues();
			break;

		case WM_LBUTTONDOWN:
			pcc->OnLButtonDown(lParam);
			break;

		case WM_MOUSEWHEEL:
			SendMessage(hwnd, UM_ENDDIALOG,0,0);
			break;

		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					SendMessage(pcc->m_hwnd,WM_COMMAND,IDCANCEL,0);
					return 0;
				case VK_RETURN:
					SendMessage(pcc->m_hwnd,WM_COMMAND,IDOK,0);
					return 0;
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					pcc->OnClickOk();
					break;
		
				case IDCANCEL:
					pcc->OnClickCancel();
					break;
			}
			break;

		case UM_ENDDIALOG:
#ifndef COMMUNITY
			if(pcc->m_isForm)
			{
				pcc->m_button.set_state(0,STATE_DISABLED);
				DestroyWindow(pcc->m_hwnd);
				//pcc->m_dvb->m_hwndcal = NULL;
				pcc->m_edit.set_state(STATE_FOCUS);
			}
			else
#endif
			{
				if(pcc->m_hwnd)
					DestroyWindow(pcc->m_hwnd);
				//pcc->m_dvb->m_hwndcal = NULL;
			}
			if(lParam==1)
				delete pcc;
			return 0;

		case WM_NCDESTROY:
			break;
	}
	return 0;
}


//Subclassed Calender procedure to set the focus in the dialog box on click
wyInt32	CALLBACK CalendarCtrl::CalendarProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CalendarCtrl* pcc = (CalendarCtrl*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(message)
	{
		case WM_LBUTTONDOWN:
			SendMessage(pcc->m_hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(pcc->m_hwnd,IDC_DATETIMEPICKER1), TRUE);
			SetFocus(GetDlgItem(pcc->m_hwnd,IDC_DATETIMEPICKER1));
			break;
	}
	return pcc->m_calproc(hwnd,message,wParam,lParam);
}


//Subclassed Time procedure to deal with escape and enter keys
wyInt32	CALLBACK CalendarCtrl::TimeProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CalendarCtrl* pcc = (CalendarCtrl*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(message)
	{
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					SendMessage(pcc->m_hwnd,WM_COMMAND,IDCANCEL,0);
					return 0;

				case VK_RETURN:
					SendMessage(pcc->m_hwnd,message,wParam,lParam);
					return 0;
			}

	}

	return pcc->m_timeproc(hwnd,message,wParam,lParam);
}


// Convert the values from calendar to string
void
CalendarCtrl::ConvertCtrlValues()
{
    wyString    temp;
	m_date.Clear();
	SYSTEMTIME seltime;
	
	MonthCal_GetCurSel(GetDlgItem(m_hwnd,IDC_MONTHCALENDAR1), &seltime);
	m_date.AddSprintf("%04u-%02u-%02u",seltime.wYear,seltime.wMonth,seltime.wDay);
	
	if(!m_isDate)
	{
		DateTime_GetSystemtime(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1), &seltime);
		m_date.AddSprintf(" %02u:%02u:%02u",seltime.wHour,seltime.wMinute,seltime.wSecond);
	}
}

// convert the values from the grid into SYSTIME struct

void
CalendarCtrl::InitCalendarValues()
{
	wyString temp,temp2;
	m_row = CustomGrid_GetCurSelRow(m_hwndparent);
	m_col = CustomGrid_GetCurSelCol(m_hwndparent);
	wyChar tempchar[5]= "";
	
	
	if(m_orgdata.Substr(0,4)==NULL)
		strcpy(tempchar,"0");
	else
		strcpy(tempchar,m_orgdata.Substr(0,4));
	temp.Add(tempchar);
	m_datetime.wYear=temp.GetAsUInt32();
	temp.Clear();
	
	
	if(m_orgdata.Substr(5,2)==NULL)
		strcpy(tempchar,"0");
	else
		strcpy(tempchar,m_orgdata.Substr(5,2));
	temp.Add(tempchar);
	m_datetime.wMonth=temp.GetAsUInt32();
	temp.Clear();
	
	if(m_orgdata.Substr(8,2)==NULL)
		strcpy(tempchar,"0");
	else
		strcpy(tempchar,m_orgdata.Substr(8,2));
	temp.Add(tempchar);
	m_datetime.wDay=temp.GetAsUInt32();
	temp.Clear();
		
	
	if(m_orgdata.Substr(11,2)==NULL)
		strcpy(tempchar,"0");
	else
		strcpy(tempchar,m_orgdata.Substr(11,2));
	temp.Add(tempchar);
	m_datetime.wHour=temp.GetAsUInt32();
	temp.Clear();
	
	
	if(m_orgdata.Substr(14,2)==NULL)
		strcpy(tempchar,"0");
	else
		strcpy(tempchar,m_orgdata.Substr(14,2));
	temp.Add(tempchar);

			m_datetime.wMinute=temp.GetAsUInt32();
			temp.Clear();

			
	if(m_orgdata.Substr(17,2)==NULL)
		strcpy(tempchar,"0");
	else
		strcpy(tempchar,m_orgdata.Substr(17,2));
	temp.Add(tempchar);
	m_datetime.wSecond=temp.GetAsUInt32();
	m_datetime.wMilliseconds=0;
	
	
	MonthCal_SetCurSel(GetDlgItem(m_hwnd,IDC_MONTHCALENDAR1), &m_datetime);
	if(!m_isDate)
	{
		DateTime_SetFormat(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1), L"HH:mm:ss");
		DateTime_SetSystemtime(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),GDT_VALID, &m_datetime);
	}
}


//Position the control
void
CalendarCtrl::PositionWindow(RECT* rect)
{
   
    RECT        temprect = {0};
	RECT prect = {0} ;
    wyInt32     width, height, hmargin = 0, vmargin = 0;
	RECT calrect,timerect,okrect,cancelrect;
	GetClientRect(m_hwndparent, &prect);
	prect.right = prect.right - prect.left;
	prect.bottom = prect.bottom - prect.top;
	prect.left = 0;
	prect.top = 0;
    
    //Get all element handles
	GetClientRect(m_hwnd, &temprect);
	MonthCal_GetMinReqRect(GetDlgItem(m_hwnd,IDC_MONTHCALENDAR1),&calrect);  
	GetWindowRect(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),&timerect);
	GetWindowRect(GetDlgItem(m_hwnd,IDOK),&okrect);
	GetWindowRect(GetDlgItem(m_hwnd,IDCANCEL),&cancelrect);

    //calculate and modify the width and height based on the availabe space to best fit the window
	
	width = calrect.right - calrect.left;
	height = calrect.bottom - calrect.top + (okrect.bottom-okrect.top);
	hmargin = prect.right - rect->right;
	vmargin = prect.bottom - rect->top;
    
    if(height > vmargin)
	{
		if(rect->bottom - height > 0)
		{
			temprect.top = rect->bottom - height;
		}
		else
		{
			temprect.top = rect->top;
		}
	}
	else
	{
		temprect.top = rect->top;
	}
	
	
	if(width > hmargin)
    {
		if(rect->left - width > 0)
		{
			temprect.left = rect->left - width;
		}
		else
		{
			temprect.left = rect->left;
			if(temprect.top == rect->top)
			{
				temprect.top = rect->bottom;
			}
			else
			{
				if((rect->top - height)>0)
				{
					temprect.top = rect->top - height;
				}
				else
				{
					temprect.left = rect->left;
					if(temprect.top == rect->top)
					{
						temprect.top = rect->bottom;
					}
				}
			}
		}
	}
	else
		temprect.left = rect->right;

	//Postion all Controls

	MoveWindow(GetDlgItem(m_hwnd,IDC_MONTHCALENDAR1),
		-1,
		-1,
		width,
		calrect.bottom - calrect.top,
		TRUE);

	MoveWindow(GetDlgItem(m_hwnd,IDC_DATETIMEPICKER1),
		0,
		(calrect.bottom - calrect.top)-1,
		width - (2*(okrect.right-okrect.left)),
		okrect.bottom-okrect.top+1,
		TRUE);

	MoveWindow(GetDlgItem(m_hwnd,IDOK),
		width - (2*(okrect.right-okrect.left)),
		(calrect.bottom - calrect.top)-2,
		(okrect.right-okrect.left),
		okrect.bottom-okrect.top+1,
		TRUE);

	MoveWindow(GetDlgItem(m_hwnd,IDCANCEL),
		width - (okrect.right-okrect.left)-1,
		(calrect.bottom - calrect.top)-2,
		(okrect.right-okrect.left),
		okrect.bottom-okrect.top+1,
		TRUE);

	//Position the window
#ifndef COMMUNITY
	if(m_isForm)
		MoveWindow(m_hwnd, temprect.left+4, temprect.top, width, height, TRUE);
	else
#endif
		MoveWindow(m_hwnd, temprect.left, temprect.top, width, height, TRUE);
	
}

// handle OK Click
void 
CalendarCtrl::OnClickOk()
{
    wyInt32 row, col;
     
    ConvertCtrlValues();
    SendMessage(m_hwnd,UM_ENDDIALOG,0,0);
	CustomGrid_ApplyChanges(m_hwndparent);

    row = CustomGrid_GetCurSelRow(m_dv->m_hwndgrid);
    col = CustomGrid_GetCurSelCol(m_dv->m_hwndgrid);
    m_dv->m_gridwndproc(m_dv->m_hwndgrid, GVN_BEGINLABELEDIT, row, col);
	CustomGrid_SetText(m_dv->m_hwndgrid, row, col, m_date.GetString());
	m_dv->m_gridwndproc(m_dv->m_hwndgrid, GVN_ENDLABELEDIT, MAKELONG(row, col), (LPARAM)m_date.GetString());
	
	delete this;
}


// handle Cancel Click
void 
CalendarCtrl::OnClickCancel()
{
	SendMessage(m_hwnd,UM_ENDDIALOG,0,0);

	if(!m_isForm)
    {
		CustomGrid_CancelChanges(m_hwndparent);
    }

	delete this;
}

// handle LButtonDown on parent (formview)
void 
CalendarCtrl::OnLButtonDown(LPARAM lParam)
{
#ifndef COMMUNITY
	int xPos, yPos;
	RECT redit;
#endif
	if(!m_isForm)
	{
		SendMessage(m_hwnd,UM_ENDDIALOG,0,0);
	}
#ifndef COMMUNITY
	else
	{	
		xPos = GET_X_LPARAM(lParam); 
		yPos = GET_Y_LPARAM(lParam); 
		
		//Edit Box Rectangle
		redit = m_edit.get_location();
		if(!(xPos > redit.left && xPos < redit.right && yPos > redit.top && yPos < redit.bottom) && 
			!(xPos > m_rectCell.left && xPos < m_rectCell.right && yPos > m_rectCell.top && yPos < m_rectCell.bottom))
			SendMessage(m_hwnd, UM_ENDDIALOG,0,0);
	}
#endif
}

#ifndef COMMUNITY
///Public function to invoke the calendar popup from formview
HWND DisplayCalendarForm(htmlayout::dom::element hwndedit, FormView *pfv , wyBool isDate)
{
	CalendarCtrl *c = new CalendarCtrl();
	return c->CreateForm(hwndedit, pfv, isDate);
}
#endif

HWND DisplayCalendarGrid(HWND hwndParent, DataView *dv, wyBool isDate, wyBool isResult)
{
	CalendarCtrl *c = new CalendarCtrl();	
	return c->CreateGrid(hwndParent, dv, isDate, isResult);
}
