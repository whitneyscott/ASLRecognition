/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#include <Windows.h>
#include <WindowsX.h>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <iostream>
#include "pxchandmodule.h"
#include "pxcsensemanager.h"
#include "timer.h"
#include "pxccapture.h"
#include "pxcvideomodule.h"
#include "pxchandconfiguration.h"
#include "pxchanddata.h"
#include "resource1.h"

extern volatile bool g_stop;
extern bool showNormalizedSkeleton;
extern bool showExtremityPoint;
extern bool noRender;
extern bool snapshotEnabled;

int gestureIndex = 0;

volatile bool g_connected = false;
extern pxcCHAR g_file[1024];
extern PXCSession *g_session;

//gesture combobox 
int GetSelectedGesture(HWND hwndDlg);
void EnableCMBItem(HWND hwndDlg, pxcBool enable);
void SetCMBGesturePos(HWND hwndDlg);
bool IsCMBGestureInit();
void AddCMBItem(HWND hwndDlg, pxcCHAR *line);
void SetGestureLeftStatus(HWND hwndDlg, pxcCHAR *line);
void SetGestureRightStatus(HWND hwndDlg, pxcCHAR *line);
void SetStatus(HWND hwndDlg, pxcCHAR *line);
pxcCHAR* GetCheckedDevice(HWND);
pxcCHAR* GetCheckedModule(HWND);
//----- Viewer Options
bool GetLabelmapState(HWND);
bool GetDepthState(HWND);
bool GetAlertState(HWND);
//---- Gesture Parameters
bool GetPositionWorldState(HWND);
bool GetPositionImageState(HWND);
bool GetGlobalRotationState(HWND);
bool GetLocalRotationState(HWND);
bool GetSpeedState(HWND);
bool GetConfidenceState(HWND);
bool GetFoldednessState(HWND);

void DrawBitmap(HWND, PXCImage*);
void DrawGesture(HWND hwndDlg, PXCHandData::GestureData gestureData, int bodySide);
void SetHandsMask(PXCImage*, pxcI32);
void ClearBuffer(PXCImage::ImageInfo);
void DrawJoints(HWND, PXCHandData::JointData[2][PXCHandData::NUMBER_OF_JOINTS], PXCHandData::ExtremityData[2][PXCHandData::NUMBER_OF_EXTREMITIES]);
void UpdatePanel(HWND);
bool GetPlaybackState(HWND hwndDlg);
bool GetRecordState(HWND hwndDlg);
int GetFramesToRecord(HWND hwndDlg);
void setFramesRecordBox(HWND hwndDlg, int frameNumber);
void StoreMocapData(char sAlerts);
void setMaxRangeValue(float value);
void SetMocapData(HWND hwndDlg, pxcCHAR *line);
void SetInfoBox(HWND hwndDlg, pxcCHAR *line);

void processFingerInfo(PXCHandData::JointType jointType, PXCHandData::IHand* handData);

static const std::string FingerTypeStrings[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };

//vector containing depth image - for synchronization purposes
std::vector<PXCImage*> m_depthImages;

const int NUMBER_OF_FRAMES_TO_DELAY = 3;



using namespace std;
template <typename T>
string NumberToString(T Number)
{
	ostringstream ss;
	ss << Number;
	return ss.str();
}

static void releaseVectorImages()
{
	for (std::vector<PXCImage *>::iterator it = m_depthImages.begin(); it != m_depthImages.end(); ++it)
	{
		(*it)->Release();
	}

	while (!m_depthImages.empty())
	{
		m_depthImages.pop_back();
	}
}

/* Checking if sensor device connect or not */
static bool DisplayDeviceConnection(HWND hwndDlg, bool state) {
	if (state) {
		if (!g_connected) SetStatus(hwndDlg, L"Device Reconnected");
		g_connected = true;
	}
	else {
		if (g_connected) SetStatus(hwndDlg, L"Device Disconnected");
		g_connected = false;
	}
	return g_connected;
}

/* Displaying Depth/Mask Images - for depth image only we use a delay of NUMBER_OF_FRAMES_TO_DELAY to sync image with tracking */
static void DisplayPicture(HWND hwndDlg, PXCImage *depth, PXCHandData *handData) {
	if (!depth) return;
	PXCImage* image = depth;
	PXCImage::ImageInfo info = image->QueryInfo();

	ClearBuffer(info);
	//Mask Image
	if (GetLabelmapState(hwndDlg))
	{
		pxcUID handID;
		pxcI32 numOfHands = handData->QueryNumberOfHands();
		info.format = PXCImage::PIXEL_FORMAT_Y8;
		info.width = 160;
		info.height = 120;
		PXCImage::ImageData bdata;
		if (numOfHands>0)
		{
			for (int i = 0; i<numOfHands; i++)
			{
				handData->QueryHandId(PXCHandData::AccessOrderType::ACCESS_ORDER_BY_ID, i, handID);
				PXCHandData::IHand* hand;

				if (handData->QueryHandDataById(handID, hand) == PXC_STATUS_NO_ERROR)
				{
					hand->QuerySegmentationImage(image);
					SetHandsMask(image, i + 1);
				}
				else
				{
					image = g_session->CreateImage(&info);
					image->AcquireAccess(PXCImage::ACCESS_WRITE, &bdata);
					memset(bdata.planes[0], 0, bdata.pitches[0] * info.height);
				}
			}
		}
		else
		{
			image = g_session->CreateImage(&info);
			image->AcquireAccess(PXCImage::ACCESS_WRITE, &bdata);
			memset(bdata.planes[0], 0, bdata.pitches[0] * info.height);
		}

		DrawBitmap(hwndDlg, image);
	}

	//Depth image
	else
	{
		//collecting 3 images inside a vector and displaying the oldest image
		PXCImage::ImageInfo iinfo;
		memset(&iinfo, 0, sizeof(iinfo));
		iinfo = image->QueryInfo();
		PXCImage *clonedImage;
		clonedImage = g_session->CreateImage(&iinfo);
		clonedImage->CopyImage(image);
		m_depthImages.push_back(clonedImage);
		if (m_depthImages.size() == NUMBER_OF_FRAMES_TO_DELAY)
		{
			DrawBitmap(hwndDlg, m_depthImages.front());
			m_depthImages.front()->Release();
			m_depthImages.erase(m_depthImages.begin());
		}
	}
}

static std::string ConvertWStringToString(const std::wstring& str)
{
	size_t numConverted, finalCount;

	// what size of buffer (in bytes) do we need to allocate for conversion?
	wcstombs_s(&numConverted, NULL, 0, str.c_str(), 1024);
	numConverted += 2; // for null termination
	char *pBuffer = new char[numConverted];

	// do the actual conversion
	wcstombs_s(&finalCount, pBuffer, numConverted, str.c_str(), 1024);
	std::string strValue = std::string(pBuffer);
	delete[] pBuffer;
	return strValue;
}

void string2wchar_t(wchar_t* wchar, const std::string &str)
{
	int index = 0;
	while (index < (int)str.size())
	{
		wchar[index] = (wchar_t)str[index];
		++index;
	}
	wchar[index] = 0;
}



/* Displaying current frame gestures */
static void DisplayGesture(HWND hwndDlg, PXCHandData *handAnalysis, int frameNumber) {

	static pxcCHAR arr[1000];

	int numOfGesture = handAnalysis->QueryFiredGesturesNumber();
	if (numOfGesture>0)
	{
		std::ostringstream s;
		s << "Frame " << frameNumber << ") ";
		std::string gestureStatus(s.str());

		//Iterate fired gestures
		for (int i = 0; i < numOfGesture; i++)
		{
			//Get fired gesture data
			PXCHandData::GestureData gestureData;
			if (handAnalysis->QueryFiredGestureData(i, gestureData) == PXC_STATUS_NO_ERROR)
			{
				//Get hand data related to fired gesture
				PXCHandData::IHand* handData;
				if (handAnalysis->QueryHandDataById(gestureData.handId, handData) == PXC_STATUS_NO_ERROR)
				{
					std::wstring str(gestureData.name);
					if (handData->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
					{
						gestureStatus += ",Left Hand Gesture: ";
						gestureStatus += ConvertWStringToString(str);
						gestureStatus += "\n";
						string2wchar_t(arr, gestureStatus);
					}
					else if (handData->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT)
					{
						gestureStatus += "Right Hand Gesture: ";
						gestureStatus += ConvertWStringToString(str);
						gestureStatus += "\n";
						string2wchar_t(arr, gestureStatus);
					}
				}
			}
		}
		SetInfoBox(hwndDlg, arr);
	}
}

/* Displaying current frames hand joints */
static void DisplayJoints(HWND hwndDlg, PXCHandData *handAnalyzer, int frameNumber) {

	PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS] = {};
	PXCHandData::ExtremityData extremitiesPointsNodes[2][PXCHandData::NUMBER_OF_EXTREMITIES] = {};
	// Initialize mocap string
	static pxcCHAR arr[50000];
	static pxcCHAR hdr[1000];
	// Populate the array of headings
	std::ostringstream st;
	st << "Joint,";
	std::string sHdr(st.str());

	if (GetLocalRotationState(hwndDlg)){
		sHdr += "XrotLocal,YrotLocal,ZrotLocal";
	}
	sHdr += "\r\n";


	std::ostringstream s;
	//s << "Frame: " << frameNumber << "\r\n";
	s << "Gesture Data: " << "\r\n";
	std::string sAlerts(s.str());
	bool fingerEnabled = false;
	sAlerts += sHdr;
	//Iterate hands
	for (pxcI32 i = 0; i < handAnalyzer->QueryNumberOfHands(); i++)
	{
		//Get hand by time of appearence
		PXCHandData::IHand* handData;
		PXCHandData::FingerData fingerData;
		if (handAnalyzer->QueryHandData(PXCHandData::AccessOrderType::ACCESS_ORDER_BY_TIME, i, handData) == PXC_STATUS_NO_ERROR)
		{
			PXCHandData::JointData jointData;
			//Iterate Joints
			for (int j = 0; j < PXCHandData::NUMBER_OF_JOINTS; j++)
			{
				if (showNormalizedSkeleton == false)
				{
					handData->QueryTrackedJoint((PXCHandData::JointType)j, jointData);
				}
				else
				{
					handData->QueryNormalizedJoint((PXCHandData::JointType)j, jointData);
				}
				PXCHandData::JointType jointType;
				int finger;

				nodes[i][j] = jointData;
				switch (j)
				{
				case 6:
					finger = 1;
					handData->QueryFingerData(PXCHandData::FingerType::FINGER_INDEX, fingerData);
					fingerEnabled = true;
					break;
				case 10:
					finger = 2;
					handData->QueryFingerData(PXCHandData::FingerType::FINGER_MIDDLE, fingerData);
					fingerEnabled = true;
					break;
				case 18:
					handData->QueryFingerData(PXCHandData::FingerType::FINGER_PINKY, fingerData);
					finger = 4;
					fingerEnabled = true;
					break;
				case 14:
					handData->QueryFingerData(PXCHandData::FingerType::FINGER_RING, fingerData);
					finger = 3;
					fingerEnabled = true;
					break;
				case 2:
					handData->QueryFingerData(PXCHandData::FingerType::FINGER_THUMB, fingerData);
					finger = 0;
					fingerEnabled = true;
					break;
				default:
					fingerEnabled = false;
					break;
				}

				// create the mocap snapshot string
				if (snapshotEnabled){
					sAlerts += NumberToString(j);
					sAlerts += ",";
					sAlerts += NumberToString(jointData.localRotation.x);
					sAlerts += ",";
					sAlerts += NumberToString(jointData.localRotation.y);
					sAlerts += ",";
					sAlerts += NumberToString(jointData.localRotation.z);
					if (fingerEnabled){

						sAlerts += " ";
						sAlerts += FingerTypeStrings[finger];
						if (FingerTypeStrings[finger] != "Thumb"){
							sAlerts += " finger has foldedness of ";
						}
						else
						{
							sAlerts += " has foldedness of ";
						}

						sAlerts += NumberToString(fingerData.foldedness);
						fingerEnabled = false;
					}
					sAlerts += "\r\n";
					string2wchar_t(arr, sAlerts);

				}
			}
			if (showExtremityPoint == true){
				for (int j = 0; j < PXCHandData::NUMBER_OF_EXTREMITIES; j++)
				{
					handData->QueryExtremityPoint((PXCHandData::ExtremityType)j, extremitiesPointsNodes[i][j]);

				}
			}
		}
		//code below works okay for single frame, but bogs down during runtime
		if (snapshotEnabled == true){
			SetMocapData(hwndDlg, arr);
			snapshotEnabled = false;
		}


		DrawJoints(hwndDlg, nodes, extremitiesPointsNodes);
	}
}


/* Display current frames alerts */
void DisplayAlerts(HWND hwndDlg, PXCHandData *handAnalyzer, int frameNumber) {

	static pxcCHAR arr[1000];
	//Itrate Alerts 
	PXCHandData::AlertData alertData;
	bool isFired = false;

	std::ostringstream s;
	s << "Frame " << frameNumber << ")  Alert: ";
	std::string sAlerts(s.str());

	for (int i = 0; i <handAnalyzer->QueryFiredAlertsNumber(); ++i)
	{
		pxcStatus sts = handAnalyzer->QueryFiredAlertData(i, alertData);
		if (sts == PXC_STATUS_NO_ERROR)
		{
			//Displaying last alert - see AlertData::AlertType for all available alerts
			switch (alertData.label)
			{
			case PXCHandData::ALERT_HAND_DETECTED:
			{

				sAlerts += "Hand Detected, ";
				isFired = true;
				break;
			}
			case PXCHandData::ALERT_HAND_NOT_DETECTED:
			{

				sAlerts += "Hand Not Detected, ";
				isFired = true;
				break;
			}
			case PXCHandData::ALERT_HAND_CALIBRATED:
			{

				sAlerts += "Hand Calibrated, ";
				isFired = true;
				break;
			}
			case PXCHandData::ALERT_HAND_NOT_CALIBRATED:
			{

				sAlerts += "Hand Not Calibrated, ";
				isFired = true;
				break;
			}
			case PXCHandData::ALERT_HAND_INSIDE_BORDERS:
			{

				sAlerts += "Hand Inside Borders, ";
				isFired = true;
				break;
			}
			case PXCHandData::ALERT_HAND_OUT_OF_BORDERS:
			{
				sAlerts += "Hand Out Of Borders, ";
				isFired = true;
				break;
			}
			}
		}
	}
	if (isFired == true)
	{
		sAlerts += "\n";
		string2wchar_t(arr, sAlerts);
		//SetInfoBox(hwndDlg,arr);
	}
}

/* Using PXCSenseManager to handle data */
void SimplePipeline(HWND hwndDlg) {
	SetMocapData(hwndDlg, NULL);
	SetInfoBox(hwndDlg, NULL);
	bool liveCamera = false;

	PXCSenseManager *pp = g_session->CreateSenseManager();
	if (!pp)
	{
		SetStatus(hwndDlg, L"Failed to create SenseManager");
		return;
	}

	/* Set Mode & Source */
	if (GetRecordState(hwndDlg)) {
		pp->QueryCaptureManager()->SetFileName(g_file, true);
		pp->QueryCaptureManager()->FilterByDeviceInfo(GetCheckedDevice(hwndDlg), 0, 0);
	}
	else if (GetPlaybackState(hwndDlg)) {
		pp->QueryCaptureManager()->SetFileName(g_file, false);
		pp->QueryCaptureManager()->SetRealtime(false);
	}
	else {
		pp->QueryCaptureManager()->FilterByDeviceInfo(GetCheckedDevice(hwndDlg), 0, 0);
		liveCamera = true;
	}

	bool sts = true;
	/* Set Module */
	pxcStatus status = pp->EnableHand(0);
	PXCHandModule *handAnalyzer = pp->QueryHand();
	if (handAnalyzer == NULL || status != pxcStatus::PXC_STATUS_NO_ERROR)
	{
		SetStatus(hwndDlg, L"Failed to pair the gesture module with I/O");
		return;
	}

	/* Init */
	FPSTimer timer;
	SetStatus(hwndDlg, L"Init Started");
	if (pp->Init() >= PXC_STATUS_NO_ERROR)
	{
		PXCHandData* outputData = handAnalyzer->CreateOutput();

		// IF IVCAM Set the following properties
		PXCCapture::Device *device = pp->QueryCaptureManager()->QueryDevice();
		PXCCapture::DeviceInfo dinfo;
		pp->QueryCaptureManager()->QueryDevice()->QueryDeviceInfo(&dinfo);
		if (dinfo.model == PXCCapture::DEVICE_MODEL_IVCAM)
		{
			device->SetDepthConfidenceThreshold(1);
			device->SetMirrorMode(PXCCapture::Device::MIRROR_MODE_DISABLED);
			device->SetIVCAMFilterOption(6);
		}

		setMaxRangeValue(pp->QueryCaptureManager()->QueryDevice()->QueryDepthSensorRange().max);

		// Hand Module Configuration
		PXCHandConfiguration* config = handAnalyzer->CreateActiveConfiguration();
		config->EnableNormalizedJoints(showNormalizedSkeleton);
		if (showExtremityPoint) config->SetTrackingMode(PXCHandData::TRACKING_MODE_EXTREMITIES);
		config->EnableAllAlerts();
		config->EnableSegmentationImage(true);
		config->EnableAllGestures();
		config->ApplyChanges();
		config->Update();

		if (IsCMBGestureInit() == false)
		{
			pxcI32 totalNumOfGestures = config->QueryGesturesTotalNumber();
			if (totalNumOfGestures > 0)
			{
				SetCMBGesturePos(hwndDlg);
				for (int i = 0; i < totalNumOfGestures; i++)
				{
					pxcCHAR* gestureName = new pxcCHAR[PXCHandData::MAX_NAME_SIZE];
					if (config->QueryGestureNameByIndex(i, PXCHandData::MAX_NAME_SIZE, gestureName) == pxcStatus::PXC_STATUS_NO_ERROR)
					{
						AddCMBItem(hwndDlg, gestureName);
					}
					delete[] gestureName;
					gestureName = NULL;
				}
				EnableCMBItem(hwndDlg, true);
			}
		}

		SetStatus(hwndDlg, L"Streaming");
		g_connected = true;
		int frameCounter = 0;
		int frameNumber = 0;

		while (!g_stop)
		{
			int index = GetSelectedGesture(hwndDlg);
			if (index>0 && gestureIndex != index)
			{
				gestureIndex = index;
				pxcCHAR* gestureName = new pxcCHAR[PXCHandData::MAX_NAME_SIZE];
				if (config->QueryGestureNameByIndex(index - 1, PXCHandData::MAX_NAME_SIZE, gestureName) == pxcStatus::PXC_STATUS_NO_ERROR)
				{
					config->DisableAllGestures();
					config->EnableGesture(gestureName, true);
					config->ApplyChanges();
				}
				delete[] gestureName;
				gestureName = NULL;
			}


			pxcStatus sts = pp->AcquireFrame(true);
			if (DisplayDeviceConnection(hwndDlg, pp->IsConnected()))
			{
				if (sts < PXC_STATUS_NO_ERROR)
					break;
				frameCounter++;
				outputData->Update();
				const PXCCapture::Sample *sample = pp->QueryHandSample();
				if (sample && sample->depth && !noRender)
				{
					frameNumber = liveCamera ? frameCounter : pp->QueryCaptureManager()->QueryFrameIndex();
					DisplayPicture(hwndDlg, sample->depth, outputData);
					DisplayJoints(hwndDlg, outputData, frameNumber);
					DisplayGesture(hwndDlg, outputData, frameNumber);
					DisplayAlerts(hwndDlg, outputData, frameNumber);
					UpdatePanel(hwndDlg);
					timer.Tick(hwndDlg);
				}
			}
			pp->ReleaseFrame();
		}

		config->Release();
		outputData->Release();

	}
	else
	{
		SetStatus(hwndDlg, L"Init Failed");
		sts = false;
	}


	releaseVectorImages();

	pp->Close();
	pp->Release();
	if (sts) SetStatus(hwndDlg, L"Stopped");
}