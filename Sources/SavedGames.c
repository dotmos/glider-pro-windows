#include "SavedGames.h"

//============================================================================
//----------------------------------------------------------------------------
//								   SavedGames.c
//----------------------------------------------------------------------------
//============================================================================


#include "ByteIO.h"
#include "DialogUtils.h"
#include "Externs.h"
#include "House.h"
#include "Macintosh.h"
#include "ResourceIDs.h"
#include "StructIO.h"


#define kSavedGameVersion		0x0200


BOOL GetSaveFolderPath (LPWSTR, DWORD);
void SavedGameMismatchError (HWND, StringPtr);


gameType	smallGame;

extern	houseSpecPtr	theHousesSpecs;
extern	SInt16			numStarsRemaining, thisHouseIndex;
extern	Boolean			twoPlayerGame;


//==============================================================  Functions
//--------------------------------------------------------------  GetSaveFolderPath

BOOL GetSaveFolderPath (LPWSTR lpSavePath, DWORD cchSavePath)
{
	WCHAR pathBuffer[MAX_PATH];
	HRESULT hr;

	if (!GetDataFolderPath(pathBuffer, ARRAYSIZE(pathBuffer)))
		return FALSE;
	hr = StringCchCat(pathBuffer, ARRAYSIZE(pathBuffer), L"\\Saves");
	if (FAILED(hr))
		return FALSE;
	if (!CreateDirectory(pathBuffer, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		return FALSE;

	hr = StringCchCopy(lpSavePath, cchSavePath, pathBuffer);
	return SUCCEEDED(hr);
}

//--------------------------------------------------------------  SaveGame2

void SaveGame2 (HWND ownerWindow)
{
	OPENFILENAME		ofn = { 0 };
	Str255				gameNameStr;
	Str15				errorLabel;
	WCHAR				startPath[MAX_PATH];
	WCHAR				gamePath[MAX_PATH];
	roomType			*srcRoom;
	savedRoom			*destRoom;
	game2Type			savedGame;
	SInt16				r, i, numRooms;
	HANDLE				gameFileHandle;
	byteio				byteWriter;

	PasStringCopyC("Saved Game", errorLabel);

	if (!GetSaveFolderPath(startPath, ARRAYSIZE(startPath)))
		startPath[0] = L'\0';

	numRooms = thisHouse->nRooms;

	savedGame.savedData = calloc(numRooms, sizeof(savedRoom));
	if (savedGame.savedData == NULL)
	{
		YellowAlert(ownerWindow, kYellowFailedSaveGame, -1);
		return;
	}

	GetFirstWordOfString(thisHouseName, gameNameStr);
	if (gameNameStr[0] > 23)
		gameNameStr[0] = 23;
	PasStringConcatC(gameNameStr, " Game");

	WinFromMacString(gamePath, ARRAYSIZE(gamePath), gameNameStr);
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = ownerWindow;
	ofn.lpstrFilter = L"Glider PRO Saved Game (*.glg)\0*.glg\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = gamePath;
	ofn.nMaxFile = ARRAYSIZE(gamePath);
	if (startPath[0] != L'\0')
		ofn.lpstrInitialDir = startPath;
	ofn.lpstrTitle = L"Save Game As:";
	ofn.Flags = OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = L"glg";
	if (!GetSaveFileName(&ofn))
	{
		free(savedGame.savedData);
		return;
	}

	savedGame.house.vRefNum = 0;
	savedGame.house.parID = 0;
	PasStringCopy(theHousesSpecs[thisHouseIndex].name, savedGame.house.name);
	savedGame.version = kSavedGameVersion;
	savedGame.wasStarsLeft = numStarsRemaining;
	savedGame.timeStamp = thisHouse->timeStamp;
	savedGame.where.h = theGlider.dest.left;
	savedGame.where.v = theGlider.dest.top;
	savedGame.score = theScore;
	savedGame.unusedLong = 0L;
	savedGame.unusedLong2 = 0L;
	savedGame.energy = batteryTotal;
	savedGame.bands = bandsTotal;
	savedGame.roomNumber = thisRoomNumber;
	savedGame.gliderState = theGlider.mode;
	savedGame.numGliders = mortals;
	savedGame.foil = foilTotal;
	savedGame.nRooms = numRooms;
	savedGame.facing = theGlider.facing;
	savedGame.showFoil = showFoil;

	for (r = 0; r < numRooms; r++)
	{
		destRoom = &(savedGame.savedData[r]);
		srcRoom = &(thisHouse->rooms[r]);

		destRoom->unusedShort = 0;
		destRoom->unusedByte = 0;
		destRoom->visited = srcRoom->visited;
		for (i = 0; i < kMaxRoomObs; i++)
			destRoom->objects[i] = srcRoom->objects[i];
	}

	gameFileHandle = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (gameFileHandle != INVALID_HANDLE_VALUE)
	{
		if (byteio_init_handle_writer(&byteWriter, gameFileHandle))
		{
			if (!WriteGame2Type(&byteWriter, &savedGame))
				CheckFileError(ownerWindow, GetLastError(), errorLabel);
			if (!byteio_close(&byteWriter))
				CheckFileError(ownerWindow, GetLastError(), errorLabel);
		}
		CloseHandle(gameFileHandle);
	}
	else
	{
		CheckFileError(ownerWindow, GetLastError(), errorLabel);
	}
	free(savedGame.savedData);
}

//--------------------------------------------------------------  SavedGameMismatchError

void SavedGameMismatchError (HWND ownerWindow, StringPtr gameName)
{
	DialogParams params = { 0 };
	wchar_t gameStr[256];
	wchar_t houseStr[64];

	WinFromMacString(gameStr, ARRAYSIZE(gameStr), gameName);
	WinFromMacString(houseStr, ARRAYSIZE(houseStr), thisHouseName);

	params.arg[0] = gameStr;
	params.arg[1] = houseStr;
	Alert(kSavedGameErrorAlert, ownerWindow, &params);
}

//--------------------------------------------------------------  OpenSavedGame

Boolean OpenSavedGame (HWND ownerWindow)
{
	OPENFILENAME		ofn = { 0 };
	Str15				errorLabel;
	WCHAR				startPath[MAX_PATH];
	WCHAR				gamePath[MAX_PATH];
	roomType			*destRoom;
	savedRoom			*srcRoom;
	game2Type			savedGame;
	SInt16				r, i;
	HANDLE				gameFileHandle;
	byteio				byteReader;
	int					result;
	DWORD				lastError;

	PasStringCopyC("Saved Game", errorLabel);

	if (!GetSaveFolderPath(startPath, ARRAYSIZE(startPath)))
		startPath[0] = L'\0';
	gamePath[0] = L'\0';

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = ownerWindow;
	ofn.lpstrFilter = L"Glider PRO Saved Game (*.glg)\0*.glg\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = gamePath;
	ofn.nMaxFile = ARRAYSIZE(gamePath);
	if (startPath[0] != L'\0')
		ofn.lpstrInitialDir = startPath;
	ofn.lpstrTitle = L"Open Saved Game:";
	ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = L"glg";
	if (!GetOpenFileName(&ofn))
		return false;

	gameFileHandle = CreateFile(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (gameFileHandle == INVALID_HANDLE_VALUE)
	{
		CheckFileError(ownerWindow, GetLastError(), errorLabel);
		return false;
	}
	if (!byteio_init_handle_reader(&byteReader, gameFileHandle))
		RedAlert(kErrNoMemory);
	savedGame.savedData = NULL;
	result = ReadGame2Type(&byteReader, &savedGame);
	lastError = GetLastError();
	byteio_close(&byteReader);
	CloseHandle(gameFileHandle);
	if (result == 0)
	{
		CheckFileError(ownerWindow, lastError, errorLabel);
		free(savedGame.savedData);
		return false;
	}

	if (!Mac_EqualString(savedGame.house.name, thisHouseName, true))
	{
		SavedGameMismatchError(ownerWindow, savedGame.house.name);
		free(savedGame.savedData);
		return false;
	}
	else if (thisHouse->timeStamp != savedGame.timeStamp)
	{
		YellowAlert(ownerWindow, kYellowSavedTimeWrong, 0);
		free(savedGame.savedData);
		return false;
	}
	else if (savedGame.version != kSavedGameVersion)
	{
		YellowAlert(ownerWindow, kYellowSavedVersWrong, kSavedGameVersion);
		free(savedGame.savedData);
		return false;
	}
	else if (savedGame.nRooms != thisHouse->nRooms)
	{
		YellowAlert(ownerWindow, kYellowSavedRoomsWrong,
				savedGame.nRooms - thisHouse->nRooms);
		free(savedGame.savedData);
		return false;
	}
	else
	{
		smallGame.wasStarsLeft = savedGame.wasStarsLeft;
		smallGame.where.h = savedGame.where.h;
		smallGame.where.v = savedGame.where.v;
		smallGame.score = savedGame.score;
		smallGame.unusedLong = savedGame.unusedLong;
		smallGame.unusedLong2 = savedGame.unusedLong2;
		smallGame.energy = savedGame.energy;
		smallGame.bands = savedGame.bands;
		smallGame.roomNumber = savedGame.roomNumber;
		smallGame.gliderState = savedGame.gliderState;
		smallGame.numGliders = savedGame.numGliders;
		smallGame.foil = savedGame.foil;
		smallGame.unusedShort = 0;
		smallGame.facing = savedGame.facing;
		smallGame.showFoil = savedGame.showFoil;

		for (r = 0; r < savedGame.nRooms; r++)
		{
			srcRoom = &(savedGame.savedData[r]);
			destRoom = &(thisHouse->rooms[r]);
			destRoom->visited = srcRoom->visited;
			for (i = 0; i < kMaxRoomObs; i++)
				destRoom->objects[i] = srcRoom->objects[i];
		}
	}

	free(savedGame.savedData);

	return true;
}

//--------------------------------------------------------------  SaveGame

// This is probably about 3 days away from becoming the "old" function…
// for saving games.

void SaveGame (HWND ownerWindow, Boolean doSave)
{
	UInt32			stamp;

	if (twoPlayerGame)
		return;

	if (doSave)
	{
		thisHouse->savedGame.version = kSavedGameVersion;
		thisHouse->savedGame.wasStarsLeft = numStarsRemaining;
		Mac_GetDateTime(&stamp);
		thisHouse->savedGame.timeStamp = (SInt32)stamp;
		thisHouse->savedGame.where.h = theGlider.dest.left;
		thisHouse->savedGame.where.v = theGlider.dest.top;
		thisHouse->savedGame.score = theScore;
		thisHouse->savedGame.unusedLong = 0L;
		thisHouse->savedGame.unusedLong2 = 0L;
		thisHouse->savedGame.energy = batteryTotal;
		thisHouse->savedGame.bands = bandsTotal;
		thisHouse->savedGame.roomNumber = thisRoomNumber;
		thisHouse->savedGame.gliderState = theGlider.mode;
		thisHouse->savedGame.numGliders = mortals;
		thisHouse->savedGame.foil = foilTotal;
		thisHouse->savedGame.unusedShort = 0;
		thisHouse->savedGame.facing = theGlider.facing;
		thisHouse->savedGame.showFoil = showFoil;

		thisHouse->hasGame = true;
	}
	else
	{
		thisHouse->hasGame = false;
	}

	if (doSave)
	{
		if (!WriteHouse(ownerWindow, theMode == kEditMode))
			YellowAlert(ownerWindow, kYellowFailedWrite, 0);
	}
}

