#include <string>
#include <iostream>

#include <windows.h>
#include "pxcsensemanager.h"
#include "pxchandconfiguration.h"

static const std::string FingerTypeStrings[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };

void displayFingerData(PXCHandData::FingerType finger, PXCHandData::IHand* handData)
{
	PXCHandData::FingerData fingerData;

	handData->QueryFingerData(finger, fingerData);

	std::cout << FingerTypeStrings[finger] << " has foldness " << fingerData.foldedness << " and radius " << fingerData.radius << std::endl;
}

void processFingerInfo(PXCHandData::JointType jointType, PXCHandData::IHand* handData)
{
	switch (jointType)
	{
	case PXCHandData::JointType::JOINT_INDEX_BASE:
		displayFingerData(PXCHandData::FingerType::FINGER_INDEX, handData);
		break;
	case PXCHandData::JointType::JOINT_MIDDLE_BASE:
		displayFingerData(PXCHandData::FingerType::FINGER_MIDDLE, handData);
		break;
	case PXCHandData::JointType::JOINT_PINKY_BASE:
		displayFingerData(PXCHandData::FingerType::FINGER_PINKY, handData);
		break;
	case PXCHandData::JointType::JOINT_RING_BASE:
		displayFingerData(PXCHandData::FingerType::FINGER_RING, handData);
		break;
	case PXCHandData::JointType::JOINT_THUMB_BASE:
		displayFingerData(PXCHandData::FingerType::FINGER_THUMB, handData);
		break;
	default:
		break;
	}
}