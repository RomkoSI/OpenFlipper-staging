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




#ifndef ASSIMPPLUGIN_HH
#define ASSIMPPLUGIN_HH

#include <QObject>

#include <OpenFlipper/common/Types.hh>
#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/FileInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/ScriptInterface.hh>
#include <OpenFlipper/BasePlugin/TypeInterface.hh>

#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


class AssimpPlugin : public QObject, BaseInterface, FileInterface, LoadSaveInterface,
    LoggingInterface, ScriptInterface
{
  Q_OBJECT
  Q_INTERFACES(FileInterface)
  Q_INTERFACES(LoadSaveInterface)
  Q_INTERFACES(LoggingInterface)
  Q_INTERFACES(BaseInterface)
  Q_INTERFACES(ScriptInterface)

  signals:
    void openedFile( int _id );
    void addEmptyObject( DataType _type, int& _id);
    void load(QString _filename, DataType _type, int& _id);
    void save(int _id , QString _filename );
    void log(Logtype _type, QString _message);
    void log(QString _message);
    void updateView();

    void deleteObject( int _id );

public:
  AssimpPlugin();

  QString name() { return (QString("AssimpPlugin")); }
  QString description( ) { return (QString(tr("Load/Save Files with the assimp library"))); }

  DataType supportedType();

  QString getSaveFilters();
  QString getLoadFilters();

  QWidget* saveOptionsWidget(QString /*_currentFilter*/);
  QWidget* loadOptionsWidget(QString /*_currentFilter*/);

public slots:

  /// Loads Object and converts it to a triangle mesh if possible
  int loadObject(QString _filename);

  /// Loads Object with given datatype
  int loadObject(QString _filename, DataType _type);

  bool saveObject(int _id, QString _filename);

  QString version() { return QString("1.0"); }

private slots:

  void fileOpened( int /*_id*/ ){}

  void noguiSupported( ) {}

  void initializePlugin();

private:

  int convertAiSceneToOpenMesh(const aiScene* _scene, QString _objectName);

  bool convertOpenMeshToAiScene(aiScene* _scene, BaseObjectData* _object);

  /// converts _mesh into _polyMesh
  void convertAiMeshToPolyMesh(PolyMesh* _polyMesh, aiMesh* _mesh);

  /// converts _mesh into _triMesh
  void convertAiMeshToTriMesh(TriMesh* _triMesh, aiMesh* _mesh);

  bool convertPolyMeshToAiMesh(PolyMesh* _polyMesh, aiMesh* _mesh);

  bool convertTriMeshToAiMesh(TriMesh* _triMesh, aiMesh* _mesh);

  /// add a vertex from _mesh to _polyMesh and stores the index to handle mapping
  void mapVertices(PolyMesh* _polyMesh, aiMesh* _mesh);

  /// add a vertex from _mesh to _trimesh and stores the index to handle mapping
  void mapVertices(TriMesh* _triMesh, aiMesh* _mesh);

private:
  //Option Widgets
  QWidget* loadOptions_;
  QWidget* saveOptions_;

  DataType type_;

  /// maps indices of vertices in an aiMesh to OpenMesh VertexHandles
  std::map<unsigned int, OpenMesh::VertexHandle> vertexHandles_;
};

#endif // ASSIMPPLUGIN_HH
