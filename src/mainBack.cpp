/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#include <Windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include "resource1.h"
#include "pxcsession.h"
#include "pxccapture.h"
#include "pxchandmodule.h"
#include "pxchanddata.h"
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <string>


#define IDC_STATUS 10000
#define ID_DEVICEX 21000
#define ID_MODULEX 22000
#define IDSMOOTHERX 23000



#define MAX_NUM_OF_HANDS 2

#define SMOOTHER_STABILIZER_VALUE L"40.0"
#define SMOOTHER_WEIGHTED_VALUE L"10.0"
#define SMOOTHER_SPRING_VALUE L"5.0"
#define SMOOTHER_QUADRATIC_VALUE L"0.1"


HINSTANCE   g_hInst = 0;
PXCSession *g_session = 0;
pxcCHAR g_file[1024] = { 0 };

/* Panel Bitmap */
HBITMAP     g_bitmap = 0;

/* None Gesture */
HBITMAP     g_none = 0;

/* Threading control */
volatile bool g_running = false;
volatile bool g_stop = true;

HANDLE m_thread = NULL;

bool snapshotEnabled = false;
bool showNormalizedSkeleton = false;
bool showExtremityPoint = false;
bool noRender = false;
bool m_useSmoother = false;
bool isGestureListInit = false;

float maxRangeValue = 1000;

/* joints colors */
HPEN red = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
HPEN green = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
HPEN blue = CreatePen(PS_SOLID, 3, RGB(0, 102, 204));
HPEN yellow = CreatePen(PS_SOLID, 3, RGB(245, 245, 0));
HPEN cyan = CreatePen(PS_SOLID, 3, RGB(0, 245, 245));
HPEN orange = CreatePen(PS_SOLID, 3, RGB(255, 184, 112));
HPEN black = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
HPEN boneColor = CreatePen(PS_SOLID, 3, RGB(51, 153, 255));

pxcI32* m_buffer = NULL;
pxcI32 m_bufferSize = 0;

unsigned char *m_charBuffer = NULL;

/* Control Layout */
int g_controls[] = { IDC_DEPTH, IDC_LABELMAP, IDC_SCALE, IDC_MIRROR, IDC_GEONODE, IDC_GESTURE, IDC_PARAMS, IDC_GESTURE2, IDC_GESTURE1, ID_START, ID_STOP, IDC_STATIC, IDC_EDITSPIN, IDC_SPIN, IDC_SVALUE, IDC_CMB_GESTURE, IDC_GestureLeftStatus, IDC_EDIT2, IDC_INFOBOX };
RECT g_layout[3 + sizeof(g_controls) / sizeof(g_controls[0])];

void convertTo8bpp(unsigned short * pSrc, int iSize, unsigned char * pDst);

void SaveLayout(HWND hwndDlg) {
	GetClientRect(hwndDlg, &g_layout[0]);
	ClientToScreen(hwndDlg, (LPPOINT)&g_layout[0].left);
	ClientToScreen(hwndDlg, (LPPOINT)&g_layout[0].right);
	GetWindowRect(GetDlgItem(hwndDlg, IDC_PANEL), &g_layout[1]);
	GetWindowRect(GetDlgItem(hwndDlg, IDC_STATUS), &g_layout[2]);

	for (int i = 0; i<sizeof(g_controls) / sizeof(g_controls[0]); i++)
		GetWindowRect(GetDlgItem(hwndDlg, g_controls[i]), &g_layout[3 + i]);
}

void RedoLayout(HWND hwndDlg) {
	RECT rect;
	GetClientRect(hwndDlg, &rect);


	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_STATUS);

	/* Status */
	SetWindowPos(hwndStatus, hwndDlg,
		0,
		rect.bottom - (g_layout[2].bottom - g_layout[2].top),
		rect.right - rect.left,
		(g_layout[2].bottom - g_layout[2].top), SWP_NOZORDER);



	/* Panel */
	SetWindowPos(GetDlgItem(hwndDlg, IDC_PANEL), hwndDlg,
		(g_layout[1].left - g_layout[0].left),
		(g_layout[1].top - g_layout[0].top),
		rect.right - (g_layout[1].left - g_layout[0].left) - (g_layout[0].right - g_layout[1].right),
		rect.bottom - (g_layout[1].top - g_layout[0].top) - (g_layout[0].bottom - g_layout[1].bottom),
		SWP_NOZORDER);

	/* Buttons & CheckBoxes */
	for (int i = 0; i<sizeof(g_controls) / sizeof(g_controls[0]); i++) {
		SetWindowPos(GetDlgItem(hwndDlg, g_controls[i]), hwndDlg,
			rect.right - (g_layout[0].right - g_layout[3 + i].left),
			(g_layout[3 + i].top - g_layout[0].top),
			(g_layout[3 + i].right - g_layout[3 + i].left),
			(g_layout[3 + i].bottom - g_layout[3 + i].top),
			SWP_NOZORDER);
	}
}

void setMaxRangeValue(float value)
{
	maxRangeValue = value;
}

static void PopulateDevice(HMENU menu) {
	DeleteMenu(menu, 0, MF_BYPOSITION);

	PXCSession::ImplDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.group = PXCSession::IMPL_GROUP_SENSOR;
	desc.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;
	HMENU menu1 = CreatePopupMenu();
	for (int i = 0, k = ID_DEVICEX;; i++) {
		PXCSession::ImplDesc desc1;
		if (g_session->QueryImpl(&desc, i, &desc1)<PXC_STATUS_NO_ERROR) break;
		PXCCapture *capture = 0;
		if (g_session->CreateImpl<PXCCapture>(&desc1, &capture)<PXC_STATUS_NO_ERROR) continue;
		for (int j = 0;; j++) {
			PXCCapture::DeviceInfo dinfo;
			if (capture->QueryDeviceInfo(j, &dinfo)<PXC_STATUS_NO_ERROR) break;
			AppendMenu(menu1, MF_STRING, k++, dinfo.name);
		}
		capture->Release();
	}
	CheckMenuRadioItem(menu1, 0, GetMenuItemCount(menu1), 0, MF_BYPOSITION);
	InsertMenu(menu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)menu1, L"Device");
}

static int GetChecked(HMENU menu) {
	for (int i = 0; i<GetMenuItemCount(menu); i++)
		if (GetMenuState(menu, i, MF_BYPOSITION)&MF_CHECKED) return i;
	return 0;
}

pxcCHAR* GetCheckedDevice(HWND hwndDlg) {
	HMENU menu = GetSubMenu(GetMenu(hwndDlg), 0);	// ID_DEVICE
	static pxcCHAR line[256];
	GetMenuString(menu, GetChecked(menu), line, sizeof(line) / sizeof(pxcCHAR), MF_BYPOSITION);
	return line;
}

static void PopulateModule(HMENU menu) {

	DeleteMenu(menu, 1, MF_BYPOSITION);

	PXCSession::ImplDesc desc, desc1;
	memset(&desc, 0, sizeof(desc));
	desc.cuids[0] = PXCHandModule::CUID;
	HMENU menu1 = CreatePopupMenu();
	int i;
	for (i = 0;; i++) {
		if (g_session->QueryImpl(&desc, i, &desc1)<PXC_STATUS_NO_ERROR) break;
		AppendMenu(menu1, MF_STRING, ID_MODULEX + i, desc1.friendlyName);
	}
	CheckMenuRadioItem(menu1, 0, i, 0, MF_BYPOSITION);
	InsertMenu(menu, 1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)menu1, L"Module");
}

pxcCHAR *GetCheckedModule(HWND hwndDlg) {
	HMENU menu = GetSubMenu(GetMenu(hwndDlg), 1);	// ID_MODULE
	static pxcCHAR line[256];
	GetMenuString(menu, GetChecked(menu), line, sizeof(line) / sizeof(pxcCHAR), MF_BYPOSITION);
	return line;
}




static DWORD WINAPI ThreadProc(LPVOID arg) {
	void SimplePipeline(HWND hwndDlg);
	SimplePipeline((HWND)arg);
	PostMessage((HWND)arg, WM_COMMAND, ID_STOP, 0);
	g_running = false;
	CloseHandle(m_thread);
	return 0;
}

static DWORD WINAPI ThreadProcAdvanced(LPVOID arg) {
	void AdvancedPipeline(HWND hwndDlg);
	AdvancedPipeline((HWND)arg);
	PostMessage((HWND)arg, WM_COMMAND, ID_STOP, 0);
	g_running = false;
	CloseHandle(m_thread);
	return 0;
}

void SetStatus(HWND hwndDlg, pxcCHAR *line) {
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_STATUS);
	SetWindowText(hwndStatus, line);
}


void SetMocapData(HWND hwndDlg, pxcCHAR *line) {

	// get edit control from dialog
	HWND hwndOutput = GetDlgItem(hwndDlg, IDC_MOCAPDATA);

	if (line == NULL)
	{
		SetWindowText(hwndOutput, L"");
		return;
	}

	// get the current selection
	DWORD StartPos, EndPos;
	SendMessage(hwndOutput, EM_GETSEL, reinterpret_cast<WPARAM>(&StartPos), reinterpret_cast<WPARAM>(&EndPos));

	// move the caret to the end of the text
	int outLength = GetWindowTextLength(hwndOutput);

	if (outLength >= 2000)
	{
		SetWindowText(hwndOutput, L"");
	}

	SendMessage(hwndOutput, EM_SETSEL, outLength, outLength);


	// insert the text at the new caret position
	SendMessage(hwndOutput, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(line));

	// restore the previous selection
	SendMessage(hwndOutput, EM_SETSEL, StartPos, EndPos);
}

void SetInfoBox(HWND hwndDlg, pxcCHAR *line) {

	// get edit control from dialog
	HWND hwndOutput = GetDlgItem(hwndDlg, IDC_INFOBOX);

	if (line == NULL)
	{
		SetWindowText(hwndOutput, L"");
		return;
	}

	// get the current selection
	DWORD StartPos, EndPos;
	SendMessage(hwndOutput, EM_GETSEL, reinterpret_cast<WPARAM>(&StartPos), reinterpret_cast<WPARAM>(&EndPos));

	// move the caret to the end of the text
	int outLength = GetWindowTextLength(hwndOutput);

	if (outLength >= 2000)
	{
		SetWindowText(hwndOutput, L"");
	}

	SendMessage(hwndOutput, EM_SETSEL, outLength, outLength);


	// insert the text at the new caret position
	SendMessage(hwndOutput, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(line));

	// restore the previous selection
	SendMessage(hwndOutput, EM_SETSEL, StartPos, EndPos);
}


void SetGestureLeftStatus(HWND hwndDlg, pxcCHAR *line) {
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_GestureLeftStatus);
	SetWindowText(hwndStatus, line);
}

void SetGestureRightStatus(HWND hwndDlg, pxcCHAR *line) {
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_GestureRightStatus);
	SetWindowText(hwndStatus, line);
}


bool IsCMBGestureInit()
{
	return isGestureListInit;
}

void SetCMBGesturePos(HWND hwndDlg)
{
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_CMB_GESTURE);
	RECT rect;
	GetClientRect(hwndStatus, &rect);
	MapDialogRect(hwndStatus, &rect);
	SetWindowPos(hwndStatus, 0, 0, 0, rect.right, (3 + 1) * rect.bottom, SWP_NOMOVE);
	ComboBox_AddString(hwndStatus, L"");
	isGestureListInit = true;
}

int GetSelectedGesture(HWND hwndDlg)
{
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_CMB_GESTURE);
	int selectedIndex = ComboBox_GetCurSel(hwndStatus);
	return selectedIndex;
}




void AddCMBItem(HWND hwndDlg, pxcCHAR *line) {

	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_CMB_GESTURE);
	ComboBox_AddString(hwndStatus, line);
}

void EnableCMBItem(HWND hwndDlg, pxcBool enable) {
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_CMB_GESTURE);
	ComboBox_Enable(hwndStatus, enable);
}

void SetFPSStatus(HWND hwndDlg, pxcCHAR *line) {
	HWND hwndStatus = GetDlgItem(hwndDlg, IDC_STATIC);
	SetWindowText(hwndStatus, line);
}



int GetFramesToRecord(HWND hwndDlg) {
	HWND hwndValue = GetDlgItem(hwndDlg, IDC_EDITSPIN);
	LPWSTR str = new TCHAR[50];
	GetWindowText(hwndValue, str, 50);

	int number = _wtoi(str);

	delete[] str;

	if (number == 0)
		return -1;
	else
		return number;
}

void setFramesRecordBox(HWND hwndDlg, int frameNumber) {
	HWND hwndValue = GetDlgItem(hwndDlg, IDC_EDITSPIN);
	wchar_t line[256];
	swprintf_s(line, L"%d", frameNumber);
	SetWindowText(hwndValue, line);
}

//---------------------- Viewer Options
bool GetLabelmapState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_LABELMAP))&BST_CHECKED);
}

bool GetDepthState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_DEPTH))&BST_CHECKED);
}

bool GetAlertState(HWND hwndDlg)
{
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_PARAMS))&BST_CHECKED);
}

bool GetPlaybackState(HWND hwndDlg) {
	return (GetMenuState(GetMenu(hwndDlg), ID_MODE_PLAYBACK, MF_BYCOMMAND)&MF_CHECKED) != 0;
}

bool GetRecordState(HWND hwndDlg) {
	return (GetMenuState(GetMenu(hwndDlg), ID_MODE_RECORD, MF_BYCOMMAND)&MF_CHECKED) != 0;
}
//------------------------- Gesture Parameter Selections
bool GetPositionWorldState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_POSITIONWORLD))&BST_CHECKED);
}
bool GetPositionImageState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_POSITIONIMAGE))&BST_CHECKED);
}
bool GetGlobalRotationState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_GLOBALROTATION))&BST_CHECKED);
}
bool GetLocalRotationState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_LOCALROTATION))&BST_CHECKED);
}
bool GetSpeedState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_SPEED))&BST_CHECKED);
}
bool GetRotationConfidenceState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_CONFIDENCE))&BST_CHECKED);
}
bool GetRotationFoldednessState(HWND hwndDlg) {
	return (Button_GetState(GetDlgItem(hwndDlg, IDC_FOLDEDNESS))&BST_CHECKED);
}
void ClearBuffer(PXCImage::ImageInfo info)
{

	int bufferSize = info.width * info.height;
	if (bufferSize != m_bufferSize)
	{
		m_bufferSize = bufferSize;
		if (m_buffer) delete[] m_buffer;
		m_buffer = new pxcI32[m_bufferSize];
		if (m_charBuffer) delete[] m_charBuffer;
		m_charBuffer = new unsigned char[info.width*info.height * 4];
	}

	if (m_bufferSize>0){
		memset(m_buffer, 0, m_bufferSize*sizeof(pxcI32));
	}
}

void SetHandsMask(PXCImage* image, pxcI32 id)
{
	PXCImage::ImageInfo info = image->QueryInfo();
	PXCImage::ImageData data;
	if (image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_Y8, &data) == PXC_STATUS_NO_ERROR)
	{
		BITMAPINFO binfo;
		memset(&binfo, 0, sizeof(binfo));
		binfo.bmiHeader.biWidth = (int)info.width;
		binfo.bmiHeader.biHeight = -(int)info.height;
		binfo.bmiHeader.biBitCount = 32;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biCompression = BI_RGB;

		int bufferSize = info.width * info.height;
		if (bufferSize != m_bufferSize)
		{
			m_bufferSize = bufferSize;
			if (m_buffer) delete[] m_buffer;
			m_buffer = new pxcI32[m_bufferSize];
			memset(m_buffer, 0, m_bufferSize*sizeof(pxcI32));
		}

		pxcI32 pitch = data.pitches[0];
		pxcBYTE* row = (pxcBYTE*)data.planes[0];
		pxcI32* dst = m_buffer;
		for (int j = 0; j<-binfo.bmiHeader.biHeight; j++){
			for (int j = 0; j<binfo.bmiHeader.biWidth; j++)
			{
				if (row[j] != 0){
					unsigned char val = id * 100;
					unsigned char* rgb = (unsigned char*)dst;
					rgb[0] = val;
					rgb[1] = val;
					rgb[2] = val;
					rgb[3] = 255;
				}
				dst++;
			}
			row += pitch;
		}

		image->ReleaseAccess(&data);
	}
}

void DrawBitmap(HWND hwndDlg, PXCImage *image) {
	if (g_bitmap) {
		DeleteObject(g_bitmap);
		g_bitmap = 0;
	}
	PXCImage::ImageInfo info = image->QueryInfo();
	PXCImage::ImageData data;

	if (info.format == PXCImage::PIXEL_FORMAT_DEPTH)
	{
		if (image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &data) >= PXC_STATUS_NO_ERROR)
		{
			HWND hwndPanel = GetDlgItem(hwndDlg, IDC_PANEL);
			HDC dc = GetDC(hwndPanel);
			if (dc == NULL){ return; }
			BITMAPINFO binfo;
			memset(&binfo, 0, sizeof(binfo));
			binfo.bmiHeader.biWidth = (int)info.width;
			binfo.bmiHeader.biHeight = -(int)info.height;
			binfo.bmiHeader.biBitCount = 32;
			binfo.bmiHeader.biPlanes = 1;
			binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			binfo.bmiHeader.biCompression = BI_RGB;

			convertTo8bpp((unsigned short*)data.planes[0], info.width*info.height, m_charBuffer);
			g_bitmap = CreateDIBitmap(dc, &binfo.bmiHeader, CBM_INIT, m_charBuffer, &binfo, DIB_RGB_COLORS);

			ReleaseDC(hwndPanel, dc);
			image->ReleaseAccess(&data);
		}
	}
	if (info.format == PXCImage::PIXEL_FORMAT_Y8)
	{
		HWND hwndPanel = GetDlgItem(hwndDlg, IDC_PANEL);
		HDC dc = GetDC(hwndPanel);
		if (dc == NULL){ return; }
		BITMAPINFO binfo;
		memset(&binfo, 0, sizeof(binfo));
		binfo.bmiHeader.biWidth = (int)info.width;
		binfo.bmiHeader.biHeight = -(int)info.height;
		binfo.bmiHeader.biBitCount = 32;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biCompression = BI_RGB;

		g_bitmap = CreateDIBitmap(dc, &binfo.bmiHeader, CBM_INIT, m_buffer, &binfo, DIB_RGB_COLORS);
		ReleaseDC(hwndPanel, dc);
	}

}

void convertTo8bpp(unsigned short * pSrc, int iSize, unsigned char * pDst)
{
	float fMaxValue = maxRangeValue;
	unsigned char cVal;
	for (int i = 0; i < iSize; i++, pSrc++, pDst += 4)
	{
		cVal = (unsigned char)((*pSrc) / fMaxValue * 255);
		if (cVal != 0)
			cVal = 255 - cVal;

		pDst[0] = cVal;
		pDst[1] = cVal;
		pDst[2] = cVal;
		pDst[3] = 255;
	}
}

static HBITMAP ResizeBitmap(HWND hwnd, HBITMAP bitmap) {
	RECT rect;
	GetClientRect(hwnd, &rect);

	BITMAP bm;
	GetObject(bitmap, sizeof(BITMAP), &bm);

	HDC dc = GetDC(hwnd);
	if (dc == NULL){
		return NULL;
	}
	HDC dc2 = CreateCompatibleDC(dc);
	if (dc2 == NULL){
		ReleaseDC(hwnd, dc);
		return NULL;
	}
	SelectObject(dc2, bitmap);

	HDC dc3 = CreateCompatibleDC(dc);
	if (dc3 == NULL){
		DeleteDC(dc2);
		ReleaseDC(hwnd, dc);
		return NULL;
	}

	HBITMAP bitmap2 = CreateCompatibleBitmap(dc, rect.right, rect.bottom);

	SelectObject(dc3, bitmap2);
	ReleaseDC(hwnd, dc);

	SetStretchBltMode(dc3, HALFTONE);
	StretchBlt(dc3, 0, 0, rect.right, rect.bottom, dc2, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

	DeleteDC(dc3);
	DeleteDC(dc2);
	return bitmap2;
}

static RECT GetResizeRect(RECT rc, BITMAP bm) { /* Keep the aspect ratio */
	RECT rc1;
	float sx = (float)rc.right / (float)bm.bmWidth;
	float sy = (float)rc.bottom / (float)bm.bmHeight;
	float sxy = sx<sy ? sx : sy;
	rc1.right = (int)(bm.bmWidth*sxy);
	rc1.left = (rc.right - rc1.right) / 2 + rc.left;
	rc1.bottom = (int)(bm.bmHeight*sxy);
	rc1.top = (rc.bottom - rc1.bottom) / 2 + rc.top;
	return rc1;
}

void UpdatePanel(HWND hwndDlg) {
	if (!g_bitmap) return;

	HWND panel = GetDlgItem(hwndDlg, IDC_PANEL);
	RECT rc;
	GetClientRect(panel, &rc);

	HDC dc = GetDC(panel);
	if (dc == NULL){
		return;
	}

	HDC dc2 = CreateCompatibleDC(dc);
	if (dc2 == NULL){
		ReleaseDC(hwndDlg, dc);
		return;
	}

	HBITMAP bitmap = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
	if (bitmap == NULL)
	{
		DeleteDC(dc2);
		ReleaseDC(hwndDlg, dc);
		return;
	}

	SelectObject(dc2, bitmap);
	FillRect(dc2, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
	SetStretchBltMode(dc2, HALFTONE);

	/* Draw the main window */
	HDC dc3 = CreateCompatibleDC(dc);
	if (dc3 == NULL){
		DeleteDC(dc2);
		DeleteObject(bitmap);
		ReleaseDC(hwndDlg, dc);
		return;
	}
	SelectObject(dc3, g_bitmap);
	BITMAP bm;
	GetObject(g_bitmap, sizeof(BITMAP), &bm);

	bool scale = Button_GetState(GetDlgItem(hwndDlg, IDC_SCALE))&BST_CHECKED;
	bool mirror = Button_GetState(GetDlgItem(hwndDlg, IDC_MIRROR))&BST_CHECKED;
	if (mirror) {
		if (scale) {
			RECT rc1 = GetResizeRect(rc, bm);
			StretchBlt(dc2, rc1.left + rc1.right - 1, rc1.top, -rc1.right, rc1.bottom, dc3, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		}
		else {
			StretchBlt(dc2, bm.bmWidth - 1, 0, -bm.bmWidth, bm.bmHeight, dc3, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		}
	}
	else {
		if (scale) {
			RECT rc1 = GetResizeRect(rc, bm);
			StretchBlt(dc2, rc1.left, rc1.top, rc1.right, rc1.bottom, dc3, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		}
		else {
			BitBlt(dc2, 0, 0, rc.right, rc.bottom, dc3, 0, 0, SRCCOPY);
		}
	}

	DeleteDC(dc3);
	DeleteDC(dc2);
	ReleaseDC(hwndDlg, dc);

	HBITMAP bitmap2 = (HBITMAP)SendMessage(panel, STM_GETIMAGE, 0, 0);
	if (bitmap2) DeleteObject(bitmap2);
	SendMessage(panel, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
	InvalidateRect(panel, 0, TRUE);

	DeleteObject(bitmap);
}

void DrawJoints(HWND hwndDlg, PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS], PXCHandData::ExtremityData extremitiesPointsNodes[2][PXCHandData::NUMBER_OF_EXTREMITIES])
{
	if (!g_bitmap) return;
	bool jointNode = Button_GetState(GetDlgItem(hwndDlg, IDC_GEONODE))&BST_CHECKED;
	bool skeletonNode = Button_GetState(GetDlgItem(hwndDlg, IDC_PARAMS))&BST_CHECKED;



	HWND hwndPanel = GetDlgItem(hwndDlg, IDC_PANEL);
	HDC dc = GetDC(hwndPanel);
	if (dc == NULL){
		return;
	}
	HDC dc2 = CreateCompatibleDC(dc);
	if (dc2 == NULL){
		ReleaseDC(hwndDlg, dc);
		return;
	}
	SelectObject(dc2, g_bitmap);

	BITMAP bm;
	GetObject(g_bitmap, sizeof(bm), &bm);

	if (showExtremityPoint == true)
	{
		int sz = 2; //joint circle size
		for (int i = 0; i < MAX_NUM_OF_HANDS; ++i)
		{
			for (int j = 0; j < PXCHandData::NUMBER_OF_EXTREMITIES; ++j)
			{
				SelectObject(dc2, red);
				int pointImageX = (int)extremitiesPointsNodes[i][j].pointImage.x;
				int pointImageY = (int)extremitiesPointsNodes[i][j].pointImage.y;
				Arc(dc2, pointImageX - sz, pointImageY - sz, pointImageX + sz, pointImageY + sz, pointImageX + sz, pointImageY + sz, pointImageX + sz, pointImageY + sz);
			}
		}

	}

	if (jointNode || skeletonNode)
	{
		for (int i = 0; i < MAX_NUM_OF_HANDS; ++i)
		{
			int wristX = (int)nodes[i][0].positionImage.x;
			int wristY = (int)nodes[i][0].positionImage.y;

			SelectObject(dc2, boneColor);

			MoveToEx(dc2, wristX, wristY, 0);

			//Draw Bones
			if (skeletonNode && !m_useSmoother)
			{
				for (int j = 1; j < PXCHandData::NUMBER_OF_JOINTS; ++j)
				{
					if (nodes[i][j].confidence == 0) continue;

					int x = (int)nodes[i][j].positionImage.x;
					int y = (int)nodes[i][j].positionImage.y;

					if (j == 2 || j == 6 || j == 10 || j == 14 || j == 18)
					{
						MoveToEx(dc2, wristX, wristY, 0);
					}

					LineTo(dc2, x, y);
					MoveToEx(dc2, x, y, 0);

				}//end for joints
			}
			//Draw Joints
			if (jointNode && !m_useSmoother)
			{
				for (int j = 0; j < PXCHandData::NUMBER_OF_JOINTS; ++j)
				{
					if (nodes[i][j].confidence == 0) continue;

					int sz = 1; //joint circle size

					int x = (int)nodes[i][j].positionImage.x;
					int y = (int)nodes[i][j].positionImage.y;

					//Wrist
					if (j == 0)
					{
						SelectObject(dc2, black);
					}
					//Center
					if (j == 1)
					{
						SelectObject(dc2, red);
						sz += 4;
					}
					//Thumb
					if (j == 2 || j == 3 || j == 4 || j == 5)
					{
						SelectObject(dc2, green);
					}
					//Index Finger
					if (j == 6 || j == 7 || j == 8 || j == 9)
					{
						SelectObject(dc2, blue);
					}
					//Finger
					if (j == 10 || j == 11 || j == 12 || j == 13)
					{
						SelectObject(dc2, yellow);
					}
					//Ring Finger
					if (j == 14 || j == 15 || j == 16 || j == 17)
					{
						SelectObject(dc2, cyan);
					}
					//Pinkey
					if (j == 18 || j == 19 || j == 20 || j == 21)
					{
						SelectObject(dc2, orange);
					}

					//if finger tip draw larger circle
					if (j == 5 || j == 9 || j == 13 || j == 17 || j == 21)
					{
						sz += 3;
					}

					MoveToEx(dc2, x, y, 0);
					Arc(dc2, x - sz, y - sz, x + sz, y + sz, x + sz, y + sz, x + sz, y + sz);

				}//end for joints					
			}
		}//end if jointNodes


		//Use Smoother
		if (m_useSmoother)
		{
			for (int j = 9; j < 11; ++j)
			{
				if (nodes[0][j].confidence == 0) continue;

				int sz = 1; //joint circle size

				int x = (int)nodes[0][j].positionImage.x;
				int y = (int)nodes[0][j].positionImage.y;

				sz = 15;

				if (j == 9)
				{
					SelectObject(dc2, blue);
					sz += 5;
				}

				if (j == 10)
				{
					SelectObject(dc2, red);
				}

				MoveToEx(dc2, x, y, 0);
				Pie(dc2, x - sz, y - sz, x + sz, y + sz, x + sz, y + sz, x + sz, y + sz);

			}
		}


	}

	DeleteDC(dc2);
	ReleaseDC(hwndPanel, dc);
}//end function


bool GetGestureCheckState(HWND hwndDlg)
{
	return Button_GetState(GetDlgItem(hwndDlg, IDC_GESTURE))&BST_CHECKED;
}


static void GetPlaybackFile(void) {
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"RSSDK clip (*.rssdk)\0*.rssdk\0Old format clip (*.pcsdk)\0*.pcsdk\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = g_file; g_file[0] = 0;
	ofn.nMaxFile = sizeof(g_file) / sizeof(pxcCHAR);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if (!GetOpenFileName(&ofn)) g_file[0] = 0;
}

static void GetRecordFile(void) {
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"RSSDK clip (*.rssdk)\0*.rssdk\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = g_file; g_file[0] = 0;
	ofn.nMaxFile = sizeof(g_file) / sizeof(pxcCHAR);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if (GetSaveFileName(&ofn)) {
		if (ofn.nFilterIndex == 1 && ofn.nFileExtension == 0) {
			int len = wcslen(g_file);
			if (len>1 && len<sizeof(g_file) / sizeof(pxcCHAR) - 7) {
				wcscpy_s(&g_file[len], rsize_t(7), L".rssdk\0");
			}
		}
	}
	else g_file[0] = 0;
}
/*static void GetSaveSnapshot(void) {
OPENFILENAME ofn;
memset(&ofn, 0, sizeof(ofn));
ofn.lStructSize = sizeof(ofn);
ofn.lpstrFilter = L"All\0 * .*\0Text\0 * .TXT\0";

ofn.lpstrFile = g_file; g_file[0] = 0;
ofn.nMaxFile = sizeof(g_file) / sizeof(pxcCHAR);
ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
if (GetSaveFileName(&ofn)) {
if (ofn.nFilterIndex == 1 && ofn.nFileExtension == 0) {
int len = wcslen(g_file);
if (len>1 && len<sizeof(g_file) / sizeof(pxcCHAR) - 7) {
wcscpy_s(&g_file[len], rsize_t(7), L".rssdk\0");
}
}
}
else g_file[0] = 0;

}
*/

BOOL GetSaveSnapshot(HWND hDlg)
{
	TCHAR   szFile[MAX_PATH] = TEXT("\0");
	OPENFILENAME   ofn;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	memset(&(ofn), 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = TEXT("Text (*.txt)\0*.txt\0");
	ofn.lpstrTitle = TEXT("Save File As");
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrDefExt = TEXT("txt");

	//get the filename the user wants to save to
	if (GetSaveFileName(&ofn))
	{
		HWND hEdit = NULL;
		DWORD dwTextLen = 0, bytesWritten = 0;
		TCHAR *wszEditText = NULL;
		char *szEditText = NULL;

		//ofn.lpstrFile contains the full path of the file, get a handle to it
		hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;

		hEdit = GetDlgItem(hDlg, IDC_MOCAPDATA);

		//get the text length of the edit controls contents
		dwTextLen = GetWindowTextLength(hEdit);

		wszEditText = (TCHAR*)malloc((dwTextLen + 1)*sizeof(TCHAR));

		memset(wszEditText, 0, (dwTextLen + 1)*sizeof(TCHAR));

		//read edit controls contents into buffer
		GetWindowText(hEdit, wszEditText, dwTextLen + 1);

		szEditText = (char*)malloc(dwTextLen + 1);

		//convert the wide char read from edit control to char
		wcstombs(szEditText, wszEditText, dwTextLen);

		//save the contents into file
		if (WriteFile(hFile, szEditText, dwTextLen, &bytesWritten, NULL))
		{
		}

		//free resources
		free(wszEditText);
		free(szEditText);
		CloseHandle(hFile);
	}

	return TRUE;
}



INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM) {
	HMENU menu = GetMenu(hwndDlg);
	HMENU menu1;

	switch (message) {
	case WM_INITDIALOG:
		CheckDlgButton(hwndDlg, IDC_DEPTH, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_SCALE, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_MIRROR, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_GEONODE, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_GESTURE, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_PARAMS, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_POSITIONWORLD, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_POSITIONIMAGE, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_GLOBALROTATION, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_LOCALROTATION, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_CONFIDENCE, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SPEED, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FOLDEDNESS, BST_CHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SVALUE), false);

		PopulateDevice(menu);
		PopulateModule(menu);
		SaveLayout(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		menu1 = GetSubMenu(menu, 0);
		if (LOWORD(wParam) >= ID_DEVICEX && LOWORD(wParam)<ID_DEVICEX + GetMenuItemCount(menu1)) {
			CheckMenuRadioItem(menu1, 0, GetMenuItemCount(menu1), LOWORD(wParam) - ID_DEVICEX, MF_BYPOSITION);
			return TRUE;
		}
		menu1 = GetSubMenu(menu, 1);
		if (LOWORD(wParam) >= ID_MODULEX && LOWORD(wParam)<ID_MODULEX + GetMenuItemCount(menu1)) {
			CheckMenuRadioItem(menu1, 0, GetMenuItemCount(menu1), LOWORD(wParam) - ID_MODULEX, MF_BYPOSITION);
			return TRUE;
		}

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			g_stop = true;
			if (g_running) {
				PostMessage(hwndDlg, WM_COMMAND, IDCANCEL, 0);
			}
			else {
				DestroyWindow(hwndDlg);
				PostQuitMessage(0);
			}
			return TRUE;

		case IDC_DEPTH:
			CheckDlgButton(hwndDlg, IDC_DEPTH, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_LABELMAP, BST_UNCHECKED);
			return TRUE;
		case IDC_LABELMAP:
			CheckDlgButton(hwndDlg, IDC_DEPTH, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LABELMAP, BST_CHECKED);
			return TRUE;
		case ID_START:
			Button_Enable(GetDlgItem(hwndDlg, ID_START), false);
			Button_Enable(GetDlgItem(hwndDlg, ID_STOP), true);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SVALUE), false);
			for (int i = 0; i<GetMenuItemCount(menu); i++)
				EnableMenuItem(menu, i, MF_BYPOSITION | MF_GRAYED);

			DrawMenuBar(hwndDlg);
			g_stop = false;
			g_running = true;
#if 1
			m_thread = CreateThread(0, 0, ThreadProc, hwndDlg, 0, 0);
#else
			m_thread = CreateThread(0, 0, ThreadProcAdvanced, hwndDlg, 0, 0);
#endif
			EnableWindow(GetDlgItem(hwndDlg, IDC_INFOBOX), true);
			Sleep(0);
			return TRUE;
		case ID_SNAPSHOT:
			snapshotEnabled = true;
			//MessageBox(NULL, L"SNAPSHOT clicked", NULL, NULL);			
			return TRUE;

		case ID_STOP:
			g_stop = true;
			if (g_running) {
				PostMessage(hwndDlg, WM_COMMAND, ID_STOP, 0);
			}
			else {
				for (int i = 0; i<GetMenuItemCount(menu); i++)
					EnableMenuItem(menu, i, MF_BYPOSITION | MF_ENABLED);
				DrawMenuBar(hwndDlg);
				Button_Enable(GetDlgItem(hwndDlg, ID_START), true);
				Button_Enable(GetDlgItem(hwndDlg, ID_STOP), false);
				if (m_useSmoother)
					EnableWindow(GetDlgItem(hwndDlg, IDC_SVALUE), true);
			}
			return TRUE;
		case ID_MODE_LIVE:
			CheckMenuItem(menu, ID_MODE_LIVE, MF_CHECKED);
			CheckMenuItem(menu, ID_MODE_PLAYBACK, MF_UNCHECKED);
			CheckMenuItem(menu, ID_MODE_RECORD, MF_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDITSPIN), false);
			return TRUE;
		case ID_MODE_PLAYBACK:
			CheckMenuItem(menu, ID_MODE_LIVE, MF_UNCHECKED);
			CheckMenuItem(menu, ID_MODE_PLAYBACK, MF_CHECKED);
			CheckMenuItem(menu, ID_MODE_RECORD, MF_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDITSPIN), false);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SVALUE), false);
			GetPlaybackFile();
			return TRUE;
		case ID_MODE_RECORD:
			CheckMenuItem(menu, ID_MODE_LIVE, MF_UNCHECKED);
			CheckMenuItem(menu, ID_MODE_PLAYBACK, MF_UNCHECKED);
			CheckMenuItem(menu, ID_MODE_RECORD, MF_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDITSPIN), true);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SVALUE), false);
			GetRecordFile();
			return TRUE;
		case IDC_CHECKALL:
			CheckDlgButton(hwndDlg, IDC_POSITIONWORLD, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_POSITIONIMAGE, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_GLOBALROTATION, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_LOCALROTATION, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_CONFIDENCE, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_SPEED, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_FOLDEDNESS, BST_CHECKED);
			return TRUE;
		case IDC_UNCHECKALL:
			CheckDlgButton(hwndDlg, IDC_POSITIONWORLD, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_POSITIONIMAGE, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_GLOBALROTATION, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOCALROTATION, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONFIDENCE, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SPEED, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FOLDEDNESS, BST_CHECKED);
			GetRecordFile();
			return TRUE;
		case ID_SNAPSHOTSAVE:
			GetSaveSnapshot(hwndDlg);
			return TRUE;


		}
		break;
	case WM_TIMER:
		SendMessage(GetDlgItem(hwndDlg, wParam), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_none);
		KillTimer(hwndDlg, wParam);
		return TRUE;
	case WM_SIZE:
		RedoLayout(hwndDlg);
		return TRUE;
	}
	return FALSE;
}

#pragma warning(disable:4706) /* assignment within conditional */
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	LPWSTR *szArgList;
	int argCount;

	szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
	if (szArgList != NULL)
	{
		for (int i = 0; i < argCount; i++)
		{

			if (wcscmp(szArgList[i], L"-nskeleton") == 0)
			{
				showNormalizedSkeleton = true;
			}
			else if (wcscmp(szArgList[i], L"-extremity") == 0)
			{
				showExtremityPoint = true;
			}
			else if (wcscmp(szArgList[i], L"-noRender") == 0)
			{
				noRender = true;
			}
		}

	}

	InitCommonControls();
	g_hInst = hInstance;

	g_session = PXCSession_Create();
	if (g_session == NULL) {
		MessageBoxW(0, L"Failed to create an SDK session", L"Hands Viewer", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	HWND hWnd = CreateDialogW(hInstance, MAKEINTRESOURCE(IDD_MAINFRAME), 0, DialogProc);
	if (!hWnd)  {
		MessageBoxW(0, L"Failed to create a window", L"Hands Viewer", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	HWND hWnd2 = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L"OK", hWnd, IDC_STATUS);
	if (!hWnd2) {
		MessageBoxW(0, L"Failed to create a status bar", L"Hands Viewer", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	HWND hwndTextValue = GetDlgItem(hWnd, IDC_RECORD_FRAME_NUMBER);
	wchar_t lineText2[256] = L"Record Frame Number:";
	SetWindowText(hwndTextValue, lineText2);

	HWND hwndStatus = GetDlgItem(hWnd, IDC_EDIT2);
	SetWindowText(hwndStatus, L"Gesture:");

	UpdateWindow(hWnd);

	MSG msg;
	for (int sts; (sts = GetMessageW(&msg, NULL, 0, 0));) {
		if (sts == -1) return sts;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}

