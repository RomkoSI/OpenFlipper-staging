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

#include "PoissonReconstructionPlugin.hh"
#include <iostream>

#include "OpenFlipper/BasePlugin/PluginFunctions.hh"
#include <OpenFlipper/common/GlobalOptions.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>

#include "PoissonReconstructionT.hh"

PoissonPlugin::PoissonPlugin() :
        tool_(0),
        toolIcon_(0)
{

}

void PoissonPlugin::initializePlugin(){
 
  tool_ = new PoissonToolBox();
  
  connect(tool_->reconstructButton, SIGNAL( clicked() ), this, SLOT( slotPoissonReconstruct() ) );

 // toolIcon_ = new QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"slice.png");
  emit addToolbox( tr("Poisson Reconstruction") , tool_);//, toolIcon_);

  QString info =
  "This plugin is based on the code published by Michael Kazhdan and Matthew Bolitho<br>   "
  "<br>                                                                                    "
  "The following license applies to their code: <br>                                       "
  "Copyright (c) 2006, Michael Kazhdan and Matthew Bolitho <br>                            "
  "All rights reserved. <br>                                                               "
  "<br>                                                                                    "
  "Redistribution and use in source and binary forms, with or without modification,        "
  "are permitted provided that the following conditions are met: <br>                      "
  "<br>                                                                                    "
  "Redistributions of source code must retain the above copyright notice, this list of     "
  "conditions and the following disclaimer. Redistributions in binary form must reproduce  "
  "the above copyright notice, this list of conditions and the following disclaimer        "
  "in the documentation and/or other materials provided with the distribution. <br>        "
  "<br>                                                                                    "
  "Neither the name of the Johns Hopkins University nor the names of its contributors      "
  "may be used to endorse or promote products derived from this software without specific  "
  "prior written permission.  <br>                                                         "
  "<br>                                                                                    "
  "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY   "
  "EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES     "
  "OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT     "
  "SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,           "
  "INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    "
  "TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR     "
  "BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN        "
  "CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN      "
  "ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH     "
  "DAMAGE.                                                                                 ";

  emit addAboutInfo(info,"Poisson Reconstruction Plugin");
}



void PoissonPlugin::slotPoissonReconstruct(){

  unsigned int n_points = 0;
  std::vector< Real > pt_data;

  for ( PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS,(DATA_TRIANGLE_MESH | DATA_POLY_MESH)) ;o_it != PluginFunctions::objectsEnd(); ++o_it)  {



    if ( o_it->dataType() == DATA_TRIANGLE_MESH) {

      TriMeshObject* object = PluginFunctions::triMeshObject(*o_it);

      if ( object == 0 ) {
        emit log(LOGWARN , "Unable to get object ( Only Triangle Meshes supported)");
        continue;
      }

      // Get triangle mesh
      TriMesh* mesh = PluginFunctions::triMesh(*o_it);

      n_points += mesh->n_vertices();

      pt_data.reserve( n_points );
      TriMesh::VertexIter vit = mesh->vertices_begin();
      for ( ; vit != mesh->vertices_end(); ++vit )
      {
        pt_data.push_back( mesh->point( vit )[0] );
        pt_data.push_back( mesh->point( vit )[1] );
        pt_data.push_back( mesh->point( vit )[2] );
        pt_data.push_back( mesh->normal( vit )[0] );
        pt_data.push_back( mesh->normal( vit )[1] );
        pt_data.push_back( mesh->normal( vit )[2] );
      }
    }

    if ( o_it->dataType() == DATA_POLY_MESH) {

         PolyMeshObject* object = PluginFunctions::polyMeshObject(*o_it);

         if ( object == 0 ) {
           emit log(LOGWARN , "Unable to get object ( Only Triangle Meshes supported)");
           continue;
         }

         // Get triangle mesh
         PolyMesh* mesh = PluginFunctions::polyMesh(*o_it);

         n_points += mesh->n_vertices();

         pt_data.reserve( n_points );
         PolyMesh::VertexIter vit = mesh->vertices_begin();
         for ( ; vit != mesh->vertices_end(); ++vit )
         {
           pt_data.push_back( mesh->point( vit )[0] );
           pt_data.push_back( mesh->point( vit )[1] );
           pt_data.push_back( mesh->point( vit )[2] );
           pt_data.push_back( mesh->normal( vit )[0] );
           pt_data.push_back( mesh->normal( vit )[1] );
           pt_data.push_back( mesh->normal( vit )[2] );
         }
       }

  }

  // Get triangle mesh
  TriMesh* final_mesh = NULL;

  ACG::PoissonReconstructionT<TriMesh> pr;

  ACG::PoissonReconstructionT<TriMesh>::Parameter params;
  params.Depth = tool_->depthBox->value();

  if ( !pt_data.empty() ) {

    // Add empty triangle mesh
    int meshId = -1;
    emit addEmptyObject ( DATA_TRIANGLE_MESH, meshId );

    TriMeshObject* finalObject = PluginFunctions::triMeshObject(meshId);

    PluginFunctions::getMesh(meshId,final_mesh);

    if ( pr.run( pt_data, *final_mesh, params ) ) {
      emit log(LOGINFO,"Reconstruction succeeded");
      emit updatedObject(meshId,UPDATE_ALL);
      finalObject->setName("Poisson Reconstruction.obj");
      //finalObject->target(true);
    } else {
      emit log(LOGERR,"Reconstruction failed");
      emit deleteObject( meshId );
    }

  }


}


Q_EXPORT_PLUGIN2( poissonplugin , PoissonPlugin );

