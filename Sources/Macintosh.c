#include "Macintosh.h"

#include "WinAPI.h"
#include <inttypes.h>
#include <limits.h>
#include <string.h>


#define CP_MACROMAN 10000


//--------------------------------------------------------------  CopyBits
// Copy some portion of a bitmap from one graphics port to another.

void Mac_CopyBits(
	HDC srcBits,
	HDC dstBits,
	const Rect* srcRect,
	const Rect* dstRect,
	SInt16 mode,
	HRGN maskRgn)
{
	INT xSrc, ySrc, wSrc, hSrc;
	INT xDst, yDst, wDst, hDst;

	if (srcRect->left >= srcRect->right || srcRect->top >= srcRect->bottom)
		return;
	if (dstRect->left >= dstRect->right || dstRect->top >= dstRect->bottom)
		return;
	if (mode != srcCopy && mode != srcXor && mode != transparent)
		return;

	xSrc = srcRect->left;
	ySrc = srcRect->top;
	wSrc = srcRect->right - srcRect->left;
	hSrc = srcRect->bottom - srcRect->top;
	xDst = dstRect->left;
	yDst = dstRect->top;
	wDst = dstRect->right - dstRect->left;
	hDst = dstRect->bottom - dstRect->top;

	SaveDC(dstBits);
	if (maskRgn != NULL)
		SelectClipRgn(dstBits, maskRgn);

	switch (mode)
	{
		case srcCopy:
		StretchBlt(dstBits, xDst, yDst, wDst, hDst,
				srcBits, xSrc, ySrc, wSrc, hSrc, SRCCOPY);
		break;

		case srcXor:
		// invert the destination where the source is black
		// and keep the destination where the source is white
		BitBlt(dstBits, xDst, yDst, wDst, hDst, NULL, 0, 0, DSTINVERT);
		SetBkColor(dstBits, RGB(0xFF, 0xFF, 0xFF));
		SetTextColor(dstBits, RGB(0x00, 0x00, 0x00));
		StretchBlt(dstBits, xDst, yDst, wDst, hDst,
				srcBits, xSrc, ySrc, wSrc, hSrc, SRCINVERT);
		break;

		case transparent:
		TransparentBlt(dstBits, xDst, yDst, wDst, hDst,
				srcBits, xSrc, ySrc, wSrc, hSrc, GetBkColor(dstBits));
		break;
	}

	RestoreDC(dstBits, -1);
}

//--------------------------------------------------------------  CopyBits
// Copy some portion of a bitmap from one graphics port to another,
// with a mask to specify how much of each pixel is copied over.

void Mac_CopyMask(
	HDC srcBits,
	HDC maskBits,
	HDC dstBits,
	const Rect* srcRect,
	const Rect* maskRect,
	const Rect* dstRect)
{
	// TODO: add support for masks where pixels are between black and white
	HDC newSrcBits;
	HBITMAP newSrcBitmap, hbmPrev;
	INT xSrc, ySrc, wSrc, hSrc;
	INT xMask, yMask, wMask, hMask;
	INT xDst, yDst, wDst, hDst;

	if (srcRect->left >= srcRect->right || srcRect->top >= srcRect->bottom)
		return;
	if (maskRect->left >= maskRect->right || maskRect->top >= maskRect->bottom)
		return;
	if (dstRect->left >= dstRect->right || dstRect->top >= dstRect->bottom)
		return;

	xSrc = srcRect->left;
	ySrc = srcRect->top;
	wSrc = srcRect->right - srcRect->left;
	hSrc = srcRect->bottom - srcRect->top;
	xMask = maskRect->left;
	yMask = maskRect->top;
	wMask = maskRect->right - maskRect->left;
	hMask = maskRect->bottom - maskRect->top;
	xDst = dstRect->left;
	yDst = dstRect->top;
	wDst = dstRect->right - dstRect->left;
	hDst = dstRect->bottom - dstRect->top;

	SaveDC(dstBits);
	// make sure that monochrome masks are colorized correctly
	SetTextColor(dstBits, RGB(0x00, 0x00, 0x00));
	SetBkColor(dstBits, RGB(0xFF, 0xFF, 0xFF));
	// apply the mask to the destination
	StretchBlt(dstBits, xDst, yDst, wDst, hDst,
		maskBits, xMask, yMask, wMask, hMask, SRCAND); // DSa
	// create temporary drawing surface to adjust the source bits
	newSrcBits = CreateCompatibleDC(srcBits);
	SetTextColor(newSrcBits, RGB(0x00, 0x00, 0x00));
	SetBkColor(newSrcBits, RGB(0xFF, 0xFF, 0xFF));
	newSrcBitmap = CreateCompatibleBitmap(srcBits, wSrc, hSrc);
	hbmPrev = SelectObject(newSrcBits, newSrcBitmap);
	BitBlt(newSrcBits, 0, 0, wSrc, hSrc, srcBits, xSrc, ySrc, SRCCOPY);
	// black out unwanted source bits with the inverted mask
	StretchBlt(newSrcBits, 0, 0, wSrc, hSrc,
		maskBits, xMask, yMask, wMask, hMask, 0x00220326); // DSna
	// apply adjusted source bits to the destination
	StretchBlt(dstBits, xDst, yDst, wDst, hDst,
		newSrcBits, 0, 0, wSrc, hSrc, SRCPAINT); // DSo
	// clean up
	RestoreDC(dstBits, -1);
	SelectObject(newSrcBits, hbmPrev);
	DeleteObject(newSrcBitmap);
	DeleteDC(newSrcBits);
}

//--------------------------------------------------------------  DeltaPoint
// Calculate the difference between two points by subtracting the coordinates
// of ptB from the coordinates of ptA. The high-order word of the result is
// the difference in the vertical coordinates, and the low-order word of the
// result is the difference of the horizontal coordinates.

SInt32 Mac_DeltaPoint(Point ptA, Point ptB)
{
	UInt32 vertDiff, horzDiff;

	vertDiff = (UInt32)(UInt16)(SInt16)(ptA.v - ptB.v);
	horzDiff = (UInt32)(UInt16)(SInt16)(ptA.h - ptB.h);

	return (vertDiff << 16) | (horzDiff);
}

//--------------------------------------------------------------  DrawPicture
// Draw the given bitmap into the destination output device. The bitmap
// is stretched or shrinked as necessary to fit exactly with the given
// destination rectangle.

void Mac_DrawPicture(HDC hdcDst, HBITMAP myPicture, const Rect *dstRect)
{
	HDC hdcSrc;
	HGDIOBJ prevBitmap;
	BITMAP bmInfo;
	INT xSrc, ySrc, wSrc, hSrc;
	INT xDst, yDst, wDst, hDst;

	if (dstRect->left >= dstRect->right || dstRect->top >= dstRect->bottom)
		return;
	if (GetObject(myPicture, sizeof(bmInfo), &bmInfo) != sizeof(bmInfo))
		return;

	xSrc = 0;
	ySrc = 0;
	wSrc = bmInfo.bmWidth;
	hSrc = bmInfo.bmHeight;
	xDst = dstRect->left;
	yDst = dstRect->top;
	wDst = dstRect->right - dstRect->left;
	hDst = dstRect->bottom - dstRect->top;

	hdcSrc = CreateCompatibleDC(NULL);
	prevBitmap = SelectObject(hdcSrc, myPicture);
	StretchBlt(hdcDst, xDst, yDst, wDst, hDst,
		hdcSrc, xSrc, ySrc, wSrc, hSrc, SRCCOPY);
	SelectObject(hdcSrc, prevBitmap);
	DeleteDC(hdcSrc);
}

//--------------------------------------------------------------  DrawString
// Draw the given character string, starting from the current position
// and using the current settings. The reference point of the text is
// the left edge, at the baseline (TA_LEFT | TA_BASELINE in GDI).
// The current position is updated by this function to the right edge
// of the text, at the baseline (TA_UPDATECP in GDI).

void Mac_DrawString(HDC hdc, StringPtr s)
{
	WCHAR buffer[256];
	INT prevBkMode;
	UINT prevTextAlign;

	WinFromMacString(buffer, ARRAYSIZE(buffer), s);
	prevBkMode = SetBkMode(hdc, TRANSPARENT);
	prevTextAlign = SetTextAlign(hdc, TA_LEFT | TA_BASELINE | TA_UPDATECP);
	// The MacRoman string's length byte is used here, because each
	// MacRoman byte corresponds to only one UTF-16 code unit.
	TextOut(hdc, 0, 0, buffer, s[0]);
	SetTextAlign(hdc, prevTextAlign);
	SetBkMode(hdc, prevBkMode);
}

//--------------------------------------------------------------  EqualString
// Compare two Pascal strings for equality. If the third parameter is
// nonzero (true), then the comparison is case-sensitive. If the third
// parameter is zero (false), then the comparison is case-insensitive.

Boolean Mac_EqualString(StringPtr aStr, StringPtr bStr, Boolean caseSens)
{
	WCHAR firstString[256];
	WCHAR secondString[256];

	if (aStr[0] != bStr[0])
		return false;

	WinFromMacString(firstString, ARRAYSIZE(firstString), aStr);
	WinFromMacString(secondString, ARRAYSIZE(secondString), bStr);
	if (caseSens)
		return (lstrcmp(firstString, secondString) == 0);
	else
		return (lstrcmpi(firstString, secondString) == 0);
}

//--------------------------------------------------------------  FrameRect
// Draw a frame within the specified rectangle using the current brush.
// The vertical strokes use the specified width, and the horizontal
// strokes use the specified height.

void Mac_FrameRect(HDC hdc, const Rect *r, HBRUSH hbr, SInt16 w, SInt16 h)
{
	HRGN theRgn = CreateRectRgn(r->left, r->top, r->right, r->bottom);
	FrameRgn(hdc, theRgn, hbr, w, h);
	DeleteObject(theRgn);
}

//--------------------------------------------------------------  GetDateTime
// Retrieve the number of seconds since midnight, January 1, 1904.
// The time difference is in terms of the local time zone.

void Mac_GetDateTime(UInt32 *secs)
{
	SYSTEMTIME localMacEpoch, utcMacEpoch;
	FILETIME epochTime, currentTime;
	ULARGE_INTEGER epochSecs, currentSecs;

	if (secs == NULL)
		return;
	*secs = 0;

	localMacEpoch.wYear = 1904;
	localMacEpoch.wMonth = 1;
	localMacEpoch.wDayOfWeek = 5; // Friday
	localMacEpoch.wDay = 1;
	localMacEpoch.wHour = 0;
	localMacEpoch.wMinute = 0;
	localMacEpoch.wSecond = 0;
	localMacEpoch.wMilliseconds = 0;
	if (!TzSpecificLocalTimeToSystemTime(NULL, &localMacEpoch, &utcMacEpoch))
		return;

	if (!SystemTimeToFileTime(&utcMacEpoch, &epochTime))
		return;
	epochSecs.LowPart = epochTime.dwLowDateTime;
	epochSecs.HighPart = epochTime.dwHighDateTime;
	epochSecs.QuadPart /= (10 * 1000 * 1000);

	GetSystemTimeAsFileTime(&currentTime);
	currentSecs.LowPart = currentTime.dwLowDateTime;
	currentSecs.HighPart = currentTime.dwHighDateTime;
	currentSecs.QuadPart /= (10 * 1000 * 1000);

	*secs = (UInt32)(currentSecs.QuadPart - epochSecs.QuadPart);
}

//--------------------------------------------------------------  InsetRect
// Shrink or expand the given rectangle. The rectangle's sides
// are moved inwards by the given deltas. To expand the rectangle,
// call this function with negative delta values. If either the width
// or the height of the resulting rectangle is less than 1, then the
// rectangle is set to the empty rectangle (0,0,0,0).

void Mac_InsetRect(Rect *r, SInt16 dh, SInt16 dv)
{
	if (r == NULL)
		return;
	r->left += dh;
	r->top += dv;
	r->right -= dh;
	r->bottom -= dv;
	if (r->left > r->right || r->top > r->bottom)
	{
		r->left = 0;
		r->top = 0;
		r->right = 0;
		r->bottom = 0;
	}
}

void Mac_InvalWindowRect(HWND window, const Rect *bounds)
{
	RECT rcDirty;

	SetRect(&rcDirty, bounds->left, bounds->top, bounds->right, bounds->bottom);
	InvalidateRect(window, &rcDirty, FALSE);
}

//--------------------------------------------------------------  Line
// Draw a line from the current position to the specified offsets.
// 'dh' specifies the horizontal movement and 'dv' specifies the
// vertical movement of the current pen. Negative values move the
// pen left or up, and positive values move the pen right or down.
// NOTE: Macintosh lines include the end pixel, unlike GDI lines.

void Mac_Line(HDC hdc, SInt16 dh, SInt16 dv)
{
	POINT curPos;

	GetCurrentPositionEx(hdc, &curPos);
	//MoveToEx(hdc, curPos.x + dh, curPos.y + dv, NULL);
	LineTo(hdc, curPos.x + dh, curPos.y + dv);
	LineTo(hdc, curPos.x, curPos.y);
	LineTo(hdc, curPos.x + dh, curPos.y + dv);
}

//--------------------------------------------------------------  LineTo
// Draw a line from the current position to the specified position.
// 'h' and 'v' specify the coordinates of the line's end point.
// NOTE: Macintosh lines include the end pixel, unlike GDI lines.

void Mac_LineTo(HDC hdc, SInt16 h, SInt16 v)
{
	POINT curPos;

	GetCurrentPositionEx(hdc, &curPos);
	//MoveToEx(hdc, h, v, NULL);
	LineTo(hdc, h, v);
	LineTo(hdc, curPos.x, curPos.y);
	LineTo(hdc, h, v);
}

//--------------------------------------------------------------  NumToString
// Convert the given number to a decimal string representation.
// The string is written to the given output string.

void Mac_NumToString(SInt32 theNum, StringPtr theString)
{
	CHAR buffer[sizeof("-2147483648")];
	size_t length;
	HRESULT hr;

	if (theString == NULL)
		return;
	theString[0] = 0; // return an empty string if an error occurs.
	hr = StringCchPrintfA(buffer, ARRAYSIZE(buffer), "%" PRId32, theNum);
	if (FAILED(hr))
		return;
	hr = StringCchLengthA(buffer, ARRAYSIZE(buffer), &length);
	if (FAILED(hr))
		return;
	theString[0] = (Byte)length;
	memcpy(&theString[1], buffer, theString[0]);
}

//--------------------------------------------------------------  PaintRect
// Fill the specified rectangle with the current brush.

void Mac_PaintRect(HDC hdc, const Rect *r, HBRUSH hbr)
{
	HRGN theRgn = CreateRectRgn(r->left, r->top, r->right, r->bottom);
	FillRgn(hdc, theRgn, hbr);
	DeleteObject(theRgn);
}

//--------------------------------------------------------------  PtInRect
// Check if the given point is enclosed within the given rectangle.
// Return nonzero (true) if the point is within the rectangle; otherwise
// return zero (false). The point is considered within the rectangle
// if it is within all four sides, or if it lies on the left or top side.
// The point is not considered within the rectangle if it lies on the
// right or bottom side (because its corresponding pixel would be
// outside the rectangle).

Boolean Mac_PtInRect(Point pt, const Rect *r)
{
	if (r == NULL)
		return false;
	return (pt.h >= r->left) && (pt.h < r->right) &&
			(pt.v >= r->top) && (pt.v < r->bottom);
}

//--------------------------------------------------------------  SectRect
// Calculate the intersection of the two source rectangles, and write
// that intersection into the destination rectangle. If the source
// rectangles do not intersect, then the destination rectangle is set
// to all zeroes. The return value is TRUE if the source rectangles
// intersect, and FALSE if they do not intersect.

Boolean Mac_SectRect(const Rect *src1, const Rect *src2, Rect *dstRect)
{
	Rect sectRect;

	sectRect.left = 0;
	sectRect.top = 0;
	sectRect.right = 0;
	sectRect.bottom = 0;
	// If either source rectangle is empty, then the intersection is empty.
	if ((src1->left >= src1->right) || (src1->top >= src1->bottom) ||
		(src2->left >= src2->right) || (src2->top >= src2->bottom))
	{
		*dstRect = sectRect;
		return false;
	}
	// If the first rectangle is above, below, or to the side of the second
	// rectangle (touching or not), then the intersection is empty.
	if ((src1->bottom <= src2->top) || (src2->bottom <= src1->top) ||
		(src1->right <= src2->left) || (src2->right <= src1->left))
	{
		*dstRect = sectRect;
		return false;
	}
	// The two rectangles intersect, so calculate the intersection's boundary.
	sectRect.left = (src1->left > src2->left) ? src1->left : src2->left;
	sectRect.top = (src1->top > src2->top) ? src1->top : src2->top;
	sectRect.right = (src1->right <= src2->right) ? src1->right : src2->right;
	sectRect.bottom = (src1->bottom <= src2->bottom) ? src1->bottom : src2->bottom;
	*dstRect = sectRect;
	return true;
}

//--------------------------------------------------------------  StringWidth
// Calculate the width, in logical units, of the given Pascal string,
// using the currently selected font.

SInt16 Mac_StringWidth(HDC hdc, StringPtr s)
{
	WCHAR buffer[256];
	SIZE extents;

	WinFromMacString(buffer, ARRAYSIZE(buffer), s);
	if (!GetTextExtentPoint32(hdc, buffer, (int)wcslen(buffer), &extents))
		return 0;
	return (SInt16)extents.cx;
}

//--------------------------------------------------------------  WinFromMacString
// Convert a MacRoman Pascal-style string to a UTF-16 C-style string.
// If the length of the converted string exceeds the output buffer's capacity,
// then the converted string is truncated.
// This is a wrapper around the Windows API function 'MultiByteToWideChar'.
// The return value of this function is the return value of 'MultiByteToWideChar'.
// Return nonzero on success and zero on failure.

int WinFromMacString(wchar_t *winbuf, int winlen, ConstStringPtr macbuf)
{
	int copySize, result;

	// Calculate the number of Mac characters to copy
	copySize = macbuf[0];
	if (copySize > winlen - 1)
		copySize = winlen - 1;

	// Do the conversion and terminate the output string
	result = MultiByteToWideChar(CP_MACROMAN, 0,
			(LPCCH)&macbuf[1], copySize, winbuf, winlen - 1);
	winbuf[result] = L'\0';
	return result;
}

//--------------------------------------------------------------  MacFromWinString
// Convert a UTF-16 C-style string to a MacRoman Pascal-style string.
// (This may result in data-loss!)
// If the length of the converted string exceeds the output buffer's capacity,
// then the converted string is truncated.
// This is a wrapper around the Windows API function 'WideCharToMultiByte'.
// The return value of this function is the return value of 'WideCharToMultiByte'.
// Return nonzero on success and zero on failure.

int MacFromWinString(StringPtr macbuf, int maclen, const wchar_t *winbuf)
{
	size_t inputSize;
	int copySize, result;

	// Calculate the number of wide characters to copy
	inputSize = wcslen(winbuf);
	if (inputSize > INT_MAX)
		inputSize = INT_MAX;
	copySize = (int)inputSize;
	if (copySize > maclen - 1)
		copySize = maclen - 1;

	// Do the conversion and save the length
	result = WideCharToMultiByte(CP_MACROMAN, 0,
			winbuf, copySize, (LPSTR)&macbuf[1], maclen - 1,
			NULL, NULL);
	macbuf[0] = (Byte)result;
	return result;
}

//--------------------------------------------------------------  Global Data

// The eight basic QuickDraw colors
const RGBColor QDColors[] = {
	{ 0x0000, 0x0000, 0x0000 }, // black
	{ 0xFC00, 0xF37D, 0x052F }, // yellow
	{ 0xF2D7, 0x0856, 0x84EC }, // magenta
	{ 0xDD6B, 0x08C2, 0x06A2 }, // red
	{ 0x0241, 0xAB54, 0xEAFF }, // cyan
	{ 0x0000, 0x8000, 0x11B0 }, // green
	{ 0x0000, 0x0000, 0xD400 }, // blue
	{ 0xFFFF, 0xFFFF, 0xFFFF }, // white
};

// The default 4-bit Macintosh color palette
const RGBColor MacColor4[] = {
	{ 0xFFFF, 0xFFFF, 0xFFFF }, // white
	{ 0xFC00, 0xF37D, 0x052F }, // yellow
	{ 0xFFFF, 0x648A, 0x028C }, // orange
	{ 0xDD6B, 0x08C2, 0x06A2 }, // red
	{ 0xF2D7, 0x0856, 0x84EC }, // magenta
	{ 0x46E3, 0x0000, 0xA53E }, // purple
	{ 0x0000, 0x0000, 0xD400 }, // blue
	{ 0x0241, 0xAB54, 0xEAFF }, // cyan
	{ 0x1F21, 0xB793, 0x1431 }, // green
	{ 0x0000, 0x64AF, 0x11B0 }, // dark green
	{ 0x5600, 0x2C9D, 0x0524 }, // brown
	{ 0x90D7, 0x7160, 0x3A34 }, // tan
	{ 0xC000, 0xC000, 0xC000 }, // light gray
	{ 0x8000, 0x8000, 0x8000 }, // medium gray
	{ 0x4000, 0x4000, 0x4000 }, // dark gray
	{ 0x0000, 0x0000, 0x0000 }, // black
};

// The default 8-bit Macintosh color palette
const RGBColor MacColor8[] = {
	{ 0xFFFF, 0xFFFF, 0xFFFF }, { 0xFFFF, 0xFFFF, 0xCCCC }, { 0xFFFF, 0xFFFF, 0x9999 },
	{ 0xFFFF, 0xFFFF, 0x6666 }, { 0xFFFF, 0xFFFF, 0x3333 }, { 0xFFFF, 0xFFFF, 0x0000 },
	{ 0xFFFF, 0xCCCC, 0xFFFF }, { 0xFFFF, 0xCCCC, 0xCCCC }, { 0xFFFF, 0xCCCC, 0x9999 },
	{ 0xFFFF, 0xCCCC, 0x6666 }, { 0xFFFF, 0xCCCC, 0x3333 }, { 0xFFFF, 0xCCCC, 0x0000 },
	{ 0xFFFF, 0x9999, 0xFFFF }, { 0xFFFF, 0x9999, 0xCCCC }, { 0xFFFF, 0x9999, 0x9999 },
	{ 0xFFFF, 0x9999, 0x6666 }, { 0xFFFF, 0x9999, 0x3333 }, { 0xFFFF, 0x9999, 0x0000 },
	{ 0xFFFF, 0x6666, 0xFFFF }, { 0xFFFF, 0x6666, 0xCCCC }, { 0xFFFF, 0x6666, 0x9999 },
	{ 0xFFFF, 0x6666, 0x6666 }, { 0xFFFF, 0x6666, 0x3333 }, { 0xFFFF, 0x6666, 0x0000 },
	{ 0xFFFF, 0x3333, 0xFFFF }, { 0xFFFF, 0x3333, 0xCCCC }, { 0xFFFF, 0x3333, 0x9999 },
	{ 0xFFFF, 0x3333, 0x6666 }, { 0xFFFF, 0x3333, 0x3333 }, { 0xFFFF, 0x3333, 0x0000 },
	{ 0xFFFF, 0x0000, 0xFFFF }, { 0xFFFF, 0x0000, 0xCCCC }, { 0xFFFF, 0x0000, 0x9999 },
	{ 0xFFFF, 0x0000, 0x6666 }, { 0xFFFF, 0x0000, 0x3333 }, { 0xFFFF, 0x0000, 0x0000 },

	{ 0xCCCC, 0xFFFF, 0xFFFF }, { 0xCCCC, 0xFFFF, 0xCCCC }, { 0xCCCC, 0xFFFF, 0x9999 },
	{ 0xCCCC, 0xFFFF, 0x6666 }, { 0xCCCC, 0xFFFF, 0x3333 }, { 0xCCCC, 0xFFFF, 0x0000 },
	{ 0xCCCC, 0xCCCC, 0xFFFF }, { 0xCCCC, 0xCCCC, 0xCCCC }, { 0xCCCC, 0xCCCC, 0x9999 },
	{ 0xCCCC, 0xCCCC, 0x6666 }, { 0xCCCC, 0xCCCC, 0x3333 }, { 0xCCCC, 0xCCCC, 0x0000 },
	{ 0xCCCC, 0x9999, 0xFFFF }, { 0xCCCC, 0x9999, 0xCCCC }, { 0xCCCC, 0x9999, 0x9999 },
	{ 0xCCCC, 0x9999, 0x6666 }, { 0xCCCC, 0x9999, 0x3333 }, { 0xCCCC, 0x9999, 0x0000 },
	{ 0xCCCC, 0x6666, 0xFFFF }, { 0xCCCC, 0x6666, 0xCCCC }, { 0xCCCC, 0x6666, 0x9999 },
	{ 0xCCCC, 0x6666, 0x6666 }, { 0xCCCC, 0x6666, 0x3333 }, { 0xCCCC, 0x6666, 0x0000 },
	{ 0xCCCC, 0x3333, 0xFFFF }, { 0xCCCC, 0x3333, 0xCCCC }, { 0xCCCC, 0x3333, 0x9999 },
	{ 0xCCCC, 0x3333, 0x6666 }, { 0xCCCC, 0x3333, 0x3333 }, { 0xCCCC, 0x3333, 0x0000 },
	{ 0xCCCC, 0x0000, 0xFFFF }, { 0xCCCC, 0x0000, 0xCCCC }, { 0xCCCC, 0x0000, 0x9999 },
	{ 0xCCCC, 0x0000, 0x6666 }, { 0xCCCC, 0x0000, 0x3333 }, { 0xCCCC, 0x0000, 0x0000 },

	{ 0x9999, 0xFFFF, 0xFFFF }, { 0x9999, 0xFFFF, 0xCCCC }, { 0x9999, 0xFFFF, 0x9999 },
	{ 0x9999, 0xFFFF, 0x6666 }, { 0x9999, 0xFFFF, 0x3333 }, { 0x9999, 0xFFFF, 0x0000 },
	{ 0x9999, 0xCCCC, 0xFFFF }, { 0x9999, 0xCCCC, 0xCCCC }, { 0x9999, 0xCCCC, 0x9999 },
	{ 0x9999, 0xCCCC, 0x6666 }, { 0x9999, 0xCCCC, 0x3333 }, { 0x9999, 0xCCCC, 0x0000 },
	{ 0x9999, 0x9999, 0xFFFF }, { 0x9999, 0x9999, 0xCCCC }, { 0x9999, 0x9999, 0x9999 },
	{ 0x9999, 0x9999, 0x6666 }, { 0x9999, 0x9999, 0x3333 }, { 0x9999, 0x9999, 0x0000 },
	{ 0x9999, 0x6666, 0xFFFF }, { 0x9999, 0x6666, 0xCCCC }, { 0x9999, 0x6666, 0x9999 },
	{ 0x9999, 0x6666, 0x6666 }, { 0x9999, 0x6666, 0x3333 }, { 0x9999, 0x6666, 0x0000 },
	{ 0x9999, 0x3333, 0xFFFF }, { 0x9999, 0x3333, 0xCCCC }, { 0x9999, 0x3333, 0x9999 },
	{ 0x9999, 0x3333, 0x6666 }, { 0x9999, 0x3333, 0x3333 }, { 0x9999, 0x3333, 0x0000 },
	{ 0x9999, 0x0000, 0xFFFF }, { 0x9999, 0x0000, 0xCCCC }, { 0x9999, 0x0000, 0x9999 },
	{ 0x9999, 0x0000, 0x6666 }, { 0x9999, 0x0000, 0x3333 }, { 0x9999, 0x0000, 0x0000 },

	{ 0x6666, 0xFFFF, 0xFFFF }, { 0x6666, 0xFFFF, 0xCCCC }, { 0x6666, 0xFFFF, 0x9999 },
	{ 0x6666, 0xFFFF, 0x6666 }, { 0x6666, 0xFFFF, 0x3333 }, { 0x6666, 0xFFFF, 0x0000 },
	{ 0x6666, 0xCCCC, 0xFFFF }, { 0x6666, 0xCCCC, 0xCCCC }, { 0x6666, 0xCCCC, 0x9999 },
	{ 0x6666, 0xCCCC, 0x6666 }, { 0x6666, 0xCCCC, 0x3333 }, { 0x6666, 0xCCCC, 0x0000 },
	{ 0x6666, 0x9999, 0xFFFF }, { 0x6666, 0x9999, 0xCCCC }, { 0x6666, 0x9999, 0x9999 },
	{ 0x6666, 0x9999, 0x6666 }, { 0x6666, 0x9999, 0x3333 }, { 0x6666, 0x9999, 0x0000 },
	{ 0x6666, 0x6666, 0xFFFF }, { 0x6666, 0x6666, 0xCCCC }, { 0x6666, 0x6666, 0x9999 },
	{ 0x6666, 0x6666, 0x6666 }, { 0x6666, 0x6666, 0x3333 }, { 0x6666, 0x6666, 0x0000 },
	{ 0x6666, 0x3333, 0xFFFF }, { 0x6666, 0x3333, 0xCCCC }, { 0x6666, 0x3333, 0x9999 },
	{ 0x6666, 0x3333, 0x6666 }, { 0x6666, 0x3333, 0x3333 }, { 0x6666, 0x3333, 0x0000 },
	{ 0x6666, 0x0000, 0xFFFF }, { 0x6666, 0x0000, 0xCCCC }, { 0x6666, 0x0000, 0x9999 },
	{ 0x6666, 0x0000, 0x6666 }, { 0x6666, 0x0000, 0x3333 }, { 0x6666, 0x0000, 0x0000 },

	{ 0x3333, 0xFFFF, 0xFFFF }, { 0x3333, 0xFFFF, 0xCCCC }, { 0x3333, 0xFFFF, 0x9999 },
	{ 0x3333, 0xFFFF, 0x6666 }, { 0x3333, 0xFFFF, 0x3333 }, { 0x3333, 0xFFFF, 0x0000 },
	{ 0x3333, 0xCCCC, 0xFFFF }, { 0x3333, 0xCCCC, 0xCCCC }, { 0x3333, 0xCCCC, 0x9999 },
	{ 0x3333, 0xCCCC, 0x6666 }, { 0x3333, 0xCCCC, 0x3333 }, { 0x3333, 0xCCCC, 0x0000 },
	{ 0x3333, 0x9999, 0xFFFF }, { 0x3333, 0x9999, 0xCCCC }, { 0x3333, 0x9999, 0x9999 },
	{ 0x3333, 0x9999, 0x6666 }, { 0x3333, 0x9999, 0x3333 }, { 0x3333, 0x9999, 0x0000 },
	{ 0x3333, 0x6666, 0xFFFF }, { 0x3333, 0x6666, 0xCCCC }, { 0x3333, 0x6666, 0x9999 },
	{ 0x3333, 0x6666, 0x6666 }, { 0x3333, 0x6666, 0x3333 }, { 0x3333, 0x6666, 0x0000 },
	{ 0x3333, 0x3333, 0xFFFF }, { 0x3333, 0x3333, 0xCCCC }, { 0x3333, 0x3333, 0x9999 },
	{ 0x3333, 0x3333, 0x6666 }, { 0x3333, 0x3333, 0x3333 }, { 0x3333, 0x3333, 0x0000 },
	{ 0x3333, 0x0000, 0xFFFF }, { 0x3333, 0x0000, 0xCCCC }, { 0x3333, 0x0000, 0x9999 },
	{ 0x3333, 0x0000, 0x6666 }, { 0x3333, 0x0000, 0x3333 }, { 0x3333, 0x0000, 0x0000 },

	{ 0x0000, 0xFFFF, 0xFFFF }, { 0x0000, 0xFFFF, 0xCCCC }, { 0x0000, 0xFFFF, 0x9999 },
	{ 0x0000, 0xFFFF, 0x6666 }, { 0x0000, 0xFFFF, 0x3333 }, { 0x0000, 0xFFFF, 0x0000 },
	{ 0x0000, 0xCCCC, 0xFFFF }, { 0x0000, 0xCCCC, 0xCCCC }, { 0x0000, 0xCCCC, 0x9999 },
	{ 0x0000, 0xCCCC, 0x6666 }, { 0x0000, 0xCCCC, 0x3333 }, { 0x0000, 0xCCCC, 0x0000 },
	{ 0x0000, 0x9999, 0xFFFF }, { 0x0000, 0x9999, 0xCCCC }, { 0x0000, 0x9999, 0x9999 },
	{ 0x0000, 0x9999, 0x6666 }, { 0x0000, 0x9999, 0x3333 }, { 0x0000, 0x9999, 0x0000 },
	{ 0x0000, 0x6666, 0xFFFF }, { 0x0000, 0x6666, 0xCCCC }, { 0x0000, 0x6666, 0x9999 },
	{ 0x0000, 0x6666, 0x6666 }, { 0x0000, 0x6666, 0x3333 }, { 0x0000, 0x6666, 0x0000 },
	{ 0x0000, 0x3333, 0xFFFF }, { 0x0000, 0x3333, 0xCCCC }, { 0x0000, 0x3333, 0x9999 },
	{ 0x0000, 0x3333, 0x6666 }, { 0x0000, 0x3333, 0x3333 }, { 0x0000, 0x3333, 0x0000 },
	{ 0x0000, 0x0000, 0xFFFF }, { 0x0000, 0x0000, 0xCCCC }, { 0x0000, 0x0000, 0x9999 },
	{ 0x0000, 0x0000, 0x6666 }, { 0x0000, 0x0000, 0x3333 },

	{ 0xEEEE, 0x0000, 0x0000 }, { 0xDDDD, 0x0000, 0x0000 },
	{ 0xBBBB, 0x0000, 0x0000 }, { 0xAAAA, 0x0000, 0x0000 },
	{ 0x8888, 0x0000, 0x0000 }, { 0x7777, 0x0000, 0x0000 },
	{ 0x5555, 0x0000, 0x0000 }, { 0x4444, 0x0000, 0x0000 },
	{ 0x2222, 0x0000, 0x0000 }, { 0x1111, 0x0000, 0x0000 },

	{ 0x0000, 0xEEEE, 0x0000 }, { 0x0000, 0xDDDD, 0x0000 },
	{ 0x0000, 0xBBBB, 0x0000 }, { 0x0000, 0xAAAA, 0x0000 },
	{ 0x0000, 0x8888, 0x0000 }, { 0x0000, 0x7777, 0x0000 },
	{ 0x0000, 0x5555, 0x0000 }, { 0x0000, 0x4444, 0x0000 },
	{ 0x0000, 0x2222, 0x0000 }, { 0x0000, 0x1111, 0x0000 },

	{ 0x0000, 0x0000, 0xEEEE }, { 0x0000, 0x0000, 0xDDDD },
	{ 0x0000, 0x0000, 0xBBBB }, { 0x0000, 0x0000, 0xAAAA },
	{ 0x0000, 0x0000, 0x8888 }, { 0x0000, 0x0000, 0x7777 },
	{ 0x0000, 0x0000, 0x5555 }, { 0x0000, 0x0000, 0x4444 },
	{ 0x0000, 0x0000, 0x2222 }, { 0x0000, 0x0000, 0x1111 },

	{ 0xEEEE, 0xEEEE, 0xEEEE }, { 0xDDDD, 0xDDDD, 0xDDDD },
	{ 0xBBBB, 0xBBBB, 0xBBBB }, { 0xAAAA, 0xAAAA, 0xAAAA },
	{ 0x8888, 0x8888, 0x8888 }, { 0x7777, 0x7777, 0x7777 },
	{ 0x5555, 0x5555, 0x5555 }, { 0x4444, 0x4444, 0x4444 },
	{ 0x2222, 0x2222, 0x2222 }, { 0x1111, 0x1111, 0x1111 },
	{ 0x0000, 0x0000, 0x0000 },
};
