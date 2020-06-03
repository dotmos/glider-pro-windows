//============================================================================
//----------------------------------------------------------------------------
//									GameOver.c
//----------------------------------------------------------------------------
//============================================================================


#define _CRT_SECURE_NO_WARNINGS
#include "Environ.h"
#include "Externs.h"
#include "Macintosh.h"
#include "MainWindow.h"
#include "Objects.h"
#include "RectUtils.h"
#include "ResourceIDs.h"
#include "Utilities.h"


#define kNumCountDownFrames		16
#define kPageFrames				14


typedef struct
{
	Rect		dest, was;
	SInt16		frame, counter;
	Boolean		stuck;
} pageType, *pagePtr;


void DoGameOverStarAnimation (void);
void SetUpFinalScreen (void);
void InitDiedGameOver (void);
void HandlePages (void);
void DrawPages (void);


pageType	pages[8];
Rect		pageSrcRect, pageSrc[kPageFrames], lettersSrc[8], angelSrcRect;
HRGN		roomRgn;
HDC			pageSrcMap, gameOverSrcMap, angelSrcMap;
HDC			pageMaskMap, angelMaskMap;
SInt16		countDown, stopPages, pagesStuck;
Boolean		gameOver;

extern	Rect		justRoomsRect;
extern	SInt16		splashOriginH, splashOriginV, numWork2Main;
extern	SInt16		numBack2Work;
extern	Boolean		playing, shadowVisible, demoGoing;


//==============================================================  Functions
//--------------------------------------------------------------  DoGameOver

// Handles a game over.  This is a game over where the player has…
// completed the house.

void DoGameOver (void)
{
	HDC			mainWindowDC;

	playing = false;
	SetUpFinalScreen();
	//SetPort((GrafPtr)mainWindow);
	mainWindowDC = GetMainWindowDC();
	ColorRect(mainWindowDC, &mainWindowRect, 244);
	ReleaseMainWindowDC(mainWindowDC);
	DoGameOverStarAnimation();
	if (!TestHighScore(mainWindow))
		RedrawSplashScreen();
}

//--------------------------------------------------------------  SetUpFinalScreen

// This sets up the game over screen (again, this function is for when…
// the player completes the house).

void SetUpFinalScreen (void)
{
	Rect		tempRect;
	Str255		tempStr, subStr;
	WCHAR		outStr[256];
	SInt16		count, hOffset, vOffset, i, textDown;
	LOGFONT		lfGameOver;
	HFONT		gameOverFont;

	lfGameOver.lfHeight = -12;
	lfGameOver.lfWidth = 0;
	lfGameOver.lfEscapement = 0;
	lfGameOver.lfOrientation = 0;
	lfGameOver.lfWeight = FW_BOLD;
	lfGameOver.lfItalic = FALSE;
	lfGameOver.lfUnderline = FALSE;
	lfGameOver.lfStrikeOut = FALSE;
	lfGameOver.lfCharSet = DEFAULT_CHARSET;
	lfGameOver.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lfGameOver.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfGameOver.lfQuality = DEFAULT_QUALITY;
	lfGameOver.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy(lfGameOver.lfFaceName, L"Tahoma");

	//SetPort((GrafPtr)workSrcMap);
	ColorRect(workSrcMap, &workSrcRect, 244);
	QSetRect(&tempRect, 0, 0, 640, 460);
	CenterRectInRect(&tempRect, &workSrcRect);
	LoadScaledGraphic(workSrcMap, kMilkywayPictID, &tempRect);
	textDown = tempRect.top;
	if (textDown < 0)
		textDown = 0;

	PasStringCopy(thisHouse->trailer, tempStr);

	SaveDC(workSrcMap);
	gameOverFont = CreateFontIndirect(&lfGameOver);
	SelectObject(workSrcMap, gameOverFont);
	SetBkMode(workSrcMap, TRANSPARENT);
	SetTextAlign(workSrcMap, TA_BASELINE | TA_CENTER);
	count = 0;
	do
	{
		GetLineOfText(tempStr, count, subStr);
		WinFromMacString(outStr, ARRAYSIZE(outStr), subStr);
		hOffset = HalfRectWide(&thisMac.screen);
		vOffset = textDown + 32 + (count * 20);
		SetTextColor(workSrcMap, blackColor);
		TextOut(workSrcMap, hOffset + 1, vOffset + 1, outStr, subStr[0]);
		SetTextColor(workSrcMap, whiteColor);
		TextOut(workSrcMap, hOffset, vOffset, outStr, subStr[0]);
		count++;
	}
	while (subStr[0] > 0);
	RestoreDC(workSrcMap, -1);
	DeleteObject(gameOverFont);

	CopyRectWorkToBack(&workSrcRect);

	for (i = 0; i < 5; i++)		// initialize the falling stars
	{
		pages[i].dest = starSrc[0];
		QOffsetRect(&pages[i].dest,
				workSrcRect.right + RandomInt(workSrcRect.right / 5) +
				(workSrcRect.right/ 4) * i,
				RandomInt(workSrcRect.bottom) - workSrcRect.bottom / 2);
		pages[i].was = pages[i].dest;
		pages[i].frame = RandomInt(6);
	}
}

//--------------------------------------------------------------  DoGameOverStarAnimation

// This handles the falling stars and the flying angel when a player…
// completes a house.

void DoGameOverStarAnimation (void)
{
	#define		kStarFalls	8
	MSG			msg;
	Rect		angelDest;
	SInt32		nextLoop;
	SInt16		which, i, count, pass;
	Boolean		noInteruption;

	angelDest = angelSrcRect;
	QOffsetRect(&angelDest, -96, 0);
	noInteruption = true;
	nextLoop = MillisToTicks(GetTickCount()) + 2;
	count = 0;
	pass = 0;

	while (noInteruption)
	{
		if ((angelDest.left % 32) == 0 && angelDest.left >= 0)		// add a star
		{
			PlayPrioritySound(kMysticSound, kMysticPriority);
			which = angelDest.left / 32;
			which = which % 5;
			ZeroRectCorner(&pages[which].dest);
			QOffsetRect(&pages[which].dest, angelDest.left, angelDest.bottom);
			if (count < (which + 1))
				count = which + 1;
		}

		for (i = 0; i < count; i++)
		{
			pages[i].frame++;
			if (pages[i].frame >= 6)
				pages[i].frame = 0;

			Mac_CopyMask(bonusSrcMap, bonusMaskMap, workSrcMap,
					&starSrc[pages[i].frame],
					&starSrc[pages[i].frame],
					&pages[i].dest);

			pages[i].was = pages[i].dest;
			pages[i].was.top -= kStarFalls;

			AddRectToWorkRectsWhole(&pages[i].was);
			AddRectToBackRects(&pages[i].dest);

			if (pages[i].dest.top < workSrcRect.bottom)
				QOffsetRect(&pages[i].dest, 0, kStarFalls);
		}

		if (angelDest.left <= (workSrcRect.right + 2))
		{
			Mac_CopyMask(angelSrcMap, angelMaskMap, workSrcMap,
					&angelSrcRect, &angelSrcRect, &angelDest);
			angelDest.left -= 2;
			AddRectToWorkRectsWhole(&angelDest);
			angelDest.left += 2;
			AddRectToBackRects(&angelDest);
			QOffsetRect(&angelDest, 2, 0);
			pass = 0;
		}

		CopyRectsQD();

		numWork2Main = 0;
		numBack2Work = 0;

		do
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					PostQuitMessage((int)msg.wParam);
					noInteruption = false;
					break;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				switch (msg.message)
				{
				case WM_KEYDOWN:
				case WM_LBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_XBUTTONDOWN:
					noInteruption = false;
					break;
				}
			}
		}
		while ((SInt32)MillisToTicks(GetTickCount()) < nextLoop);
		nextLoop = MillisToTicks(GetTickCount()) + 2;

		if (pass < 80)
			pass++;
		else
		{
			WaitForInputEvent(5);
			noInteruption = false;
		}
	}
}

//--------------------------------------------------------------  FlagGameOver

// Called to indicate (flag) that a game is over.  Actual game over…
// sequence comes up after a short delay.

void FlagGameOver (void)
{
	gameOver = true;
	countDown = kNumCountDownFrames;
	SetMusicalMode(kPlayWholeScoreMode);
}

//--------------------------------------------------------------  InitDiedGameOver
// This is called when a game is over due to the fact that the player…
// lost their last glider (died), not due to getting through the entire…
// house.  This function initializes the strucures/variables.

void InitDiedGameOver (void)
{
	#define		kPageSpacing		40
	#define		kPageRightOffset	128
	#define		kPageBackUp			128
	SInt16		i;
	OSErr		theErr;

	QSetRect(&pageSrcRect, 0, 0, 25, 32 * 8);
	theErr = CreateOffScreenGWorld(&gameOverSrcMap, &pageSrcRect, kPreferredDepth);
	LoadGraphic(gameOverSrcMap, kLettersPictID);

	QSetRect(&pageSrcRect, 0, 0, 32, 32 * kPageFrames);
	theErr = CreateOffScreenGWorld(&pageSrcMap, &pageSrcRect, kPreferredDepth);
	LoadGraphic(pageSrcMap, kPagesPictID);

	theErr = CreateOffScreenGWorld(&pageMaskMap, &pageSrcRect, 1);
	LoadGraphic(pageMaskMap, kPagesMaskID);

	for (i = 0; i < kPageFrames; i++)	// initialize src page rects
	{
		QSetRect(&pageSrc[i], 0, 0, 32, 32);
		QOffsetRect(&pageSrc[i], 0, 32 * i);
	}

	for (i = 0; i < 8; i++)				// initialize dest page rects
	{
		QSetRect(&pages[i].dest, 0, 0, 32, 32);
		CenterRectInRect(&pages[i].dest, &thisMac.screen);
		QOffsetRect(&pages[i].dest, -thisMac.screen.left, -thisMac.screen.top);
		if (i < 4)
			QOffsetRect(&pages[i].dest, -kPageSpacing * (4 - i), 0);
		else
			QOffsetRect(&pages[i].dest, kPageSpacing * (i - 3), 0);
		QOffsetRect(&pages[i].dest, (thisMac.screen.right - thisMac.screen.left) / -2,
				(thisMac.screen.right - thisMac.screen.left) / -2);
		if (pages[i].dest.left % 2 == 1)
			QOffsetRect(&pages[i].dest, 1, 0);
		pages[i].was = pages[i].dest;
		pages[i].frame = 0;
		pages[i].counter = RandomInt(32);
		pages[i].stuck = false;
	}

	for (i = 0; i < 8; i++)
	{
		QSetRect(&lettersSrc[i], 0, 0, 25, 32);
		QOffsetRect(&lettersSrc[i], 0, 32 * i);
	}

	roomRgn = CreateRectRgn(justRoomsRect.left, justRoomsRect.top,
			justRoomsRect.right, justRoomsRect.bottom);
	pagesStuck = 0;
	stopPages = ((thisMac.screen.bottom - thisMac.screen.top) / 2) - 16;
}

//--------------------------------------------------------------  HandlePages

// This handles the pieces of paper that blow across the screen.

void HandlePages (void)
{
	SInt16		i;

	for (i = 0; i < 8; i++)
	{
		if ((pages[i].dest.bottom + RandomInt(8)) > stopPages)
		{
			pages[i].frame = 0;
			if (!pages[i].stuck)
			{
				pages[i].dest.right = pages[i].dest.left + 25;
				pages[i].stuck = true;
				pagesStuck++;
			}
		}
		else
		{
			if (pages[i].frame == 0)
			{
				pages[i].counter--;
				if (pages[i].counter <= 0)
					pages[i].frame = 1;
			}
			else if (pages[i].frame == 7)
			{
				pages[i].counter--;
				if (pages[i].counter <= 0)
				{
					pages[i].frame = 8;
					if (RandomInt(2) == 0)
						PlayPrioritySound(kPaper3Sound, kPapersPriority);
					else
						PlayPrioritySound(kPaper4Sound, kPapersPriority);
				}
				else
					QOffsetRect(&pages[i].dest, 10, 10);
			}
			else
			{
				pages[i].frame++;
				switch (pages[i].frame)
				{
					case 5:
					QOffsetRect(&pages[i].dest, 6, 6);
					break;

					case 6:
					QOffsetRect(&pages[i].dest, 8, 8);
					break;

					case 7:
					QOffsetRect(&pages[i].dest, 8, 8);
					pages[i].counter = RandomInt(4) + 4;
					break;

					case 8:
					case 9:
					QOffsetRect(&pages[i].dest, 8, 8);
					break;

					case 10:
					QOffsetRect(&pages[i].dest, 6, 6);
					break;

					case kPageFrames:
					QOffsetRect(&pages[i].dest, 8, 0);
					pages[i].frame = 0;
					pages[i].counter = RandomInt(8) + 8;
					if (RandomInt(2) == 0)
						PlayPrioritySound(kPaper1Sound, kPapersPriority);
					else
						PlayPrioritySound(kPaper2Sound, kPapersPriority);
					break;
				}
			}
		}
	}
}

//--------------------------------------------------------------  DrawPages

// This function does the drawing for the pieces of paper that blow…
// across the screen.

void DrawPages (void)
{
	SInt16		i;

	for (i = 0; i < 8; i++)
	{
		if (pages[i].stuck)
		{
			Mac_CopyBits(gameOverSrcMap, workSrcMap,
					&lettersSrc[i], &pages[i].dest,
					srcCopy, roomRgn);
		}
		else
		{
			Mac_CopyMask(pageSrcMap, pageMaskMap, workSrcMap,
					&pageSrc[pages[i].frame],
					&pageSrc[pages[i].frame],
					&pages[i].dest);
		}

		QUnionSimilarRect(&pages[i].dest, &pages[i].was, &pages[i].was);
		AddRectToWorkRects(&pages[i].was);
		AddRectToBackRects(&pages[i].dest);

		CopyRectsQD();

		numWork2Main = 0;
		numBack2Work = 0;

		pages[i].was = pages[i].dest;
	}
}

//--------------------------------------------------------------  DoDiedGameOver

// This is called when a game is over due to the fact that the player…
// lost their last glider (died), not due to getting through the entire…
// house.

void DoDiedGameOver (void)
{
	MSG			msg;
	SInt32		nextLoop;
	Boolean		userAborted;

	userAborted = false;
	InitDiedGameOver();
	CopyRectMainToWork(&workSrcRect);
	CopyRectMainToBack(&workSrcRect);

	nextLoop = MillisToTicks(GetTickCount()) + 2;
	while (pagesStuck < 8)
	{
		HandlePages();
		DrawPages();
		do
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					PostQuitMessage((int)msg.wParam);
					pagesStuck = 8;
					userAborted = true;
					break;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				switch (msg.message)
				{
					case WM_KEYDOWN:
					case WM_LBUTTONDOWN:
					case WM_MBUTTONDOWN:
					case WM_RBUTTONDOWN:
					case WM_XBUTTONDOWN:
					pagesStuck = 8;
					userAborted = true;
					break;
				}
			}
		}
		while ((SInt32)MillisToTicks(GetTickCount()) < nextLoop);
		nextLoop = MillisToTicks(GetTickCount()) + 2;
	}

	if (roomRgn != nil)
		DeleteObject(roomRgn);

	DisposeGWorld(pageSrcMap);
	pageSrcMap = nil;

	DisposeGWorld(pageMaskMap);
	pageMaskMap = nil;

	DisposeGWorld(gameOverSrcMap);
	gameOverSrcMap = nil;
	playing = false;

	if (demoGoing)
	{
		if (!userAborted)
			WaitForInputEvent(1);
	}
	else
	{
		if (!userAborted)
			WaitForInputEvent(10);
		TestHighScore(mainWindow);
	}
	RedrawSplashScreen();
}

