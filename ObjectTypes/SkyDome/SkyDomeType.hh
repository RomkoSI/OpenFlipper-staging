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
 * \file SkyDomeType.hh
 * This File contains the required types and typedefs for using SkyDome
 */

#ifndef SKYDOME_TYPE_HH
#define SKYDOME_TYPE_HH

//== INCLUDES =================================================================

#include <OpenFlipper/common/GlobalDefines.hh>
#include <QString>

//== SkyDome Type ===============================================

class ACGDLLEXPORT SkyDome {

public:
  SkyDome() :
    horizontalFOV_(360),
    verticalFOV_(90),
    topOffset_(45),
    textureFileName_("")
  {

  }

  /// Defines the texture that will be used
  QString textureFileName() {
    return textureFileName_;
  }

  void setTextureFileName(QString _textureFileName) {
    textureFileName_ = _textureFileName;
  }

  void setHorizontalFOV( float _value) {
    horizontalFOV_ = _value;
  }

  float horizontalFOV() {
    return horizontalFOV_;
  }

  void setVerticalFOV( float _value) {
    verticalFOV_ = _value;
  }

  float verticalFOV() {
    return verticalFOV_;
  }

  void setTopOffset( float _value) {
    topOffset_ = _value;
  }

  float topOffset() {
    return topOffset_;
  }

private:

  // Angle defining the horizontal field of view of the current Dome
  float horizontalFOV_;

  // Angle defining the vertical field of view of the current Dome
  float verticalFOV_;

  // Angle defining the top offset (missing part) of the Dome
  float topOffset_;

  QString textureFileName_;


};


//== TYPEDEFS FOR SCENEGRAPH ===============================================
   
//=============================================================================
#endif //SKYDOME_TYPE_HH
//=============================================================================
