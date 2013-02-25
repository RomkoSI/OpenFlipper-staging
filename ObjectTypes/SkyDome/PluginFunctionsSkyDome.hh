/*===========================================================================*\
*                                                                            *
*                              OpenFlipper                                   *
*      Copyright (C) 2001-2011 by Computer Graphics Group, RWTH Aachen       *
*                           www.openflipper.org                              *
*                                                                            *
*--------------------------------------------------------------------------- *
*  This file is part of OpenFlipper.                                         *
*                                                                            *
*  OpenFlipper is free software: you can redistribute it and/or modify       *
*  it under the terms of the GNU Lesser General Public License as            *
*  published by the Free Software Foundation, either version 3 of            *
*  the License, or (at your option) any later version with the               *
*  following exceptions:                                                     *
*                                                                            *
*  If other files instantiate templates or use macros                        *
*  or inline functions from this file, or you compile this file and          *
*  link it with other files to produce an executable, this file does         *
*  not by itself cause the resulting executable to be covered by the         *
*  GNU Lesser General Public License. This exception does not however        *
*  invalidate any other reasons why the executable file might be             *
*  covered by the GNU Lesser General Public License.                         *
*                                                                            *
*  OpenFlipper is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should have received a copy of the GNU LesserGeneral Public           *
*  License along with OpenFlipper. If not,                                   *
*  see <http://www.gnu.org/licenses/>.                                       *
*                                                                            *
\*===========================================================================*/

/*===========================================================================*\
*                                                                            *
*   $Revision: 13743 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2012-02-10 08:41:26 +0100 (Fr, 10 Feb 2012) $                     *
*                                                                            *
\*===========================================================================*/




//=============================================================================
//
//  Standard Functions
//
//=============================================================================

/**
 * \file PluginFunctionsSkyDome.hh
 * This file contains functions which can be used by plugins to access SkyDomes in the framework.
 */

#pragma once

#include <ObjectTypes/SkyDome/SkyDomeObject.hh>
#include <OpenFlipper/common/Types.hh>

/** The Namespace PluginFunctions contains functions for all plugins. */
namespace PluginFunctions {

/** This functions returns the object with the given id if it is a SkyDomeObject.
 * See get_object(  int _identifier , BaseObject*& _object ) for more details.
 */
DLLEXPORT
bool getObject(  int _identifier , SkyDomeObject*& _object );

/** \brief Get a SkyDomeNode from an object.
 *
 * @param _object The object should be of type BaseDataObject. If the content is a SkyDome, a
 *                SkyDomeNode will be returned. Otherwise a NULL pointer is returned.
 */
DLLEXPORT
SkyDomeNode* skyDomeNode( BaseObjectData* _object );

/** \brief Cast an BaseObject to a SkyDomeObject if possible
 *
 * @param _object The object should be of type BaseDataObject. If the content is a SkyDome, a
 *                a SkyDomeObject is returned. Otherwise a NULL pointer is returned.
 */
DLLEXPORT
SkyDomeObject* skyDomeObject( BaseObjectData* _object );

/** \brief Get a SkyDomeObject from id if possible
*
* @param _objectId If the content is a SkyDome, a SkyDomeObject is returned. Otherwise a NULL pointer is returned.
*/
DLLEXPORT
SkyDomeObject* skyDomeObject( int _objectId );

}
