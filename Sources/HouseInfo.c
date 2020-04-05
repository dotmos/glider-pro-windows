
//============================================================================
//----------------------------------------------------------------------------
//								  HouseInfo.c
//----------------------------------------------------------------------------
//============================================================================


//#include <NumberFormatting.h>
#include "Macintosh.h"
#include "Externs.h"
#include "DialogUtils.h"


#define kHouseInfoDialogID		1001
#define kBannerTextItem			4
#define kLockHouseButton		6
#define kClearScoresButton		9
#define kTrailerTextItem		11
#define kNoPhoneCheck			14
#define kBannerNCharsItem		15
#define kTrailerNCharsItem		16
#define kHouseSizeItem			18
#define kLockHouseAlert			1029
#define kZeroScoresAlert		1032


SInt32 CountTotalHousePoints (void);
void UpdateHouseInfoDialog (DialogPtr);
Boolean HouseFilter (DialogPtr, EventRecord *, SInt16 *);
Boolean WarnLockingHouse (void);
void HowToZeroScores (void);


Str255		banner, trailer;
Rect		houseEditText1, houseEditText2;
SInt16		houseCursorIs;
Boolean		keyHit, tempPhoneBit;

extern	Cursor		beamCursor;
extern	Boolean		noRoomAtAll, changeLockStateOfHouse, saveHouseLocked;
extern	Boolean		phoneBitSet;

#ifndef COMPILEDEMO


//==============================================================  Functions
//--------------------------------------------------------------  CountTotalHousePoints

// The following functions all handle the "House Info" dialog in the editor.

SInt32 CountTotalHousePoints (void)
{
	SInt32		pointTotal;
	SInt16		numRooms, h, i;

	pointTotal = (SInt32)RealRoomNumberCount() * (SInt32)kRoomVisitScore;

	numRooms = thisHouse->nRooms;
	for (i = 0; i < numRooms; i++)
	{
		if (thisHouse->rooms[i].suite != kRoomIsEmpty)
		{
			for (h = 0; h < kMaxRoomObs; h++)
			{
				switch (thisHouse->rooms[i].objects[h].what)
				{
					case kRedClock:
					pointTotal += kRedClockPoints;
					break;

					case kBlueClock:
					pointTotal += kBlueClockPoints;
					break;

					case kYellowClock:
					pointTotal += kYellowClockPoints;
					break;

					case kCuckoo:
					pointTotal += kCuckooClockPoints;
					break;

					case kStar:
					pointTotal += kStarPoints;
					break;

					case kInvisBonus:
					pointTotal += thisHouse->rooms[i].objects[h].data.c.points;
					break;

					default:
					break;
				}
			}
		}
	}

	return (pointTotal);
}

//--------------------------------------------------------------  UpdateHouseInfoDialog

void UpdateHouseInfoDialog (DialogPtr theDialog)
{
	return;
#if 0
	short		nChars;

	DrawDialog(theDialog);
	nChars = GetDialogStringLen(theDialog, kBannerTextItem);
	SetDialogNumToStr(theDialog, kBannerNCharsItem, (long)nChars);
	nChars = GetDialogStringLen(theDialog, kTrailerTextItem);
	SetDialogNumToStr(theDialog, kTrailerNCharsItem, (long)nChars);
	SetDialogNumToStr(theDialog, kHouseSizeItem, CountTotalHousePoints());
	FrameDialogItemC(theDialog, 10, kRedOrangeColor8);
	SetDialogItemValue(theDialog, kNoPhoneCheck, (short)tempPhoneBit);
#endif
}

//--------------------------------------------------------------  HouseFilter

Boolean HouseFilter (DialogPtr dial, EventRecord *event, SInt16 *item)
{
	return false;
#if 0
	Point		mouseIs;
	short		nChars;

	if (keyHit)
	{
		nChars = GetDialogStringLen(dial, kBannerTextItem);
		SetDialogNumToStr(dial, kBannerNCharsItem, (long)nChars);
		nChars = GetDialogStringLen(dial, kTrailerTextItem);
		SetDialogNumToStr(dial, kTrailerNCharsItem, (long)nChars);
		keyHit = false;
	}

	switch (event->what)
	{
		case keyDown:
		switch ((event->message) & charCodeMask)
		{
			case kEnterKeyASCII:
			FlashDialogButton(dial, kOkayButton);
			*item = kOkayButton;
			return(true);
			break;

			case kEscapeKeyASCII:
			FlashDialogButton(dial, kCancelButton);
			*item = kCancelButton;
			return(true);
			break;

			default:
			keyHit = true;
			return(false);
		}
		break;

		case mouseDown:
		return(false);
		break;

		case mouseUp:
		return(false);
		break;

		case updateEvt:
		SetPort((GrafPtr)dial);
		BeginUpdate(GetDialogWindow(dial));
		UpdateHouseInfoDialog(dial);
		EndUpdate(GetDialogWindow(dial));
		event->what = nullEvent;
		return(false);
		break;

		default:
		mouseIs = event->where;
		GlobalToLocal(&mouseIs);
		if ((PtInRect(mouseIs, &houseEditText1)) ||
				(PtInRect(mouseIs, &houseEditText2)))
		{
			if (houseCursorIs != kBeamCursor)
			{
				SetCursor(&beamCursor);
				houseCursorIs = kBeamCursor;
			}
		}
		else
		{
			if (houseCursorIs != kArrowCursor)
			{
				InitCursor();
				houseCursorIs = kArrowCursor;
			}
		}
		return(false);
		break;
	}
#endif
}

//--------------------------------------------------------------  DoHouseInfo

void DoHouseInfo (void)
{
	return;
#if 0
	DialogPtr		houseInfoDialog;
	Str255			versStr, loVers, nRoomsStr;
	long			h, v;
	short			item, numRooms, version;
	char			wasState;
	Boolean			leaving;
	ModalFilterUPP	houseFilterUPP;

	houseFilterUPP = NewModalFilterUPP(HouseFilter);
	tempPhoneBit = phoneBitSet;

	wasState = HGetState((Handle)thisHouse);
	numRooms = RealRoomNumberCount();
	HLock((Handle)thisHouse);
	PasStringCopy((*thisHouse)->banner, banner);
	PasStringCopy((*thisHouse)->trailer, trailer);
	version = (*thisHouse)->version;
	if (!noRoomAtAll)
	{
		h = (long)(*thisHouse)->rooms[(*thisHouse)->firstRoom].suite;
		v = (long)(*thisHouse)->rooms[(*thisHouse)->firstRoom].floor;
	}
	HSetState((Handle)thisHouse, wasState);

	NumToString((long)version >> 8, versStr);		// Convert version to two strings…
	NumToString((long)version % 0x0100, loVers);	// the 1's and 1/10th's part.
	NumToString((long)numRooms, nRoomsStr);			// Number of rooms -> string.

	ParamText(versStr, loVers, nRoomsStr, "\p");

//	CenterDialog(kHouseInfoDialogID);
	houseInfoDialog = GetNewDialog(kHouseInfoDialogID, nil, kPutInFront);
	if (houseInfoDialog == nil)
		RedAlert(kErrDialogDidntLoad);
	SetPort((GrafPtr)houseInfoDialog);
	ShowWindow(GetDialogWindow(houseInfoDialog));

	SetDialogString(houseInfoDialog, kBannerTextItem, banner);
	SetDialogString(houseInfoDialog, kTrailerTextItem, trailer);
	SelectDialogItemText(houseInfoDialog, kBannerTextItem, 0, 1024);
	GetDialogItemRect(houseInfoDialog, kBannerTextItem, &houseEditText1);
	GetDialogItemRect(houseInfoDialog, kTrailerTextItem, &houseEditText2);
	houseCursorIs = kArrowCursor;
	leaving = false;

	while (!leaving)
	{
		ModalDialog(houseFilterUPP, &item);

		if (item == kOkayButton)
		{
			GetDialogString(houseInfoDialog, kBannerTextItem, banner);
			GetDialogString(houseInfoDialog, kTrailerTextItem, trailer);

			wasState = HGetState((Handle)thisHouse);
			HLock((Handle)thisHouse);
			PasStringCopyNum(banner, (*thisHouse)->banner, 255);
			PasStringCopyNum(trailer, (*thisHouse)->trailer, 255);
			if (tempPhoneBit != phoneBitSet)
			{
				phoneBitSet = tempPhoneBit;
				if (phoneBitSet)
					(*thisHouse)->flags = (*thisHouse)->flags | 0x00000002;
				else
					(*thisHouse)->flags = (*thisHouse)->flags & 0xFFFFDFFD;
			}
			HSetState((Handle)thisHouse, wasState);

			fileDirty = true;
			UpdateMenus(false);
			leaving = true;
		}
		else if (item == kCancelButton)
			leaving = true;
		else if (item == kLockHouseButton)
		{
			if (WarnLockingHouse())
			{
				changeLockStateOfHouse = true;
				saveHouseLocked = true;
				fileDirty = true;
				UpdateMenus(false);
			}
		}
		else if (item == kClearScoresButton)
			HowToZeroScores();
		else if (item == kNoPhoneCheck)
		{
			tempPhoneBit = !tempPhoneBit;
			SetDialogItemValue(houseInfoDialog, kNoPhoneCheck, (short)tempPhoneBit);
		}
	}
	InitCursor();
	DisposeDialog(houseInfoDialog);
	DisposeModalFilterUPP(houseFilterUPP);
#endif
}

//--------------------------------------------------------------  WarnLockingHouse

Boolean WarnLockingHouse (void)
{
	return true;
#if 0
	short		hitWhat;

//	CenterAlert(kLockHouseAlert);
	hitWhat = Alert(kLockHouseAlert, nil);

	return (hitWhat == 1);
#endif
}

//--------------------------------------------------------------  HowToZeroScores

void HowToZeroScores (void)
{
	return;
#if 0
	short		hitWhat;

//	CenterAlert(kZeroScoresAlert);
	hitWhat = Alert(kZeroScoresAlert, nil);

	switch (hitWhat)
	{
		case 2:		// zero all
		ZeroHighScores();
		fileDirty = true;
		UpdateMenus(false);
		break;

		case 3:		// zero all but highest
		ZeroAllButHighestScore();
		fileDirty = true;
		UpdateMenus(false);
		break;
	}
#endif
}

#endif

