
//============================================================================
//----------------------------------------------------------------------------
//									Sound.c
//----------------------------------------------------------------------------
//============================================================================


//#include <Resources.h>
//#include <Sound.h>
#include "Macintosh.h"
#include "Audio.h"
#include "DialogUtils.h"
#include "Externs.h"
#include "ResourceIDs.h"


#define kBaseBufferSoundID			1000
#define kMaxSounds					64
#define kNumSoundChannels			3
#define kNoSoundPlaying				-1


typedef struct StaticSound {
	LPDIRECTSOUNDBUFFER8 dsBuffer;
	WaveData wave;
} StaticSound;


void Gp_PlaySound(SInt16, SInt16, SInt16);
void UpdateSoundState (SInt16);
void FlushCurrentSound (SInt16);
OSErr LoadBufferSounds (void);
void DumpBufferSounds (void);
LPDIRECTSOUNDBUFFER8 LoadStaticBuffer (SInt16);
void RestoreStaticBuffer (SInt16, LPDIRECTSOUNDBUFFER8);


StaticSound				theSoundData[kMaxSounds];
Boolean					dontLoadSounds, isSoundOn, failedSound;
LPDIRECTSOUNDBUFFER8	channels[kNumSoundChannels];
SInt16					priorities[kNumSoundChannels];
SInt16					soundsPlaying[kNumSoundChannels];


extern HMODULE		houseResFork;


//==============================================================  Functions
//--------------------------------------------------------------  PlayPrioritySound

void PlayPrioritySound (SInt16 which, SInt16 priority)
{
	SInt16		lowestPriority, whosLowest, i;

	if (failedSound || dontLoadSounds)
		return;

	for (i = 0; i < kNumSoundChannels; i++)
	{
		UpdateSoundState(i);
	}

	if ((priority == kTriggerPriority) &&
			((priorities[0] == kTriggerPriority) ||
			((priorities[1] == kTriggerPriority)) ||
			((priorities[2] == kTriggerPriority))))
		return;

	whosLowest = 0;
	lowestPriority = priorities[0];

	if (priorities[1] < lowestPriority)
	{
		lowestPriority = priorities[1];
		whosLowest = 1;
	}

	if (priorities[2] < lowestPriority)
	{
		lowestPriority = priorities[2];
		whosLowest = 2;
	}

	if (priority >= lowestPriority)
	{
		Gp_PlaySound(whosLowest, which, priority);
	}
}

//--------------------------------------------------------------  FlushAnyTriggerPlaying

void FlushAnyTriggerPlaying (void)
{
	SInt16 channelID;

	for (channelID = 0; channelID < kNumSoundChannels; channelID++)
	{
		if (priorities[channelID] == kTriggerPriority)
		{
			FlushCurrentSound(channelID);
		}
	}
}

//--------------------------------------------------------------  PlaySound

void Gp_PlaySound (SInt16 channelID, SInt16 soundID, SInt16 priority)
{
	HRESULT hr;

	if (failedSound || dontLoadSounds)
		return;

	if (isSoundOn)
	{
		FlushCurrentSound(channelID);

		if (theSoundData[soundID].dsBuffer == NULL)
		{
			theSoundData[soundID].dsBuffer = LoadStaticBuffer(soundID);
			if (theSoundData[soundID].dsBuffer == NULL)
			{
				return;
			}
		}

		hr = Audio_DuplicateSoundBuffer(theSoundData[soundID].dsBuffer, &channels[channelID]);
		if (SUCCEEDED(hr))
		{
			IDirectSoundBuffer8_SetCurrentPosition(channels[channelID], 0);
			hr = IDirectSoundBuffer8_Play(channels[channelID], 0, 0, 0);
			if (FAILED(hr) && hr == DSERR_BUFFERLOST)
			{
				RestoreStaticBuffer(soundID, channels[channelID]);
				hr = IDirectSoundBuffer8_Play(channels[channelID], 0, 0, 0);
			}
			if (SUCCEEDED(hr))
			{
				priorities[channelID] = priority;
				soundsPlaying[channelID] = soundID;
			}
			else
			{
				IDirectSoundBuffer8_Release(channels[channelID]);
				channels[channelID] = NULL;
			}
		}
	}
}

//--------------------------------------------------------------  UpdateSoundState

void UpdateSoundState (SInt16 channelID)
{
	DWORD bufferStatus;
	HRESULT hr;

	if (channels[channelID] != NULL)
	{
		hr = IDirectSoundBuffer8_GetStatus(channels[channelID], &bufferStatus);
		if (FAILED(hr) || (bufferStatus & DSBSTATUS_PLAYING) == 0)
		{
			FlushCurrentSound(channelID);
		}
	}
}

//--------------------------------------------------------------  FlushCurrentSound

void FlushCurrentSound (SInt16 channelID)
{
	if (channels[channelID] != NULL)
	{
		IDirectSoundBuffer8_Stop(channels[channelID]);
		IDirectSoundBuffer8_Release(channels[channelID]);
	}
	channels[channelID] = NULL;
	priorities[channelID] = 0;
	soundsPlaying[channelID] = kNoSoundPlaying;
}

//--------------------------------------------------------------  LoadTriggerSound

OSErr LoadTriggerSound (SInt16 soundID)
{
	if ((houseResFork == NULL) || (dontLoadSounds) ||
			(theSoundData[kMaxSounds - 1].wave.dataBytes != NULL))
	{
		return -1;
	}

//	FlushAnyTriggerPlaying();

	if (!ReadWAVFromResource(houseResFork, (WORD)soundID,
			&theSoundData[kMaxSounds - 1].wave))
	{
		return -1;
	}
	theSoundData[kMaxSounds - 1].dsBuffer = LoadStaticBuffer(kMaxSounds - 1);
	if (theSoundData[kMaxSounds - 1].dsBuffer == NULL)
	{
		return -1;
	}

	return noErr;
}

//--------------------------------------------------------------  DumpTriggerSound

void DumpTriggerSound (void)
{
	if (theSoundData[kMaxSounds - 1].dsBuffer != NULL)
	{
		Audio_ReleaseSoundBuffer(theSoundData[kMaxSounds - 1].dsBuffer);
	}
	theSoundData[kMaxSounds - 1].dsBuffer = NULL;
	theSoundData[kMaxSounds - 1].wave.dataLength = 0;
	theSoundData[kMaxSounds - 1].wave.dataBytes = NULL;
}

//--------------------------------------------------------------  LoadBufferSounds

OSErr LoadBufferSounds (void)
{
	WORD i;

	for (i = 0; i < kMaxSounds - 1; i++)
	{
		if (!ReadWAVFromResource(HINST_THISCOMPONENT,
				i + kBaseBufferSoundID, &theSoundData[i].wave))
		{
			return -1;
		}
		theSoundData[i].dsBuffer = NULL;
	}

	theSoundData[kMaxSounds - 1].wave.dataLength = 0;
	theSoundData[kMaxSounds - 1].wave.dataBytes = NULL;

	return noErr;
}

//--------------------------------------------------------------  DumpBufferSounds

void DumpBufferSounds (void)
{
	WORD i;

	for (i = 0; i < kNumSoundChannels; i++)
	{
		FlushCurrentSound(i);
	}

	for (i = 0; i < kMaxSounds - 1; i++)
	{
		if (theSoundData[i].dsBuffer != NULL)
		{
			Audio_ReleaseSoundBuffer(theSoundData[i].dsBuffer);
			theSoundData[i].dsBuffer = NULL;
		}
	}
}

//--------------------------------------------------------------  InitSound

void InitSound (void)
{
	OSErr		theErr;
	SInt16		i;

	if (dontLoadSounds)
		return;

	failedSound = false;

	for (i = 0; i < kNumSoundChannels; i++)
	{
		channels[i] = NULL;
		priorities[i] = 0;
		soundsPlaying[i] = kNoSoundPlaying;
	}

	theErr = LoadBufferSounds();
	if (theErr != noErr)
	{
		YellowAlert(kYellowFailedSound, theErr);
		failedSound = true;
	}
}

//--------------------------------------------------------------  KillSound

void KillSound (void)
{
	if (dontLoadSounds)
		return;

	DumpBufferSounds();
}

//--------------------------------------------------------------  SoundBytesNeeded

SInt32 SoundBytesNeeded (void)
{
	HRSRC		hRsrc;
	DWORD		totalBytes;
	WORD		i;

	totalBytes = 0L;
	for (i = 0; i < kMaxSounds - 1; i++)
	{
		hRsrc = FindResource(HINST_THISCOMPONENT,
				MAKEINTRESOURCE(i + kBaseBufferSoundID), L"WAVE");
		if (hRsrc == NULL)
			return -1;
		totalBytes += SizeofResource(HINST_THISCOMPONENT, hRsrc);
	}
	return (SInt32)totalBytes;
}

//--------------------------------------------------------------  TellHerNoSounds

void TellHerNoSounds (void)
{
	SInt16			hitWhat;
	DialogParams	params = { 0 };

	params.hwndParent = mainWindow;
	hitWhat = Alert(kNoMemForSoundsAlert, &params);
}

//--------------------------------------------------------------  BitchAboutSM3

void BitchAboutSM3 (void)
{
	SInt16			hitWhat;
	DialogParams	params = { 0 };

	params.hwndParent = mainWindow;
	hitWhat = Alert(kNoSoundManager3Alert, &params);
}

//--------------------------------------------------------------  LoadStaticBuffer

LPDIRECTSOUNDBUFFER8 LoadStaticBuffer (SInt16 which)
{
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	LPDIRECTSOUNDBUFFER8 soundBuffer;
	LPVOID audioPtr;
	DWORD audioSize;
	HRESULT hr;

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = theSoundData[which].wave.channels;
	waveFormat.nSamplesPerSec = theSoundData[which].wave.samplesPerSec;
	waveFormat.wBitsPerSample = theSoundData[which].wave.bitsPerSample;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.dwSize = sizeof(bufferDesc);
	bufferDesc.dwFlags = DSBCAPS_LOCSOFTWARE
		| DSBCAPS_CTRLVOLUME
		| DSBCAPS_GLOBALFOCUS
		| DSBCAPS_GETCURRENTPOSITION2;
	bufferDesc.dwBufferBytes = (DWORD)theSoundData[which].wave.dataLength;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	hr = Audio_CreateSoundBuffer(&bufferDesc, &soundBuffer, NULL);
	if (FAILED(hr))
	{
		return NULL;
	}

	hr = IDirectSoundBuffer8_Lock(soundBuffer, 0, 0,
			&audioPtr, &audioSize, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (SUCCEEDED(hr))
	{
		memcpy(audioPtr, theSoundData[which].wave.dataBytes, audioSize);
		IDirectSoundBuffer8_Unlock(soundBuffer, audioPtr, audioSize, NULL, 0);
	}
	else
	{
		Audio_ReleaseSoundBuffer(soundBuffer);
		return NULL;
	}

	return soundBuffer;
}

//--------------------------------------------------------------  RestoreStaticBuffer

void RestoreStaticBuffer (SInt16 which, LPDIRECTSOUNDBUFFER8 soundBuffer)
{
	LPVOID audioPtr;
	DWORD audioSize;
	HRESULT hr;

	if (soundBuffer == NULL)
	{
		return;
	}
	hr = IDirectSoundBuffer8_Restore(soundBuffer);
	if (SUCCEEDED(hr))
	{
		IDirectSoundBuffer8_Stop(soundBuffer);
		hr = IDirectSoundBuffer8_Lock(soundBuffer, 0, 0,
			&audioPtr, &audioSize, NULL, NULL, DSBLOCK_ENTIREBUFFER);
		if (SUCCEEDED(hr))
		{
			memcpy(audioPtr, theSoundData[which].wave.dataBytes, audioSize);
			IDirectSoundBuffer8_Unlock(soundBuffer, audioPtr, audioSize, NULL, 0);
		}
	}
}

