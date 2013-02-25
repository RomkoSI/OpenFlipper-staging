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
*   $Revision: 10745 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2011-01-26 10:23:50 +0100 (Mi, 26 Jan 2011) $                     *
*                                                                            *
\*===========================================================================*/


//=============================================================================
//
//  SkyDomeTypes
//
//=============================================================================

/**
 * \file SkyDomeType.cc
 * This File contains the implementation for SkyDome
 */


//== INCLUDES =================================================================

#include "SkyDomeType.hh"

#include <iostream>

//== SkyDome Implementation ===============================================


SkyDome::SkyDome() :
horizontalFOV_(360),
verticalFOV_(90),
topOffset_(45),
textureFileName_("")
{

}

/// Defines the texture that will be used
QString SkyDome::textureFileName() {
  return textureFileName_;
}

void SkyDome::setTextureFileName(const QString& _textureFileName) {
  textureFileName_ = _textureFileName;
}

void SkyDome::setHorizontalFOV( const float& _value) {
  horizontalFOV_ = _value;
}

float SkyDome::horizontalFOV() {
  return horizontalFOV_;
}

void SkyDome::setVerticalFOV( const float& _value) {
  verticalFOV_ = _value;
}

float SkyDome::verticalFOV() {
  return verticalFOV_;
}

void SkyDome::setTopOffset( const float& _value) {
  topOffset_ = _value;
}

float SkyDome::topOffset() {
  return topOffset_;
}


//== TYPEDEFS FOR SCENEGRAPH ===============================================
   
//=============================================================================
//=============================================================================
