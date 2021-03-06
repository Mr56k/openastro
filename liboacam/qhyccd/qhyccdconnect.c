/*****************************************************************************
 *
 * qhyccdconnect.c -- Initialise cameras supported by libqhyccd
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#include <openastro/camera.h>
#include <qhyccd.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "qhyccdprivate.h"
#include "qhyccdoacam.h"
#include "qhyccdstate.h"

static void _QHYCCDInitFunctionPointers ( oaCamera* );

struct qhyCtrl QHYControlData[] = {
  {
    .qhyControl   = CONTROL_BRIGHTNESS,
    .oaControl    = OA_CAM_CTRL_BRIGHTNESS,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_CONTRAST,
    .oaControl    = OA_CAM_CTRL_CONTRAST,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_WBR,
    .oaControl    = OA_CAM_CTRL_RED_BALANCE,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_WBB,
    .oaControl    = OA_CAM_CTRL_BLUE_BALANCE,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_WBG,
    .oaControl    = OA_CAM_CTRL_GREEN_BALANCE,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_GAMMA,
    .oaControl    = OA_CAM_CTRL_GAMMA,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_GAIN,
    .oaControl    = OA_CAM_CTRL_GAIN,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_EXPOSURE,
    .oaControl    = OA_CAM_CTRL_EXPOSURE_ABSOLUTE,
    .oaControlType  = OA_CTRL_TYPE_INT64
  }, {
    .qhyControl   = CONTROL_SPEED,
    .oaControl    = OA_CAM_CTRL_SPEED,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_USBTRAFFIC,
    .oaControl    = OA_CAM_CTRL_USBTRAFFIC,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_ROWNOISERE,
    .oaControl    = OA_CAM_CTRL_NOISE_REDUCTION,
    .oaControlType  = OA_CTRL_TYPE_INT32
  }, {
    .qhyControl   = CONTROL_CURTEMP,
    .oaControl    = OA_CAM_CTRL_TEMPERATURE,
    .oaControlType  = OA_CTRL_TYPE_READONLY
  }
};

unsigned int numQHYControls = sizeof ( QHYControlData ) /
		sizeof ( struct qhyCtrl );

/**
 * Initialise a given camera device
 */

oaCamera*
oaQHYCCDInitCamera ( oaCameraDevice* device )
{
  oaCamera*					camera;
  QHYCCD_STATE*			cameraInfo;
  COMMON_INFO*			commonInfo;
  unsigned int			numCameras, oactrl, cfaMask;
	qhyccd_handle*		handle;
  DEVICE_INFO*			devInfo;
  unsigned int			i, j;
  int								found, binModes, maxBytesPerPixel;
	double						min, max, step, pixelSizeX, pixelSizeY, ddummy;
	uint32_t					dummy, x, y;
	CONTROL_ID				binning;

	devInfo = device->_private;

  if ( p_InitQHYCCDResource() != QHYCCD_SUCCESS ) {
    fprintf ( stderr, "can't init libqhyccd\n" );
    return 0;
  }
  numCameras = ( p_ScanQHYCCD )();
  if ( numCameras < 1 || devInfo->devIndex > numCameras ) {
    p_ReleaseQHYCCDResource();
    return 0;
  }

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    p_ReleaseQHYCCDResource();
    perror ( "malloc oaCamera failed" );
    return 0;
  }

  if (!( cameraInfo = ( QHYCCD_STATE* ) malloc (
      sizeof ( QHYCCD_STATE )))) {
    p_ReleaseQHYCCDResource();
    free (( void* ) camera );
    perror ( "malloc QHYCCD_STATE failed" );
    return 0;
  }

  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    p_ReleaseQHYCCDResource();
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }
  OA_CLEAR ( *camera );
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

	found = -1;
	for ( i = 0; i < numCameras && found == -1; i++ ) {
		if ( p_GetQHYCCDId ( i, cameraInfo->qhyccdId ) != QHYCCD_SUCCESS ) {
      p_ReleaseQHYCCDResource();
      fprintf ( stderr, "can't get id for camera %d\n", i );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
		if ( !strcmp ( cameraInfo->qhyccdId, devInfo->deviceId )) {
			found = i;
		}
	}

  _oaInitCameraFunctionPointers ( camera );
  _QHYCCDInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;

  camera->interface = device->interface;

  if (!( handle = ( p_OpenQHYCCD )( cameraInfo->qhyccdId ))) {
    p_ReleaseQHYCCDResource();
    fprintf ( stderr, "Can't get QHYCCD handle\n" );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

	if ( p_SetQHYCCDStreamMode ( handle, 1 ) != QHYCCD_SUCCESS ) {
		p_CloseQHYCCD ( handle );
    p_ReleaseQHYCCDResource();
    fprintf ( stderr, "Can't set streaming mode\n" );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}

	if ( p_InitQHYCCD ( handle ) != QHYCCD_SUCCESS ) {
		p_CloseQHYCCD ( handle );
    p_ReleaseQHYCCDResource();
    fprintf ( stderr, "Can't init camera\n" );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}

	camera->features.readableControls = 1; // allegedy

	for ( i = 0; i < numQHYControls; i++ ) {
		int m = 1, qhyControl = QHYControlData[i].qhyControl;
		if ( p_IsQHYCCDControlAvailable ( handle, qhyControl ) == QHYCCD_SUCCESS ) {
      p_GetQHYCCDParamMinMaxStep ( handle, qhyControl, &min, &max, &step );
			// fprintf ( stderr, "qhy control: %d, min: %f, max: %f, step: %f ", qhyControl, min, max, step );
			while (( step - (( int ) step )) != 0 ) {
				step *= 10;
				m *= 10;
			}
			QHYControlData[i].multiplier = m;
			oactrl = QHYControlData[i].oaControl;
			camera->OA_CAM_CTRL_TYPE ( oactrl ) = QHYControlData[i].oaControlType;
			commonInfo->OA_CAM_CTRL_MIN( oactrl ) = min * m;
			commonInfo->OA_CAM_CTRL_MAX( oactrl ) = max * m;
			commonInfo->OA_CAM_CTRL_STEP( oactrl ) = step;
			// this is just a best guess, really.
			commonInfo->OA_CAM_CTRL_DEF( oactrl ) = p_GetQHYCCDParam (
					handle, qhyControl ) * m;
			//fprintf ( stderr, "def: %f\n", p_GetQHYCCDParam ( handle, qhyControl ));
		}
	}

	// It looks as though all cameras might support ROI
  camera->features.ROI = 1;

	cfaMask = p_IsQHYCCDControlAvailable ( handle, CAM_COLOR );
	if ( cfaMask == BAYER_GB || cfaMask == BAYER_GR || cfaMask == BAYER_BG ||
			cfaMask == BAYER_RG ) {
		cameraInfo->colour = 1;
	} else {
		cameraInfo->colour = 0;
	}

	cameraInfo->has8Bit = ( p_IsQHYCCDControlAvailable ( handle, CAM_8BITS ) ==
			QHYCCD_SUCCESS ) ? 1 : 0;
	cameraInfo->has16Bit = ( p_IsQHYCCDControlAvailable ( handle, CAM_16BITS ) ==
		QHYCCD_SUCCESS ) ? 1 : 0;

	// Assuming all 16-bit modes are big-endian here...

	// We can't tell if the data might be 12 bits padded to 16

	if ( cameraInfo->colour ) {
		if ( cameraInfo->has8Bit ) {
			camera->frameFormats[ OA_PIX_FMT_RGB24 ] = 1;
		}
		if ( cameraInfo->has16Bit ) {
			camera->frameFormats[ OA_PIX_FMT_RGB48LE ] = 1;
		}
    camera->features.rawMode = camera->features.demosaicMode = 1;
		switch ( cfaMask ) {
			case BAYER_GB:
				camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
				camera->frameFormats[ OA_PIX_FMT_GBRG16LE ] = 1;
				break;
			case BAYER_GR:
				camera->frameFormats[ OA_PIX_FMT_GRBG8 ] = 1;
				camera->frameFormats[ OA_PIX_FMT_GRBG16LE ] = 1;
				break;
			case BAYER_BG:
				camera->frameFormats[ OA_PIX_FMT_BGGR8 ] = 1;
				camera->frameFormats[ OA_PIX_FMT_BGGR16LE ] = 1;
				break;
			case BAYER_RG:
				camera->frameFormats[ OA_PIX_FMT_RGGB8 ] = 1;
				camera->frameFormats[ OA_PIX_FMT_RGGB16LE ] = 1;
				break;
		}
		// Force RGB images for startup of colour cameras
		if ( p_SetQHYCCDDebayerOnOff ( handle, cameraInfo->colour ) !=
				QHYCCD_SUCCESS ) {
      fprintf ( stderr, "p_SetQHYCCDDebayerOnOff ( %d ) returns error\n",
					cameraInfo->colour );
			p_CloseQHYCCD ( handle );
			p_ReleaseQHYCCDResource();
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
		}
	} else {
		if ( cameraInfo->has8Bit ) {
			camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
		}
		if ( cameraInfo->has16Bit ) {
			camera->frameFormats[ OA_PIX_FMT_GREY16LE ] = 1;
		}
	}

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  // force camera into 8-bit mode if it has it

	if ( cameraInfo->has8Bit ) {
		if ( p_SetQHYCCDBitsMode ( handle, 8 ) !=
				QHYCCD_SUCCESS ) {
      fprintf ( stderr,
          "SetQHYCCDParam ( transferbit, 8 ) returns error\n" );
			p_CloseQHYCCD ( handle );
			p_ReleaseQHYCCDResource();
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
		if ( cameraInfo->colour ) {
			cameraInfo->currentVideoFormat = OA_PIX_FMT_RGB24;
			cameraInfo->currentBitsPerPixel = 24;
			cameraInfo->currentBytesPerPixel = 3;
		} else {
			cameraInfo->currentVideoFormat = OA_PIX_FMT_GREY8;
			cameraInfo->currentBitsPerPixel = 8;
			cameraInfo->currentBytesPerPixel = 1;
		}
	} else {
		if ( cameraInfo->colour ) {
			cameraInfo->currentVideoFormat = OA_PIX_FMT_RGB48LE;
			cameraInfo->currentBitsPerPixel = 48;
			cameraInfo->currentBytesPerPixel = 6;
		} else {
			cameraInfo->currentVideoFormat = OA_PIX_FMT_GREY16LE;
			cameraInfo->currentBitsPerPixel = 16;
			cameraInfo->currentBytesPerPixel = 2;
		}
	}

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

	if ( p_GetQHYCCDChipInfo ( handle, &ddummy, &ddummy, &x, &y, &pixelSizeX,
			&pixelSizeY, &dummy ) != QHYCCD_SUCCESS ) {
    fprintf ( stderr, "GetQHYCCDChipInfo returns error\n" );
    p_CloseQHYCCD ( handle );
    p_ReleaseQHYCCDResource();
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}
	camera->features.pixelSizeX = pixelSizeX;
	camera->features.pixelSizeY = pixelSizeY;
	cameraInfo->maxResolutionX = cameraInfo->currentXSize = x;
	cameraInfo->maxResolutionY = cameraInfo->currentYSize = y;

	cameraInfo->frameSizes[1].numSizes = 1;
	if (!( cameraInfo->frameSizes[1].sizes = malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "malloc for frame sizes failed\n" );
    p_CloseQHYCCD ( handle );
    p_ReleaseQHYCCDResource();
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}
	cameraInfo->frameSizes[1].sizes[0].x = x;
	cameraInfo->frameSizes[1].sizes[0].y = y;

	binModes = 0;
  for ( binning = CAM_BIN2X2MODE, i = 2; binning <= CAM_BIN4X4MODE;
			binning++, i++ ) {
		if ( p_IsQHYCCDControlAvailable ( handle, binning ) == QHYCCD_SUCCESS ) {
			  cameraInfo->frameSizes[i].numSizes = 1;
			binModes++;
			if (!( cameraInfo->frameSizes[i].sizes = malloc (
					sizeof ( FRAMESIZE )))) {
				fprintf ( stderr, "malloc for frame sizes failed\n" );
				p_CloseQHYCCD ( handle );
				p_ReleaseQHYCCDResource();
				free (( void* ) commonInfo );
				free (( void* ) cameraInfo );
				free (( void* ) camera );
				return 0;
			}
			cameraInfo->frameSizes[i].sizes[0].x = x / i;
			cameraInfo->frameSizes[i].sizes[0].y = y / i;
		} else {
			  cameraInfo->frameSizes[i].numSizes = 0;
		}
	}
  cameraInfo->binMode = 1;
  if ( binModes ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_DISCRETE;
  }

	maxBytesPerPixel = 1;
	if ( cameraInfo->colour ) {
		maxBytesPerPixel = 3;
		if ( cameraInfo->has16Bit ) {
			maxBytesPerPixel = 6;
		}
	} else {
		if ( cameraInfo->has16Bit ) {
			maxBytesPerPixel = 2;
		}
	}

  // The largest buffer size we should need

  cameraInfo->buffers = 0;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * maxBytesPerPixel;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof (
      struct qhyccdbuffer ));
  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      fprintf ( stderr, "%s malloc failed\n", __FUNCTION__ );
      if ( i ) {
        for ( j = 0; j < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
          cameraInfo->buffers[j].start = 0;
        }
      }
      // FIX ME -- free frame data
			p_CloseQHYCCD ( handle );
			p_ReleaseQHYCCDResource();
			free (( void* ) commonInfo );
			free (( void* ) cameraInfo );
			free (( void* ) camera );
      return 0;
    }
  }

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamQHYCCDcontroller, ( void* ) camera )) {
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamQHYCCDcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }

  cameraInfo->handle = handle;
  cameraInfo->initialised = 1;
  return camera;
}


static void
_QHYCCDInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaQHYCCDInitCamera;
  camera->funcs.closeCamera = oaQHYCCDCloseCamera;

  camera->funcs.setControl = oaQHYCCDCameraSetControl;
  camera->funcs.readControl = oaQHYCCDCameraReadControl;
  camera->funcs.testControl = oaQHYCCDCameraTestControl;
  camera->funcs.getControlRange = oaQHYCCDCameraGetControlRange;
  camera->funcs.getControlDiscreteSet = oaQHYCCDCameraGetControlDiscreteSet;

  camera->funcs.startStreaming = oaQHYCCDCameraStartStreaming;
  camera->funcs.stopStreaming = oaQHYCCDCameraStopStreaming;
  camera->funcs.isStreaming = oaQHYCCDCameraIsStreaming;

  camera->funcs.setResolution = oaQHYCCDCameraSetResolution;
  camera->funcs.setROI = oaQHYCCDCameraSetROI;
  camera->funcs.testROISize = oaQHYCCDCameraTestROISize;

  camera->funcs.hasAuto = oacamHasAuto;

  camera->funcs.enumerateFrameSizes = oaQHYCCDCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaQHYCCDCameraGetFramePixelFormat;
}


int
oaQHYCCDCloseCamera ( oaCamera* camera )
{
  void*			dummy;
  QHYCCD_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    ( p_CloseQHYCCD ) ( cameraInfo->handle );

    free (( void* ) cameraInfo->frameSizes[1].sizes );

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    p_ReleaseQHYCCDResource();
    free (( void* ) camera->_common );
    free (( void* ) cameraInfo );
    free (( void* ) camera );

  } else {
    p_ReleaseQHYCCDResource();
    return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
