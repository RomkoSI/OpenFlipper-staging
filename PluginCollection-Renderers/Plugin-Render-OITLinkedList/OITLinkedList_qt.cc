/*===========================================================================*\
*                                                                            *
*                              OpenFlipper                                   *
*      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen       *
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
*   $Revision: 18127 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2014-02-05 10:12:54 +0100 (Wed, 05 Feb 2014) $                     *
*                                                                            *
\*===========================================================================*/

/** @file
 *
 *  Contains definitions of the OITLinkedList Renderer that require qt headers
 *  which are incompatible with glew.h.
 */

#include "OITLinkedList.hh"

#include <QGLFormat>


QString OITLinkedList::checkOpenGL()
{
  // Get version and check
  QGLFormat::OpenGLVersionFlags flags = QGLFormat::openGLVersionFlags();
  if ( !flags.testFlag(QGLFormat::OpenGL_Version_4_0) ) // actually 4.2 required, but flag missing in qt 4.8
    return QString("Insufficient OpenGL Version! OpenGL 4.0 or higher required");

  // Check extensions
  QString glExtensions = QString((const char*)glGetString(GL_EXTENSIONS));
  QString missing("");
  if ( !glExtensions.contains("ARB_shader_image_load_store") )
    missing += "ARB_shader_image_load_store extension missing\n";

  if ( !glExtensions.contains("GL_ARB_shader_atomic_counters") )
    missing += "GL_ARB_shader_atomic_counters extension missing\n";

  if ( !glExtensions.contains("GL_ARB_gpu_shader5") )
    missing += "GL_ARB_gpu_shader5 extension missing\n";

  return missing;
}