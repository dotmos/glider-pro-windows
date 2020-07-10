#ifndef ROOM_INFO_H_
#define ROOM_INFO_H_

//============================================================================
//----------------------------------------------------------------------------
//                                 RoomInfo.h
//----------------------------------------------------------------------------
//============================================================================

#include "MacTypes.h"
#include "WinAPI.h"

extern Rect tileSrcRect;
extern HDC tileSrcMap;

void DoRoomInfo (HWND hwndOwner);
Boolean PictIDExists (SInt16 theID);

#endif
