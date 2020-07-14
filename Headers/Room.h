#ifndef ROOM_H_
#define ROOM_H_

//============================================================================
//----------------------------------------------------------------------------
//                                   Room.h
//----------------------------------------------------------------------------
//============================================================================

#include "GliderStructs.h"
#include "MacTypes.h"
#include "WinAPI.h"

extern roomPtr thisRoom;
extern Rect backSrcRect;
extern HDC backSrcMap;
extern SInt16 numberRooms;
extern SInt16 thisRoomNumber;
extern SInt16 previousRoom;
extern SInt16 leftThresh;
extern SInt16 rightThresh;
extern SInt16 lastBackground;
extern Boolean autoRoomEdit;
extern Boolean newRoomNow;
extern Boolean noRoomAtAll;
extern Boolean leftOpen;
extern Boolean rightOpen;
extern Boolean topOpen;
extern Boolean bottomOpen;
extern Boolean doBitchDialogs;

void SetInitialTiles (SInt16 background, SInt16 *theTiles);
Boolean CreateNewRoom (HWND ownerWindow, SInt16 h, SInt16 v);
void ReadyBackground (SInt16 theID, const SInt16 *theTiles);
void ReflectCurrentRoom (Boolean forceMapRedraw);
void CopyRoomToThisRoom (SInt16 roomNumber);
void CopyThisRoomToRoom (void);
void ForceThisRoom (SInt16 roomNumber);
Boolean RoomExists (SInt16 suite, SInt16 floor, SInt16 *roomNum);
Boolean RoomNumExists (SInt16 roomNum);
void DeleteRoom (HWND ownerWindow, Boolean doWarn);
SInt16 DoesNeighborRoomExist (SInt16 whichNeighbor);
void SelectNeighborRoom (SInt16 whichNeighbor);
SInt16 GetNeighborRoomNumber (SInt16 which);
Boolean GetRoomFloorSuite (SInt16 room, SInt16 *floor, SInt16 *suite);
SInt16 GetRoomNumber (SInt16 floor, SInt16 suite);
Boolean IsRoomAStructure (SInt16 roomNum);
void DetermineRoomOpenings (void);
SInt16 GetOriginalBounding (SInt16 theID);
SInt16 GetNumberOfLights (SInt16 where);
Boolean IsShadowVisible (void);
Boolean DoesRoomHaveFloor (void);
Boolean DoesRoomHaveCeiling (void);

#endif
