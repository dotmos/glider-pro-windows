#ifndef RENDER_H_
#define RENDER_H_

//============================================================================
//----------------------------------------------------------------------------
//                                  Render.h
//----------------------------------------------------------------------------
//============================================================================

#include "GliderStructs.h"
#include "MacTypes.h"
#include "WinAPI.h"

extern HRGN mirrorRgn;
extern SInt16 numWork2Main;
extern SInt16 numBack2Work;
extern Boolean hasMirror;

void AddRectToWorkRects (Rect *theRect);
void AddRectToBackRects (Rect *theRect);
void AddRectToWorkRectsWhole (Rect *theRect);
void RenderGlider (gliderPtr thisGlider, Boolean oneOrTwo);
void CopyRectsQD (void);
void RenderFrame (void);
void InitGarbageRects (void);
void CopyRectBackToWork (Rect *theRect);
void CopyRectWorkToBack (Rect *theRect);
void CopyRectWorkToMain (Rect *theRect);
void CopyRectMainToWork (Rect *theRect);
void CopyRectMainToBack (Rect *theRect);
void AddToMirrorRegion (Rect *theRect);
void ZeroMirrorRegion (void);

#endif
