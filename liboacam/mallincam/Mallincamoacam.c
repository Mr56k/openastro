/*****************************************************************************
 *
 * Mallincamoacam.c -- main entrypoint for Mallincam cameras
 *
 * Copyright 2016,2017,2018,2019 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <oa_common.h>

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif
#include <openastro/camera.h>
#include <toupcam.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Mallincamoacam.h"

// Pointers to libmallincam functions so we can use them via libdl.

const char*	( *p_Mallincam_Version )();
unsigned	( *p_Mallincam_Enum )( ToupcamInst* );
HToupCam	( *p_Mallincam_Open )( const char* );
HToupCam	( *p_Mallincam_OpenByIndex )( unsigned );
void		( *p_Mallincam_Close )( HToupCam );
HRESULT		( *p_Mallincam_StartPullModeWithCallback )( HToupCam,
		    PTOUPCAM_EVENT_CALLBACK, void* );
HRESULT		( *p_Mallincam_PullImage )( HToupCam, void*, int, unsigned*,
		    unsigned* );
HRESULT		( *p_Mallincam_PullStillImage )( HToupCam, void*, int,
		    unsigned*, unsigned* );
HRESULT		( *p_Mallincam_StartPushMode )( HToupCam,
		    PTOUPCAM_DATA_CALLBACK, void* );
HRESULT		( *p_Mallincam_Stop )( HToupCam );
HRESULT		( *p_Mallincam_Pause )( HToupCam, int );
HRESULT		( *p_Mallincam_Snap )( HToupCam, unsigned );
HRESULT		( *p_Mallincam_Trigger )( HToupCam );
HRESULT		( *p_Mallincam_get_Size )( HToupCam, int*, int* );
HRESULT		( *p_Mallincam_put_Size )( HToupCam, int, int );
HRESULT		( *p_Mallincam_get_eSize )( HToupCam, unsigned* );
HRESULT		( *p_Mallincam_put_eSize )( HToupCam, unsigned );
HRESULT		( *p_Mallincam_get_Resolution )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Mallincam_get_ResolutionNumber )( HToupCam );
HRESULT		( *p_Mallincam_get_ResolutionRatio )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Mallincam_get_RawFormat )( HToupCam, unsigned*,
		    unsigned* );
HRESULT		( *p_Mallincam_get_AutoExpoEnable )( HToupCam, int* );
HRESULT		( *p_Mallincam_get_AutoExpoTarget )( HToupCam,
		    unsigned short* );
HRESULT		( *p_Mallincam_put_AutoExpoEnable )( HToupCam, int );
HRESULT		( *p_Mallincam_put_AutoExpoTarget )( HToupCam, unsigned short );
HRESULT		( *p_Mallincam_get_ExpoTime )( HToupCam, unsigned* );
HRESULT		( *p_Mallincam_get_ExpTimeRange )( HToupCam, unsigned*,
		    unsigned*, unsigned* );
HRESULT		( *p_Mallincam_put_ExpoTime )( HToupCam, unsigned );
HRESULT		( *p_Mallincam_put_MaxAutoExpoTimeAGain )( HToupCam, unsigned,
		    unsigned short );
HRESULT		( *p_Mallincam_get_ExpoAGain )( HToupCam, unsigned short* );
HRESULT		( *p_Mallincam_put_ExpoAGain )( HToupCam, unsigned short );
HRESULT		( *p_Mallincam_get_ExpoAGainRange )( HToupCam, unsigned short*,
		    unsigned short*, unsigned short* );
HRESULT		( *p_Mallincam_AwbInit )( HToupCam,
		    PITOUPCAM_WHITEBALANCE_CALLBACK, void* );
HRESULT		( *p_Mallincam_AwbOnePush )( HToupCam,
		    PITOUPCAM_TEMPTINT_CALLBACK, void* );
HRESULT		( *p_Mallincam_get_TempTint )( HToupCam, int*, int* );
HRESULT		( *p_Mallincam_put_TempTint )( HToupCam, int, int );
HRESULT		( *p_Mallincam_get_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_Mallincam_put_WhiteBalanceGain )( HToupCam, int[3] );
HRESULT		( *p_Mallincam_get_Hue )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Hue )( HToupCam, int );
HRESULT		( *p_Mallincam_get_Saturation )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Saturation )( HToupCam, int );
HRESULT		( *p_Mallincam_get_Brightness )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Brightness )( HToupCam, int );
HRESULT		( *p_Mallincam_get_Contrast )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Contrast )( HToupCam, int );
HRESULT		( *p_Mallincam_get_Gamma )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Gamma )( HToupCam, int );
HRESULT		( *p_Mallincam_get_Chrome )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Chrome )( HToupCam, int );
HRESULT		( *p_Mallincam_get_VFlip )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_VFlip )( HToupCam, int );
HRESULT		( *p_Mallincam_get_HFlip )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_HFlip )( HToupCam, int );
HRESULT		( *p_Mallincam_get_Negative )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Negative )( HToupCam, int );
HRESULT		( *p_Mallincam_get_MaxSpeed )( HToupCam );
HRESULT		( *p_Mallincam_get_Speed )( HToupCam, unsigned short* );
HRESULT		( *p_Mallincam_put_Speed )( HToupCam, unsigned short );
HRESULT		( *p_Mallincam_get_MaxBitDepth )( HToupCam );
HRESULT		( *p_Mallincam_get_HZ )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_HZ )( HToupCam, int );
HRESULT		( *p_Mallincam_get_Mode )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_Mode )( HToupCam, int );
HRESULT		( *p_Mallincam_get_AWBAuxRect )( HToupCam, RECT* );
HRESULT		( *p_Mallincam_put_AWBAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_Mallincam_get_AEAuxRect )( HToupCam, RECT* );
HRESULT		( *p_Mallincam_put_AEAuxRect )( HToupCam, const RECT* );
HRESULT		( *p_Mallincam_get_MonoMode )( HToupCam );
HRESULT		( *p_Mallincam_get_StillResolution )( HToupCam, unsigned, int*,
		    int* );
HRESULT		( *p_Mallincam_get_StillResolutionNumber )( HToupCam );
HRESULT		( *p_Mallincam_get_RealTime )( HToupCam, int* );
HRESULT		( *p_Mallincam_put_RealTime )( HToupCam, int );
HRESULT		( *p_Mallincam_Flush )( HToupCam );
HRESULT		( *p_Mallincam_get_Temperature )( HToupCam, short* );
HRESULT		( *p_Mallincam_put_Temperature )( HToupCam, short );
HRESULT		( *p_Mallincam_get_SerialNumber )( HToupCam, char[32] );
HRESULT		( *p_Mallincam_get_FwVersion )( HToupCam, char[16] );
HRESULT		( *p_Mallincam_get_HwVersion )( HToupCam, char[16] );
HRESULT		( *p_Mallincam_get_ProductionDate )( HToupCam, char[10] );
HRESULT		( *p_Mallincam_get_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Mallincam_put_LevelRange )( HToupCam, unsigned short[4],
		    unsigned short[4] );
HRESULT		( *p_Mallincam_put_ExpoCallback )( HToupCam,
		    PITOUPCAM_EXPOSURE_CALLBACK, void* );
HRESULT		( *p_Mallincam_put_ChromeCallback )( HToupCam,
		    PITOUPCAM_CHROME_CALLBACK, void* );
HRESULT		( *p_Mallincam_LevelRangeAuto )( HToupCam );
HRESULT		( *p_Mallincam_GetHistogram )( HToupCam,
		    PITOUPCAM_HISTOGRAM_CALLBACK, void* );
HRESULT		( *p_Mallincam_put_LEDState )( HToupCam, unsigned short,
		    unsigned short, unsigned short );
HRESULT		( *p_Mallincam_read_EEPROM )( HToupCam, unsigned,
		    unsigned char*, unsigned );
HRESULT		( *p_Mallincam_write_EEPROM )( HToupCam, unsigned,
		    const unsigned char*, unsigned );
HRESULT		( *p_Mallincam_get_Option )( HToupCam, unsigned, unsigned* );
HRESULT		( *p_Mallincam_put_Option )( HToupCam, unsigned, unsigned );
HRESULT		( *p_Mallincam_get_Roi )( HToupCam, unsigned*, unsigned* );
HRESULT		( *p_Mallincam_put_Roi )( HToupCam, unsigned, unsigned,
		    unsigned, unsigned );
HRESULT		( *p_Mallincam_ST4PlusGuide )( HToupCam, unsigned, unsigned );
HRESULT		( *p_Mallincam_ST4PlusGuideState )( HToupCam );
double		( *p_Mallincam_calc_ClarityFactor )( const void*, int,
		    unsigned, unsigned );
void		( *p_Mallincam_deBayer )( unsigned, int, int, const void*,
		    void*, unsigned char );
void		( *p_Mallincam_HotPlug )( PTOUPCAM_HOTPLUG, void* );

// These are apparently obsolete
//
// Toupcam_get_RoiMode
// Toupcamm_put_RoiMode
// Toupcamm_get_VignetAmountInt
// Toupcamm_get_VignetEnable
// Toupcamm_get_VignetMidPointInt
// Toupcamm_put_VignetAmountInt
// Toupcamm_put_VignetEnable
// Toupcamm_put_VignetMidPointInt

// And these are not documented as far as I can see
// Toupcam_AbbOnePush
// Toupcam_EnumV2
// Toupcam_FfcOnePush
// Toupcam_get_ABBAuxRect
// Toupcam_get_BlackBalance
// Toupcam_get_FanMaxSpeed
// Toupcam_get_Field
// Toupcam_get_FpgaVersion
// Toupcam_get_FrameRate
// Toupcam_get_PixelSize
// Toupcam_get_Revision
// Toupcam_InitOcl
// Toupcam_IoControl
// Toupcam_PullImageWithRowPitch
// Toupcam_PullStillImageWithRowPitch
// Toupcam_put_ABBAuxRect
// Toupcam_put_BlackBalance
// Toupcam_put_ColorMatrix
// Toupcam_put_Curve
// Toupcam_put_Demosaic
// Toupcam_put_InitWBGain
// Toupcam_put_Linear
// Toupcam_read_UART
// Toupcam_StartOclWithSharedTexture
// Toupcam_StartPushMode
// Toupcam_write_UART


static void*		_getDLSym ( void*, const char* );

/**
 * Cycle through the list of cameras returned by the mallincam library
 */

int
oaMallincamGetCameras ( CAMERA_LIST* deviceList, int flags )
{
  ToupcamInst		devList[ TOUPCAM_MAX ];
  unsigned int		numCameras;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int                   ret;
  static void*		libHandle = 0;

  // On Linux, the only place we're going to look for this library is
  // in the default installation directory for the Mallincam LITE application
  // which is /usr/local/MALLINCAMLITE.
  // 
  // On MacOS we just try /Applications/MALLINCAMLITE.app/Contents/MacOS

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  const char*           libName = "/Applications/MALLINCAMLITE.app/Contents/"
                            "MacOS/libmallincam.dylib";
#else
  const char*           libName = "/usr/local/MALLINCAMLITE/libmallincam.so";
#endif

  if ( !libHandle ) {
    if (!( libHandle = dlopen ( libName, RTLD_LAZY ))) {
      // fprintf ( stderr, "can't load %s\n", libraryPath );
      return 0;
    }
  }

  dlerror();

  if (!( *( void** )( &p_Mallincam_AwbInit ) = _getDLSym ( libHandle,
      "Toupcam_AwbInit" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_AwbOnePush ) = _getDLSym ( libHandle,
      "Toupcam_AwbOnePush" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_calc_ClarityFactor ) = _getDLSym ( libHandle,
      "Toupcam_calc_ClarityFactor" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Close ) = _getDLSym ( libHandle,
      "Toupcam_Close" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_deBayer ) = _getDLSym ( libHandle,
      "Toupcam_deBayer" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Enum ) = _getDLSym ( libHandle,
      "Toupcam_Enum" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Flush ) = _getDLSym ( libHandle,
      "Toupcam_Flush" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_AEAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_get_AEAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_AutoExpoEnable ) = _getDLSym ( libHandle,
      "Toupcam_get_AutoExpoEnable" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_AutoExpoTarget ) = _getDLSym ( libHandle,
      "Toupcam_get_AutoExpoTarget" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_AWBAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_get_AWBAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Brightness ) = _getDLSym ( libHandle,
      "Toupcam_get_Brightness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Chrome ) = _getDLSym ( libHandle,
      "Toupcam_get_Chrome" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Contrast ) = _getDLSym ( libHandle,
      "Toupcam_get_Contrast" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_eSize ) = _getDLSym ( libHandle,
      "Toupcam_get_eSize" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_ExpoAGain ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpoAGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_ExpoAGainRange ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpoAGainRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_ExpoTime ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpoTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_ExpTimeRange ) = _getDLSym ( libHandle,
      "Toupcam_get_ExpTimeRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  /*
  if (!( *( void** )( &p_Mallincam_get_FanMaxSpeed ) = _getDLSym ( libHandle,
      "Toupcam_get_FanMaxSpeed" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Field ) = _getDLSym ( libHandle,
      "Toupcam_get_Field" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  if (!( *( void** )( &p_Mallincam_get_FwVersion ) = _getDLSym ( libHandle,
      "Toupcam_get_FwVersion" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Gamma ) = _getDLSym ( libHandle,
      "Toupcam_get_Gamma" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_HFlip ) = _getDLSym ( libHandle,
      "Toupcam_get_HFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_GetHistogram ) = _getDLSym ( libHandle,
      "Toupcam_GetHistogram" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Hue ) = _getDLSym ( libHandle,
      "Toupcam_get_Hue" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_HwVersion ) = _getDLSym ( libHandle,
      "Toupcam_get_HwVersion" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_HZ ) = _getDLSym ( libHandle,
      "Toupcam_get_HZ" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_LevelRange ) = _getDLSym ( libHandle,
      "Toupcam_get_LevelRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_MaxBitDepth ) = _getDLSym ( libHandle,
      "Toupcam_get_MaxBitDepth" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_MaxSpeed ) = _getDLSym ( libHandle,
      "Toupcam_get_MaxSpeed" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Mode ) = _getDLSym ( libHandle,
      "Toupcam_get_Mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_MonoMode ) = _getDLSym ( libHandle,
      "Toupcam_get_MonoMode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Negative ) = _getDLSym ( libHandle,
      "Toupcam_get_Negative" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Option ) = _getDLSym ( libHandle,
      "Toupcam_get_Option" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  /*
  if (!( *( void** )( &p_Mallincam_get_PixelSize ) = _getDLSym ( libHandle,
      "Toupcam_get_PixelSize" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  if (!( *( void** )( &p_Mallincam_get_ProductionDate ) = _getDLSym ( libHandle,
      "Toupcam_get_ProductionDate" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_RawFormat ) = _getDLSym ( libHandle,
      "Toupcam_get_RawFormat" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_RealTime ) = _getDLSym ( libHandle,
      "Toupcam_get_RealTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Resolution ) = _getDLSym ( libHandle,
      "Toupcam_get_Resolution" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_ResolutionNumber ) = _getDLSym ( libHandle,
      "Toupcam_get_ResolutionNumber" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_ResolutionRatio ) = _getDLSym ( libHandle,
      "Toupcam_get_ResolutionRatio" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Roi ) = _getDLSym ( libHandle,
      "Toupcam_get_Roi" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  /*
  if (!( *( void** )( &p_Mallincam_get_RoiMode ) = _getDLSym ( libHandle,
      "Toupcam_get_RoiMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  if (!( *( void** )( &p_Mallincam_get_Saturation ) = _getDLSym ( libHandle,
      "Toupcam_get_Saturation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_SerialNumber ) = _getDLSym ( libHandle,
      "Toupcam_get_SerialNumber" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Size ) = _getDLSym ( libHandle,
      "Toupcam_get_Size" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Speed ) = _getDLSym ( libHandle,
      "Toupcam_get_Speed" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_StillResolution ) = _getDLSym ( libHandle,
      "Toupcam_get_StillResolution" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_StillResolutionNumber ) = _getDLSym (
      libHandle, "Toupcam_get_StillResolutionNumber" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_Temperature ) = _getDLSym ( libHandle,
      "Toupcam_get_Temperature" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_TempTint ) = _getDLSym ( libHandle,
      "Toupcam_get_TempTint" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_VFlip ) = _getDLSym ( libHandle,
      "Toupcam_get_VFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  /*
  if (!( *( void** )( &p_Mallincam_get_VignetAmountInt ) = _getDLSym ( libHandle,
      "Toupcam_get_VignetAmountInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_VignetEnable ) = _getDLSym ( libHandle,
      "Toupcam_get_VignetEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_get_VignetMidPointInt ) = _getDLSym (
      libHandle, "Toupcam_get_VignetMidPointInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  if (!( *( void** )( &p_Mallincam_get_WhiteBalanceGain ) = _getDLSym ( libHandle,
      "Toupcam_get_WhiteBalanceGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_HotPlug ) = _getDLSym ( libHandle,
      "Toupcam_HotPlug" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_LevelRangeAuto ) = _getDLSym ( libHandle,
      "Toupcam_LevelRangeAuto" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Open ) = _getDLSym ( libHandle,
      "Toupcam_Open" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_OpenByIndex ) = _getDLSym ( libHandle,
      "Toupcam_OpenByIndex" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Pause ) = _getDLSym ( libHandle,
      "Toupcam_Pause" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_PullImage ) = _getDLSym ( libHandle,
      "Toupcam_PullImage" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_PullStillImage ) = _getDLSym ( libHandle,
      "Toupcam_PullStillImage" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_AEAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_put_AEAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_AutoExpoEnable ) = _getDLSym ( libHandle,
      "Toupcam_put_AutoExpoEnable" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_AutoExpoTarget ) = _getDLSym ( libHandle,
      "Toupcam_put_AutoExpoTarget" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_AWBAuxRect ) = _getDLSym ( libHandle,
      "Toupcam_put_AWBAuxRect" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Brightness ) = _getDLSym ( libHandle,
      "Toupcam_put_Brightness" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Chrome ) = _getDLSym ( libHandle,
      "Toupcam_put_Chrome" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_ChromeCallback ) = _getDLSym ( libHandle,
      "Toupcam_put_ChromeCallback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Contrast ) = _getDLSym ( libHandle,
      "Toupcam_put_Contrast" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_eSize ) = _getDLSym ( libHandle,
      "Toupcam_put_eSize" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_ExpoAGain ) = _getDLSym ( libHandle,
      "Toupcam_put_ExpoAGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_ExpoCallback ) = _getDLSym ( libHandle,
      "Toupcam_put_ExpoCallback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_ExpoTime ) = _getDLSym ( libHandle,
      "Toupcam_put_ExpoTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Gamma ) = _getDLSym ( libHandle,
      "Toupcam_put_Gamma" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_HFlip ) = _getDLSym ( libHandle,
      "Toupcam_put_HFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Hue ) = _getDLSym ( libHandle,
      "Toupcam_put_Hue" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_HZ ) = _getDLSym ( libHandle,
      "Toupcam_put_HZ" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_LEDState ) = _getDLSym ( libHandle,
      "Toupcam_put_LEDState" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_LevelRange ) = _getDLSym ( libHandle,
      "Toupcam_put_LevelRange" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_MaxAutoExpoTimeAGain ) = _getDLSym (
      libHandle, "Toupcam_put_MaxAutoExpoTimeAGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Mode ) = _getDLSym ( libHandle,
      "Toupcam_put_Mode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Negative ) = _getDLSym ( libHandle,
      "Toupcam_put_Negative" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Option ) = _getDLSym ( libHandle,
      "Toupcam_put_Option" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_RealTime ) = _getDLSym ( libHandle,
      "Toupcam_put_RealTime" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  // We don't worry if this one is missing.  Not all versions of the
  // mallincam libraries support it
  if (!( *( void** )( &p_Mallincam_put_Roi ) = _getDLSym ( libHandle,
      "Toupcam_put_Roi" ))) {
    fprintf ( stderr, "libmallincam.so does not support put_Roi\n" );
  }

  /*
  if (!( *( void** )( &p_Mallincam_put_RoiMode ) = _getDLSym ( libHandle,
      "Toupcam_put_RoiMode" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  if (!( *( void** )( &p_Mallincam_put_Saturation ) = _getDLSym ( libHandle,
      "Toupcam_put_Saturation" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Size ) = _getDLSym ( libHandle,
      "Toupcam_put_Size" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Speed ) = _getDLSym ( libHandle,
      "Toupcam_put_Speed" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_Temperature ) = _getDLSym ( libHandle,
      "Toupcam_put_Temperature" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_TempTint ) = _getDLSym ( libHandle,
      "Toupcam_put_TempTint" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_VFlip ) = _getDLSym ( libHandle,
      "Toupcam_put_VFlip" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  /*
  if (!( *( void** )( &p_Mallincam_put_VignetAmountInt ) = _getDLSym ( libHandle,
      "Toupcam_put_VignetAmountInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_VignetEnable ) = _getDLSym ( libHandle,
      "Toupcam_put_VignetEnable" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_put_VignetMidPointInt ) = _getDLSym (
      libHandle, "Toupcam_put_VignetMidPointInt" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  if (!( *( void** )( &p_Mallincam_put_WhiteBalanceGain ) = _getDLSym ( libHandle,
      "Toupcam_put_WhiteBalanceGain" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_read_EEPROM ) = _getDLSym ( libHandle,
      "Toupcam_read_EEPROM" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  /*
  if (!( *( void** )( &p_Mallincam_read_UART ) = _getDLSym ( libHandle,
      "Toupcam_read_UART" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  if (!( *( void** )( &p_Mallincam_Snap ) = _getDLSym ( libHandle,
      "Toupcam_Snap" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_ST4PlusGuide ) = _getDLSym ( libHandle,
      "Toupcam_ST4PlusGuide" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_ST4PlusGuideState ) = _getDLSym ( libHandle,
      "Toupcam_ST4PlusGuideState" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_StartPullModeWithCallback ) = _getDLSym (
      libHandle, "Toupcam_StartPullModeWithCallback" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_StartPushMode ) = _getDLSym ( libHandle,
      "Toupcam_StartPushMode" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Stop ) = _getDLSym ( libHandle,
      "Toupcam_Stop" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Trigger ) = _getDLSym ( libHandle,
      "Toupcam_Trigger" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_Version ) = _getDLSym ( libHandle,
      "Toupcam_Version" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  if (!( *( void** )( &p_Mallincam_write_EEPROM ) = _getDLSym ( libHandle,
      "Toupcam_write_EEPROM" ))) {
		dlclose ( libHandle );
		libHandle = 0;
    return 0;
  }

  /*
  if (!( *( void** )( &p_Mallincam_write_UART ) = _getDLSym ( libHandle,
      "Mallincam_write_UART" ))) {
			dlclose ( libHandle );
			libHandle = 0;
    return 0;
  }
   */

  numCameras = ( p_Mallincam_Enum )( devList );
  if ( numCameras < 1 ) {
    return 0;
  }

  for ( i = 0; i < numCameras; i++ ) {

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_MALLINCAM;
    ( void ) strncpy ( dev->deviceName, devList[i].displayname,
        OA_MAX_NAME_LEN+1 );
    _private->devIndex = i;
    ( void ) strcpy ( _private->deviceId, devList[i].id );
    dev->_private = _private;
    dev->initCamera = oaMallincamInitCamera;
    dev->hasLoadableFirmware = 0;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
  }

  return numCameras;
}


static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "libmallincam DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
