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
*   $Revision: 13354 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2012-01-12 13:39:10 +0100 (Do, 12 Jan 2012) $                     *
*                                                                            *
\*===========================================================================*/


#include <QtGui>

#include "PanoramaControlPlugin.hh"
#include <iostream>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/GlobalOptions.hh>
#include <ObjectTypes/SkyDome/SkyDome.hh>

PanoramaControlPlugin::PanoramaControlPlugin() :
        tool_(0)
{

}

void PanoramaControlPlugin::initializePlugin(){

  tool_ = new PanoramaToolBox();
  
  connect(tool_->hFOV  , SIGNAL( valueChanged(double) ), this, SLOT( slotValuesChanged(double) ) );
  connect(tool_->vFOV  , SIGNAL( valueChanged(double) ), this, SLOT( slotValuesChanged(double) ) );
  connect(tool_->cutOff, SIGNAL( valueChanged(double) ), this, SLOT( slotValuesChanged(double) ) );

  connect(tool_->loadButton, SIGNAL( clicked() ), this, SLOT( slotLoadImage() ) );

  //toolIcon_ = new QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"PoissonReconstruction.png");
  emit addToolbox( tr("Panorama Control") , tool_);

}

void PanoramaControlPlugin::slotLoadImage() {

  SkyDomeObject* domeObject = 0;

  for ( PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS,( DATA_SKYDOME )) ;o_it != PluginFunctions::objectsEnd(); ++o_it)
  {
    domeObject = PluginFunctions::skyDomeObject(*o_it);

    // We only take the first one
    if ( domeObject )
      break;
  }

  // No dome object found, so we have to create one
  if ( !domeObject ) {

    // Add empty triangle mesh
    int domeId = -1;
    emit addEmptyObject ( DATA_SKYDOME, domeId );

    domeObject = PluginFunctions::skyDomeObject(domeId);
  }

  if ( !domeObject ) {
    emit log(LOGERR,"Unable to get or create new Sky Dome Object");
    return;
  }

  // Ask user to load the file
  QString file = QFileDialog::getOpenFileName ( 0, "Load Panorama Image", "", "Images (*.png *.xpm *.jpg)", 0, 0 );

  // Update the texture file name
  domeObject->getSkyDome().setTextureFileName(file);

  // Update the object and its buffers
  emit updatedObject(domeObject->id(), UPDATE_TEXTURE);

  // Switch to shader pipeline renderer
  emit setRenderer(PluginFunctions::activeExaminer(),"Shader Pipeline Renderer Plugin");

}

void PanoramaControlPlugin::slotValuesChanged(double) {

  for ( PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS,( DATA_SKYDOME )) ;o_it != PluginFunctions::objectsEnd(); ++o_it)
  {
    SkyDomeObject* domeObject = PluginFunctions::skyDomeObject(*o_it);

    if ( domeObject ) {
      domeObject->getSkyDome().setHorizontalFOV( tool_->hFOV->value() );
      domeObject->getSkyDome().setVerticalFOV(tool_->vFOV->value() );
      domeObject->getSkyDome().setTopOffset(tool_->cutOff->value() );

      // Only first object will be handled for now!
      break;
    }

  }

  // Only update the view, as only uniforms will change here
  emit updateView();

}


Q_EXPORT_PLUGIN2( panoramacontrolplugin , PanoramaControlPlugin );

