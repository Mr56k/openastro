/*****************************************************************************
 *
 * sxInit.c -- Initialise Starlight Xpress filter wheels
 *
 * Copyright 2014,2015,2018 James Fidell (james@openastroproject.org)
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

#include <hidapi.h>
#include <errno.h>
#include <pthread.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "sxfw.h"


static void _sxInitFunctionPointers ( oaFilterWheel* );
static int  _getNumSlots ( oaFilterWheel* );

/**
 * Initialise a given filter wheel device
 */

oaFilterWheel*
oaSXInitFilterWheel ( oaFilterWheelDevice* device )
{
  hid_device*				hidHandle = 0;
  oaFilterWheel*			wheel;
  DEVICE_INFO*				devInfo;
  PRIVATE_INFO*				privateInfo;

  devInfo = device->_private;

  if ( hid_init()) {
    perror ( "hid_init() failed" );
    return 0;
  }

  if (!( wheel = ( oaFilterWheel* ) malloc ( sizeof ( oaFilterWheel )))) {
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }
  if (!( privateInfo = ( PRIVATE_INFO* ) malloc ( sizeof ( PRIVATE_INFO )))) {
    free (( void* ) wheel );
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }

  OA_CLEAR ( *wheel );
  OA_CLEAR ( *privateInfo );
  OA_CLEAR ( wheel->controls );
  // OA_CLEAR ( wheel->features );

  wheel->_private = privateInfo;

  pthread_mutex_init ( &privateInfo->ioMutex, 0 );

  ( void ) strcpy ( wheel->deviceName, device->deviceName );

  _oaInitFilterWheelFunctionPointers ( wheel );
  _sxInitFunctionPointers ( wheel );

  privateInfo->initialised = 0;
  privateInfo->index = -1;

  if (!( hidHandle = hid_open_path ( devInfo->sysPath ))) {
    fprintf ( stderr, "No matching USB device found!\n" );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }

  wheel->interface = device->interface;
  privateInfo->hidHandle = hidHandle;
  privateInfo->index = devInfo->devIndex;
  privateInfo->wheelType = devInfo->devType;
  privateInfo->currentPosition = 1;

  pthread_mutex_init ( &privateInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &privateInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &privateInfo->callbackQueued, 0 );
  pthread_cond_init ( &privateInfo->commandQueued, 0 );
  pthread_cond_init ( &privateInfo->commandComplete, 0 );

  privateInfo->stopControllerThread = privateInfo->stopCallbackThread = 0;
  privateInfo->commandQueue = oaDLListCreate();
  privateInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( privateInfo->controllerThread ), 0,
      oafwSXcontroller, ( void* ) wheel )) {
    free (( void* ) wheel->_private );
    free (( void* ) wheel );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }

  if ( pthread_create ( &( privateInfo->callbackThread ), 0,
      oafwCallbackHandler, ( void* ) wheel )) {

    void* dummy;
    privateInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &privateInfo->commandQueued );
    pthread_join ( privateInfo->controllerThread, &dummy );
    free (( void* ) wheel->_private );
    free (( void* ) wheel );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }


  if (( wheel->numSlots = _getNumSlots ( wheel )) < 1 ) {
    fprintf ( stderr, "%s: invalid number of slots in filter wheel\n",
        __FUNCTION__ );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }

  wheel->controls [ OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC ] = OA_CTRL_TYPE_INT32;
  return wheel;
}


static void
_sxInitFunctionPointers ( oaFilterWheel* wheel )
{
  wheel->funcs.initWheel = oaSXInitFilterWheel;
  wheel->funcs.closeWheel = oaSXWheelClose;
  wheel->funcs.setControl = oaWheelSetControl;
  wheel->funcs.readControl = oaWheelReadControl;
  // wheel->funcs.testControl = XXXX;
}


int
_getNumSlots ( oaFilterWheel* wheel )
{
  unsigned char		buffer[2];
  int			numSlots;
  PRIVATE_INFO*		privateInfo;

  privateInfo = wheel->_private;

  pthread_mutex_lock ( &privateInfo->ioMutex );

  buffer[0] = 0;
  buffer[1] = 1;
  if ( _sxWheelWrite ( privateInfo, buffer )) {
    fprintf ( stderr, "%s: write error on get filters command\n",
      __FUNCTION__ );
    pthread_mutex_unlock ( &privateInfo->ioMutex );
    return 0;
  }

  if ( _sxWheelRead ( privateInfo, buffer )) {
    pthread_mutex_unlock ( &privateInfo->ioMutex );
    return 0;
  }

  do {
    sleep ( 1 );
    buffer[0] = 0;
    buffer[1] = 0;
    if ( _sxWheelWrite ( privateInfo, buffer )) {
      fprintf ( stderr, "%s: write error on get filters command\n",
        __FUNCTION__ );
      pthread_mutex_unlock ( &privateInfo->ioMutex );
      return 0;
    }

    if ( _sxWheelRead ( privateInfo, buffer )) {
      pthread_mutex_unlock ( &privateInfo->ioMutex );
      return 0;
    }
  } while ( !buffer[0] );

  pthread_mutex_unlock ( &privateInfo->ioMutex );

  numSlots = buffer[1];
  return numSlots;
}


int
oaSXWheelClose ( oaFilterWheel* wheel )
{
  PRIVATE_INFO*		privateInfo;

  privateInfo = wheel->_private;
  privateInfo->initialised = 0;
  hid_close ( privateInfo->hidHandle );
  return 0;
}
