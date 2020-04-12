
//============================================================================
//----------------------------------------------------------------------------
//								  ColorUtils.c
//----------------------------------------------------------------------------
//============================================================================


#include "Environ.h"
#include "Externs.h"
//#include <Palettes.h>
#include "Macintosh.h"


//==============================================================  Functions
//--------------------------------------------------------------  Index2ColorRef

// Given an index into the current palette, this function returns the
// corresponding COLORREF value. If the index is out of bounds, then
// RGB(0,0,0) is returned.

COLORREF Index2ColorRef (SInt32 color)
{
	const COLORREF defaultClr = RGB(0x00, 0x00, 0x00);
	RGBColor rgb;
	size_t index;

	if (color < 0)
		return defaultClr;
	index = (size_t)color;

	if (thisMac.isDepth == 4)
	{
		if (index >= ARRAYSIZE(MacColor4))
			return defaultClr;
		rgb = MacColor4[index];
	}
	else
	{
		if (index >= ARRAYSIZE(MacColor8))
			return defaultClr;
		rgb = MacColor8[index];
	}

	return RGB(rgb.red >> 8, rgb.green >> 8, rgb.blue >> 8);
}

//--------------------------------------------------------------  ColorText

// Given a string and a color index (index into the current palette),…
// this function draws text in that color.  It assumes the current port,…
// the current font, the current pen location, etc.

void ColorText (HDC hdc, StringPtr theStr, SInt32 color)
{
	COLORREF	theRGBColor, wasColor;

	theRGBColor = Index2ColorRef(color);
	wasColor = SetTextColor(hdc, theRGBColor);
	Mac_DrawString(hdc, theStr);
	SetTextColor(hdc, wasColor);
}

//--------------------------------------------------------------  ColorRect

// Given a rectangle and color index, this function draws a solid…
// rectangle in that color.  Current port, pen mode, etc. assumed.

void ColorRect (HDC hdc, Rect *theRect, SInt32 color)
{
	COLORREF	theRGBColor, wasColor;
	RECT		rc;

	theRGBColor = Index2ColorRef(color);
	wasColor = SetDCBrushColor(hdc, theRGBColor);
	SetRect(&rc, theRect->left, theRect->top, theRect->right, theRect->bottom);
	FillRect(hdc, &rc, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hdc, wasColor);
}

//--------------------------------------------------------------  ColorOval

// Given a rectangle and color index, this function draws a solid…
// oval in that color.  Current port, pen mode, etc. assumed.

void ColorOval (HDC hdc, Rect *theRect, SInt32 color)
{
	COLORREF	theRGBColor, wasColor;
	HRGN		theRegion;

	theRGBColor = Index2ColorRef(color);
	wasColor = SetDCBrushColor(hdc, theRGBColor);
	theRegion = CreateEllipticRgn(theRect->left, theRect->top,
			theRect->right, theRect->bottom);
	FillRgn(hdc, theRegion, GetStockObject(DC_BRUSH));
	DeleteObject(theRegion);
	SetDCBrushColor(hdc, wasColor);
}

//--------------------------------------------------------------  ColorRegion

// Given a region and color index, this function draws a solid…
// region in that color.  Current port, pen mode, etc. assumed.

void ColorRegion (HDC hdc, HRGN theRgn, SInt32 color)
{
	COLORREF	theRGBColor, wasColor;

	theRGBColor = Index2ColorRef(color);
	wasColor = SetDCBrushColor(hdc, theRGBColor);
	FillRgn(hdc, theRgn, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hdc, wasColor);
}

//--------------------------------------------------------------  ColorLine

// Given a the end points for a line and color index, this function…
// draws a line in that color.  Current port, pen mode, etc. assumed.

void ColorLine (HDC hdc, SInt16 h0, SInt16 v0, SInt16 h1, SInt16 v1, SInt32 color)
{
	COLORREF	theRGBColor, wasColor;
	HGDIOBJ		wasPen;

	theRGBColor = Index2ColorRef(color);
	wasColor = SetDCPenColor(hdc, theRGBColor);
	wasPen = SelectObject(hdc, GetStockObject(DC_PEN));
	// GDI draws lines by including the start point and excluding the
	// end point. QuickDraw includes the end point when it draws lines.
	// To emulate QuickDraw's behavior in GDI, the line is drawn twice
	// in opposite directions, so that the start and end points are
	// both included.
	MoveToEx(hdc, h1, v1, NULL);
	LineTo(hdc, h0, v0);
	LineTo(hdc, h1, v1);
	SelectObject(hdc, wasPen);
	SetDCPenColor(hdc, wasColor);
}

//--------------------------------------------------------------  HiliteRect

// Given a rect and two hilite colors, this function frames the top and…
// left edges of the rect with color 1 and frames the bottom and right…
// sides with color 2.  A rect can be made to appear "hi-lit" or "3D"…
// in this way.

void HiliteRect (Rect *theRect, SInt16 color1, SInt16 color2)
{
	return;
#if 0
	ColorLine(theRect->left, theRect->top, theRect->right - 2,
			theRect->top, color1);
	ColorLine(theRect->left, theRect->top, theRect->left,
			theRect->bottom - 2, color1);
	ColorLine(theRect->right - 1, theRect->top, theRect->right - 1,
			theRect->bottom - 2, color2);
	ColorLine(theRect->left + 1, theRect->bottom - 1, theRect->right - 1,
			theRect->bottom - 1, color2);
#endif
}

//--------------------------------------------------------------  ColorFrameRect

// Given a rectangle and color index, this function frames a…
// rectangle in that color.  Current port, pen mode, etc. assumed.

void ColorFrameRect (Rect *theRect, SInt32 color)
{
	return;
#if 0
	RGBColor	theRGBColor, wasColor;

	GetForeColor(&wasColor);
	Index2Color(color, &theRGBColor);
	RGBForeColor(&theRGBColor);
	FrameRect(theRect);
	RGBForeColor(&wasColor);
#endif
}

//--------------------------------------------------------------  ColorFrameWHRect

// Given a the top-left corner of a rectangle, its width and height,…
// and a color index, this function frames a rectangle in that color.
// Current port, pen mode, etc. assumed.

void ColorFrameWHRect (SInt16 left, SInt16 top, SInt16 wide, SInt16 high, SInt32 color)
{
	return;
#if 0
	Rect		theRect;

	theRect.left = left;
	theRect.top = top;
	theRect.right = left + wide;
	theRect.bottom = top + high;
	ColorFrameRect(&theRect, color);
#endif
}

//--------------------------------------------------------------  ColorFrameOval

// Given a rectangle and color index, this function frames an…
// oval in that color.  Current port, pen mode, etc. assumed.

void ColorFrameOval (Rect *theRect, SInt32 color)
{
	return;
#if 0
	RGBColor	theRGBColor, wasColor;

	GetForeColor(&wasColor);
	Index2Color(color, &theRGBColor);
	RGBForeColor(&theRGBColor);
	FrameOval(theRect);
	RGBForeColor(&wasColor);
#endif
}

//--------------------------------------------------------------  LtGrayForeColor

// This function finds the closest match to a "light gray" in the…
// current palette and sets the pen color to that.

void LtGrayForeColor (void)
{
	return;
#if 0
	RGBColor	color;

	color.red = (unsigned short) 0xBFFF;
	color.green = (unsigned short) 0xBFFF;
	color.blue = (unsigned short) 0xBFFF;

	RGBForeColor(&color);
#endif
}

//--------------------------------------------------------------  GrayForeColor

// This function finds the closest match to a "medium gray" in the…
// current palette and sets the pen color to that.

void GrayForeColor (void)
{
	return;
#if 0
	RGBColor	color;

	color.red = (unsigned short) 0x7FFF;
	color.green = (unsigned short) 0x7FFF;
	color.blue = (unsigned short) 0x7FFF;

	RGBForeColor(&color);
#endif
}

//--------------------------------------------------------------  DkGrayForeColor

// This function finds the closest match to a "dark gray" in the…
// current palette and sets the pen color to that.

void DkGrayForeColor (void)
{
	return;
#if 0
	RGBColor	color;

	color.red = (unsigned short) 0x3FFF;
	color.green = (unsigned short) 0x3FFF;
	color.blue = (unsigned short) 0x3FFF;

	RGBForeColor(&color);
#endif
}

//--------------------------------------------------------------  RestoreColorsSlam

// This function forces the Macintosh to rebuild the palette.  It is…
// called to restore a sense or normality after some serious munging…
// with the palette.

void RestoreColorsSlam (void)
{
	return;
#if 0
	RestoreDeviceClut(nil);
	PaintBehind(nil, GetGrayRgn());
	DrawMenuBar();
#endif
}

