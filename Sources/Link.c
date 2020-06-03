
//============================================================================
//----------------------------------------------------------------------------
//									  Link.c
//----------------------------------------------------------------------------
//============================================================================


#include "Environ.h"
#include "Externs.h"
#include "ObjectEdit.h"
#include "RectUtils.h"


#define kLinkControlID			130
#define kUnlinkControlID		131


void DoLink (void);
void DoUnlink (void);


Rect			linkWindowRect;
ControlHandle	linkControl, unlinkControl;
HWND			linkWindow;
SInt16			isLinkH, isLinkV, linkRoom, linkType;
Byte			linkObject;
Boolean			isLinkOpen, linkerIsSwitch;


//==============================================================  Functions
//--------------------------------------------------------------  MergeFloorSuite

SInt16 MergeFloorSuite (SInt16 floor, SInt16 suite)
{
	return ((suite * 100) + floor);
}

//--------------------------------------------------------------  ExtractFloorSuite

void ExtractFloorSuite (SInt16 combo, SInt16 *floor, SInt16 *suite)
{
	if (thisHouse->version < 0x0200)		// old floor/suite combo
	{
		*floor = (combo / 100) - kNumUndergroundFloors;
		*suite = combo % 100;
	}
	else
	{
		*suite = combo / 100;
		*floor = (combo % 100) - kNumUndergroundFloors;
	}
}

//--------------------------------------------------------------  UpdateLinkControl

void UpdateLinkControl (void)
{
	return;
#if 0
#ifndef COMPILEDEMO
	if (linkWindow == nil)
		return;

	switch (linkType)
	{
		case kSwitchLinkOnly:
		if (objActive == kNoObjectSelected)
			HiliteControl(linkControl, kControlInactive);
		else
			switch (thisRoom->objects[objActive].what)
			{
				case kFloorVent:
				case kCeilingVent:
				case kFloorBlower:
				case kCeilingBlower:
				case kSewerGrate:
				case kLeftFan:
				case kRightFan:
				case kInvisBlower:
				case kGrecoVent:
				case kSewerBlower:
				case kLiftArea:
				case kRedClock:
				case kBlueClock:
				case kYellowClock:
				case kCuckoo:
				case kPaper:
				case kBattery:
				case kBands:
				case kFoil:
				case kInvisBonus:
				case kHelium:
				case kDeluxeTrans:
				case kCeilingLight:
				case kLightBulb:
				case kTableLamp:
				case kHipLamp:
				case kDecoLamp:
				case kFlourescent:
				case kTrackLight:
				case kInvisLight:
				case kShredder:
				case kToaster:
				case kMacPlus:
				case kTV:
				case kCoffee:
				case kOutlet:
				case kVCR:
				case kStereo:
				case kMicrowave:
				case kBalloon:
				case kCopterLf:
				case kCopterRt:
				case kDartLf:
				case kDartRt:
				case kBall:
				case kDrip:
				case kFish:
				HiliteControl(linkControl, kControlActive);
				break;

				default:
				HiliteControl(linkControl, kControlInactive);
				break;
			}
		break;

		case kTriggerLinkOnly:
		if (objActive == kNoObjectSelected)
			HiliteControl(linkControl, kControlInactive);
		else
			switch (thisRoom->objects[objActive].what)
			{
				case kGreaseRt:
				case kGreaseLf:
				case kToaster:
				case kGuitar:
				case kCoffee:
				case kOutlet:
				case kBalloon:
				case kCopterLf:
				case kCopterRt:
				case kDartLf:
				case kDartRt:
				case kDrip:
				case kFish:
				HiliteControl(linkControl, kControlActive);
				break;

				case kLightSwitch:
				case kMachineSwitch:
				case kThermostat:
				case kPowerSwitch:
				case kKnifeSwitch:
				case kInvisSwitch:
				if (linkRoom == thisRoomNumber)
					HiliteControl(linkControl, kControlActive);
				break;

				default:
				HiliteControl(linkControl, kControlInactive);
				break;
			}
		break;

		case kTransportLinkOnly:
		if (objActive == kNoObjectSelected)
			HiliteControl(linkControl, kControlInactive);
		else
			switch (thisRoom->objects[objActive].what)
			{
				case kMailboxLf:
				case kMailboxRt:
				case kCeilingTrans:
				case kInvisTrans:
				case kDeluxeTrans:
				case kInvisLight:
				case kOzma:
				case kMirror:
				case kFireplace:
				case kWallWindow:
				case kCalendar:
				case kBulletin:
				case kCloud:
				HiliteControl(linkControl, kControlActive);
				break;

				default:
				HiliteControl(linkControl, kControlInactive);
				break;
			}
		break;
	}
#endif
#endif
}

//--------------------------------------------------------------  UpdateLinkWindow

void UpdateLinkWindow (void)
{
	return;
#if 0
#ifndef COMPILEDEMO
	if (linkWindow == nil)
		return;

	SetPortWindowPort(linkWindow);
	DrawControls(linkWindow);
	UpdateLinkControl();
#endif
#endif
}

//--------------------------------------------------------------  OpenLinkWindow

void OpenLinkWindow (void)
{
	return;
#if 0
#ifndef COMPILEDEMO
	Rect		src, dest;
	Point		globalMouse;

	if (linkWindow == nil)
	{
		QSetRect(&linkWindowRect, 0, 0, 129, 30);
		if (thisMac.hasColor)
			linkWindow = NewCWindow(nil, &linkWindowRect,
					"\pLink", false, kWindoidWDEF, kPutInFront, true, 0L);
		else
			linkWindow = NewWindow(nil, &linkWindowRect,
					"\pLink", false, kWindoidWDEF, kPutInFront, true, 0L);

		MoveWindow(linkWindow, isLinkH, isLinkV, true);
		globalMouse = MyGetGlobalMouse();
		QSetRect(&src, 0, 0, 1, 1);
		QOffsetRect(&src, globalMouse.h, globalMouse.v);
		GetWindowRect(linkWindow, &dest);
		BringToFront(linkWindow);
		ShowHide(linkWindow, true);
//		FlagWindowFloating(linkWindow);	TEMP - use flaoting windows
		HiliteAllWindows();

		linkControl = GetNewControl(kLinkControlID, linkWindow);
		if (linkControl == nil)
			RedAlert(kErrFailedResourceLoad);

		unlinkControl = GetNewControl(kUnlinkControlID, linkWindow);
		if (unlinkControl == nil)
			RedAlert(kErrFailedResourceLoad);

		linkRoom = -1;
		linkObject = 255;

		isLinkOpen = true;
	}
#endif
#endif
}

//--------------------------------------------------------------  CloseLinkWindow

void CloseLinkWindow (void)
{
	return;
#if 0
#ifndef COMPILEDEMO
	if (linkWindow != nil)
		DisposeWindow(linkWindow);

	linkWindow = nil;
	isLinkOpen = false;
#endif
#endif
}

//--------------------------------------------------------------  DoLink

#ifndef COMPILEDEMO
void DoLink (void)
{
	SInt16		floor, suite;

	if (GetRoomFloorSuite(thisRoomNumber, &floor, &suite))
	{
		floor += kNumUndergroundFloors;
		if (thisRoomNumber == linkRoom)
		{
			if (linkerIsSwitch)
			{
				thisRoom->objects[linkObject].data.e.where =
						MergeFloorSuite(floor, suite);
				thisRoom->objects[linkObject].data.e.who =
						(Byte)objActive;
			}
			else
			{
				thisRoom->objects[linkObject].data.d.where =
						MergeFloorSuite(floor, suite);
				thisRoom->objects[linkObject].data.d.who =
						(Byte)objActive;
			}
		}
		else
		{
			if (linkerIsSwitch)
			{
				thisHouse->rooms[linkRoom].objects[linkObject].data.e.where =
						MergeFloorSuite(floor, suite);
				thisHouse->rooms[linkRoom].objects[linkObject].data.e.who =
						(Byte)objActive;
			}
			else	// linker is transport
			{
				thisHouse->rooms[linkRoom].objects[linkObject].data.d.where =
						MergeFloorSuite(floor, suite);
				thisHouse->rooms[linkRoom].objects[linkObject].data.d.who =
						(Byte)objActive;
			}
		}
		fileDirty = true;
		UpdateMenus(false);
		CloseLinkWindow();
	}
}
#endif

//--------------------------------------------------------------  DoUnlink

#ifndef COMPILEDEMO
void DoUnlink (void)
{
	if (thisRoomNumber == linkRoom)
	{
		if (linkerIsSwitch)
		{
			thisRoom->objects[linkObject].data.e.where = -1;
			thisRoom->objects[linkObject].data.e.who = 255;
		}
		else
		{
			thisRoom->objects[linkObject].data.d.where = -1;
			thisRoom->objects[linkObject].data.d.who = 255;
		}
	}
	else
	{
		if (linkerIsSwitch)
		{
			thisHouse->rooms[linkRoom].objects[linkObject].data.e.where = -1;
			thisHouse->rooms[linkRoom].objects[linkObject].data.e.who = 255;
		}
		else
		{
			thisHouse->rooms[linkRoom].objects[linkObject].data.d.where = -1;
			thisHouse->rooms[linkRoom].objects[linkObject].data.d.who = 255;
		}
	}
	fileDirty = true;
	UpdateMenus(false);
	CloseLinkWindow();
}
#endif

//--------------------------------------------------------------  HandleLinkClick

void HandleLinkClick (Point wherePt)
{
	return;
#if 0
#ifndef COMPILEDEMO
	ControlHandle	theControl;
	short			part;

	if (linkWindow == nil)
		return;

	SetPortWindowPort(linkWindow);
	GlobalToLocal(&wherePt);

	part = FindControl(wherePt, linkWindow, &theControl);
	if ((theControl != nil) && (part != 0))
	{
		part = TrackControl(theControl, wherePt, nil);
		if (part != 0)
		{
			if (theControl == linkControl)
				DoLink();
			else if (theControl == unlinkControl)
				DoUnlink();

			if (thisRoomNumber == linkRoom)
				CopyThisRoomToRoom();
			GenerateRetroLinks();
		}
	}
#endif
#endif
}

