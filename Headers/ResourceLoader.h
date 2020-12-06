#ifndef RESOURCE_LOADER_H_
#define RESOURCE_LOADER_H_

#ifdef GP_USE_WINAPI_H

#include "Audio.h"
#include "GliderStructs.h"
#include "WinAPI.h"

typedef struct Gp_ResourceFile Gp_ResourceFile;

// Load in the game's built-in assets from the Mermaid.dat file.
// If the assets have already been loaded, then the function succeeds
// and nothing happens.
HRESULT Gp_LoadBuiltInAssets (void);

// Unload the game's built-in assets. If the assets aren't loaded, then
// nothing happens.
void Gp_UnloadBuiltInAssets (void);

// Return whether the built-in assets are loaded up.
BOOLEAN Gp_BuiltInAssetsLoaded (void);

// Create a new house file with the given file name. If there
// is already a file with the same file name, this function will
// overwrite that file's data. This function will not affect the
// currently loaded house; it only creates a new file.
HRESULT Gp_CreateHouseFile (PCWSTR fileName);

// Load the house file with the given file name. If there is already a
// house file loaded, it is unloaded before attempting to load the new
// house file.
HRESULT Gp_LoadHouseFile (PCWSTR fileName);

// Unload any loaded house file.
void Gp_UnloadHouseFile (void);

// Return whether a house file is loaded up.
BOOLEAN Gp_HouseFileLoaded (void);

// Return whether a loaded house file is read-only.
BOOLEAN Gp_HouseFileReadOnly (void);

// Create an HICON for the house, if the house contains an icon. If the
// width parameter is zero, the SM_CXICON system metric is used instead.
// If the height parameter is zero, the SM_CYICON system metric is used
// instead. If the house does not contain an icon of its own, NULL is
// returned. In this case, a default icon should be used.
//
// The returned HICON should be cleaned up by calling DestroyIcon().
HICON Gp_LoadHouseIcon (UINT width, UINT height);

// Return the uncompressed size of the house data, in bytes.
uint64_t Gp_HouseFileDataSize (void);

// Read in the house's data from the loaded house file.
// This function will fail if house data hasn't been written to the
// house file yet.
//
// The field house->rooms should be cleaned up by calling free().
HRESULT Gp_ReadHouseData (houseType *house);

// Write out the house's data to the loaded house file.
// NOTE: this function temporarily unloads the house file, and then
// reloads it. If, somehow, the house file goes missing, there will
// be no house file loaded. Therefore, if the function fails, you
// should check whether the house file is still loaded.
HRESULT Gp_WriteHouseData (const houseType *house);

// Load an image. The loader attempts to load from the house file first.
// If that fails, then the loader attempts to load from the built-in
// assets. If that also fails, then the function fails and returns NULL.
//
// The returned HBITMAP should be cleaned up by calling DeleteObject().
HBITMAP Gp_LoadImage (SInt16 imageID);

// Load an image as a DIB section. The only difference between this
// function and Gp_LoadImage() is that the returned HBITMAP is a handle
// to a DIB section bitmap.
//
// The returned HBITMAP should be cleaned up by calling DeleteObject().
HBITMAP Gp_LoadImageAsDIB (SInt16 imageID);

// Load a sound. The loader attempts to load from the house file first.
// If that fails, then the loader attempts to load from the built-in
// assets. If that also fails, then the function fails.
//
// The sound->dataBytes field should be cleaned up by calling free().
HRESULT Gp_LoadSound (SInt16 soundID, WaveData *sound);

// Typedef for resource enumerator procedure. 'resID' is the ID of the
// image or sound, and 'userData' is whatever was passed in to the
// enumerator function. Return nonzero to continue enumerating, or zero
// to stop.
typedef BOOLEAN (*Gp_EnumResProc)(SInt16 resID, void *userData);

// Load an image from the built-in assets. If the image does not
// exist in the built-in assets or the built-in assets have not been
// loaded yet, the function will return NULL.
//
// The returned HBITMAP should be cleaned up by calling DeleteObject().
HBITMAP Gp_LoadBuiltInImage (SInt16 imageID);

// Load an image from the built-in assets as a DIB section. The only
// difference between this function and Gp_LoadBuiltInImage() is that
// the returned HBITMAP is a DIB section bitmap.
//
// The returned HBITMAP should be cleaned up by calling DeleteObject().
HBITMAP Gp_LoadBuiltInImageAsDIB (SInt16 imageID);

// Load a sound from the built-in assets. If the sound does not
// exist in the built-in assets or the built-in assets have not been
// loaded yet, the function will fail.
//
// The field sound->dataBytes should be cleaned up by calling free().
HRESULT Gp_LoadBuiltInSound (SInt16 soundID, WaveData *sound);

// Return the first image ID in the given range, or the default value
// in the 'def' parameter if no image is in the given range. The
// 'first' and 'last' parameters are both inclusive.
SInt16 Gp_GetFirstHouseImageID (SInt16 first, SInt16 last, SInt16 def);

// Return whether the house file contains an image with the given ID.
BOOLEAN Gp_HouseImageExists (SInt16 imageID);

// Load an image from the house file. If the image does not exist in the
// house file or there is no house file loaded, the function returns NULL.
//
// The returned HBITMAP should be cleaned up by calling DeleteObject().
HBITMAP Gp_LoadHouseImage (SInt16 imageID);

// Load an image from the house file as a DIB section. The only
// difference between this function and Gp_LoadHouseImage is that the
// returned HBITMAP is a DIB section bitmap.
//
// The returned HBITMAP should be cleaned up by calling DeleteObject().
HBITMAP Gp_LoadHouseImageAsDIB (SInt16 imageID);

// Load a sound from the house file. If the sound does not exist in
// the built-in assets or the built-in assets have not been loaded yet,
// the function will fail.
//
// The field sound->dataBytes should be cleaned up by calling free().
HRESULT Gp_LoadHouseSound (SInt16 soundID, WaveData *sound);

// Load boundary information from the house file for the given image ID.
HRESULT Gp_LoadHouseBounding (SInt16 imageID, boundsType *bounds);

#endif

#endif
