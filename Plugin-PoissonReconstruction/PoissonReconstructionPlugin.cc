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
#include <ObjectTypes/SplatCloud/SplatCloud.hh>

#include "PoissonReconstructionT.hh"

PoissonPlugin::PoissonPlugin() :
        tool_(0),
        toolIcon_(0)
{

}

void PoissonPlugin::initializePlugin(){
 
  tool_ = new PoissonToolBox();
  
  connect(tool_->reconstructButton, SIGNAL( clicked() ), this, SLOT( slotPoissonReconstruct() ) );

  toolIcon_ = new QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"PoissonReconstruction.png");
  emit addToolbox( tr("Poisson Reconstruction") , tool_, toolIcon_);

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


void PoissonPlugin::pluginsInitialized()
{
  emit setSlotDescription("poissonReconstruct(int,int)",tr("Reconstruct a triangle mesh from the given object."),
      QStringList(tr("ObjectId;depth").split(';')),QStringList(tr("ObjectId of the object;octree depth").split(';')));
  emit setSlotDescription("poissonReconstruct(IdList,int)",tr("Reconstruct one triangle mesh from the given objects."),
      QStringList(tr("IdList;depth").split(';')),QStringList(tr("Id of the objects;octree depth").split(';')));

  emit setSlotDescription("poissonReconstruct(int)",tr("Reconstruct a triangle mesh from the given object. (Octree depth defaults to 7)"),
      QStringList(tr("ObjectId")),QStringList(tr("ObjectId of the object")));
  emit setSlotDescription("poissonReconstruct(IdList)",tr("Reconstruct one triangle mesh from the given objects. (Octree depth defaults to 7)"),
      QStringList(tr("IdList")),QStringList(tr("Id of the objects")));
}

void PoissonPlugin::poissonReconstruct(int _id, int _depth)
{
  IdList list(1,_id);
  poissonReconstruct(list, _depth);
}

void PoissonPlugin::poissonReconstruct(IdList _ids, int _depth)
{
  unsigned int n_points = 0;

  // Data container for the algorithm
  // holds two 3D vectors in 6 columns, first the position, followed by the normal of the point
  std::vector< Real > pt_data;

  //get data from objects
  for (IdList::iterator idIter = _ids.begin(); idIter != _ids.end(); ++idIter)
  {
    BaseObjectData* obj = 0;
    PluginFunctions::getObject(*idIter,obj);
    if ( obj == 0 ) {
      emit log(LOGERR , QString("Unable to get Object width id %1").arg(*idIter));
      continue;
    }

    //Triangle mesh
    if ( obj->dataType() == DATA_TRIANGLE_MESH) {

      // Get triangle mesh
      TriMesh* mesh = PluginFunctions::triMesh(obj);

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
    //Poly mesh
    else if ( obj->dataType() == DATA_POLY_MESH) {
      // Get poly mesh
      PolyMesh* mesh = PluginFunctions::polyMesh(obj);

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
    //Splat cloud
    else if( obj->dataType() == DATA_SPLATCLOUD)
    {

      // Get splat cloud mesh
      SplatCloud* cloud = PluginFunctions::splatCloud(obj);

      if ( ! cloud->hasNormals() ) {
        emit log(LOGERR,"Splat cloud has no normals. Skipping it");
        continue;
      }

      n_points += cloud->numSplats();

      pt_data.reserve( n_points );
      for (unsigned i = 0 ; i < cloud->numSplats(); ++i )
      {
        pt_data.push_back( cloud->positions( i )[0] );
        pt_data.push_back( cloud->positions( i )[1] );
        pt_data.push_back( cloud->positions( i )[2] );
        pt_data.push_back( cloud->normals( i )[0] );
        pt_data.push_back( cloud->normals( i )[1] );
        pt_data.push_back( cloud->normals( i )[2] );
      }
    }
    else
      emit log(LOGERR,QString("ObjectType of Object with id %1 is unsupported").arg(*idIter));
  }


  //create and reconstruct mesh
  if ( !pt_data.empty() ) {

    // Add empty triangle mesh
    int meshId = -1;
    emit addEmptyObject ( DATA_TRIANGLE_MESH, meshId );

    TriMeshObject* finalObject = PluginFunctions::triMeshObject(meshId);

    // Get triangle mesh
    TriMesh* final_mesh = NULL;

    PluginFunctions::getMesh(meshId,final_mesh);

    //Reconstruct
    ACG::PoissonReconstructionT<TriMesh> pr;

    ACG::PoissonReconstructionT<TriMesh>::Parameter params;
    params.Depth = _depth;

    if ( pr.run( pt_data, *final_mesh, params ) ) {
      emit log(LOGINFO,"Reconstruction succeeded");
      emit updatedObject(meshId,UPDATE_ALL);
      finalObject->setName("Poisson Reconstruction.obj");
    } else {
      emit log(LOGERR,"Reconstruction failed");
      emit deleteObject( meshId );
    }

  }
}


void PoissonPlugin::slotPoissonReconstruct(){

  IdList ids;
  for ( PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS,(DATA_TRIANGLE_MESH | DATA_POLY_MESH | DATA_SPLATCLOUD )) ;o_it != PluginFunctions::objectsEnd(); ++o_it)
  {
    ids.push_back(o_it->id());
  }

  const int depth = tool_->depthBox->value();

  poissonReconstruct(ids,depth);

}


Q_EXPORT_PLUGIN2( poissonplugin , PoissonPlugin );

