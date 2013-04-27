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
*   $Revision: 13361 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2012-01-12 16:33:16 +0100 (Thu, 12 Jan 2012) $                     *
*                                                                            *
\*===========================================================================*/




#include "AssimpPlugin.hh"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

AssimpPlugin::AssimpPlugin()
  :
  loadOptions_(0),
  saveOptions_(0),
  type_(DATA_TRIANGLE_MESH)
{
}

void AssimpPlugin::initializePlugin() {
}

int AssimpPlugin::convertToOpenMesh(const aiScene *_scene) {
    int objectId = -1;
    emit addEmptyObject(type_, objectId);

    BaseObject* object(0);
    if(!PluginFunctions::getObject( objectId, object )) {
        emit log(LOGERR, tr("Could not create new object!"));
        return -1;
    }

    PolyMeshObject* polyMeshObj = dynamic_cast< PolyMeshObject* > (object);
    TriMeshObject*  triMeshObj  = dynamic_cast< TriMeshObject*  > (object);

    if (polyMeshObj) {
      for (unsigned int i = 0; i < _scene->mNumMeshes; ++i)
        convertAiMesh(polyMeshObj->mesh(), _scene->mMeshes[i]);

      polyMeshObj->update();
      polyMeshObj->show();

    } else if (triMeshObj) {
      for (unsigned int i = 0; i < _scene->mNumMeshes; ++i)
        convertAiMesh(triMeshObj->mesh(), _scene->mMeshes[i]);

      triMeshObj->update();
      triMeshObj->show();
    }

    emit openedFile( object->id() );

    // Update viewport
    PluginFunctions::viewAll();

    return objectId;
}

void AssimpPlugin::convertAiMesh(PolyMesh *_polyMesh, aiMesh *_mesh) {
  mapVertices(_polyMesh, _mesh);

  std::vector<OpenMesh::VertexHandle> vhandles;
  for (unsigned int i = 0; i < _mesh->mNumFaces; ++i) {
    aiFace& face = _mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j) {
      vhandles.push_back(vertexHandles_[face.mIndices[j]]);
      if (_mesh->HasNormals()) {
        aiVector3D& aiNormal = _mesh->mNormals[face.mIndices[j]];
        _polyMesh->set_normal(vhandles.back(), ACG::Vec3d(aiNormal.x, aiNormal.y, aiNormal.z));
      }
    }

    _polyMesh->add_face(vhandles);
    vhandles.clear();
  }

  if (!_mesh->HasNormals())
    _polyMesh->update_normals();
  else
    _polyMesh->update_face_normals();
}

void AssimpPlugin::convertAiMesh(TriMesh *_triMesh, aiMesh *_mesh) {
  mapVertices(_triMesh, _mesh);

  std::vector<OpenMesh::VertexHandle> vhandles;
  for (unsigned int i = 0; i < _mesh->mNumFaces; ++i) {
    aiFace& face = _mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j) {
      vhandles.push_back(vertexHandles_[face.mIndices[j]]);
      if (_mesh->HasNormals()) {
        aiVector3D& aiNormal = _mesh->mNormals[face.mIndices[j]];
        _triMesh->set_normal(vhandles.back(), ACG::Vec3d(aiNormal.x, aiNormal.y, aiNormal.z));
      }
    }

    _triMesh->add_face(vhandles);
    vhandles.clear();
  }

  if (!_mesh->HasNormals())
    _triMesh->update_normals();
  else
    _triMesh->update_face_normals();
}

void AssimpPlugin::mapVertices(PolyMesh *_polyMesh, aiMesh *_mesh) {
  vertexHandles_.clear();

  for (unsigned int i = 0; i < _mesh->mNumVertices; ++i) {
    vertexHandles_[i] = _polyMesh->add_vertex(ACG::Vec3d(_mesh->mVertices[i].x, _mesh->mVertices[i].y, _mesh->mVertices[i].z));
  }
}

void AssimpPlugin::mapVertices(TriMesh *_triMesh, aiMesh *_mesh) {
  vertexHandles_.clear();

  for (unsigned int i = 0; i < _mesh->mNumVertices; ++i) {
    vertexHandles_[i] = _triMesh->add_vertex(ACG::Vec3d(_mesh->mVertices[i].x, _mesh->mVertices[i].y, _mesh->mVertices[i].z));
  }
}

DataType AssimpPlugin::supportedType() {
  DataType type = DATA_POLY_MESH | DATA_TRIANGLE_MESH;
  return type;
}


QString AssimpPlugin::getSaveFilters() {
  return QString( tr("Alias/Wavefront ( *.obj )") );
}

QString AssimpPlugin::getLoadFilters() {
  return QString( tr("Alias/Wavefront ( *.obj )") );
}

QWidget *AssimpPlugin::saveOptionsWidget(QString) {
  if (!saveOptions_) {
    saveOptions_ = new QWidget();
  }

  return saveOptions_;
}

QWidget *AssimpPlugin::loadOptionsWidget(QString) {
  if (!loadOptions_) {
    loadOptions_ = new QWidget();
  }

  return loadOptions_;
}

int AssimpPlugin::loadObject(QString _filename) {
  Assimp::Importer importer;

  const aiScene* scene = NULL;
  if (type_ == DATA_TRIANGLE_MESH)
    scene = importer.ReadFile(_filename.toStdString(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
  else
    scene = importer.ReadFile(_filename.toStdString(), aiProcess_JoinIdenticalVertices);

  if (!scene) {
    emit log(LOGERR, tr(importer.GetErrorString()));
    return -1;
  }

  return convertToOpenMesh(scene);
}

int AssimpPlugin::loadObject(QString _filename, DataType _type) {
  type_ = _type;
  return loadObject(_filename);
}

bool AssimpPlugin::saveObject(int _id, QString _filename) {
  return true;
}

Q_EXPORT_PLUGIN2( assimpplugin , AssimpPlugin );
