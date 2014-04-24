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

#pragma once

#include <QObject>

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/RenderInterface.hh>

#include <ACG/GL/IRenderer.hh>
#include <ACG/GL/FBO.hh>
#include <ACG/GL/globjects.hh>
#include <ACG/GL/AntiAliasing.hh>

#include <vector>

class DeferredShading : public QObject, BaseInterface, RenderInterface, ACG::IRenderer
{
  Q_OBJECT
    Q_INTERFACES(BaseInterface)
    Q_INTERFACES(RenderInterface)

#if QT_VERSION >= 0x050000
  Q_PLUGIN_METADATA(IID "org.OpenFlipper.Plugins.Plugin-Deferred-Shading")
#endif

public:
  DeferredShading();
  ~DeferredShading();

  QString name() { return (QString("Deferred Shading Renderer")); };
  QString description( ) { return (QString(tr("Render scene with deferred shading for each light"))); };

public slots:
  QString version() { return QString("1.0"); };
  QString renderObjectsInfo(bool _outputShaderInfo);

  QAction* optionsAction();

private slots:

  //BaseInterface
  void initializePlugin();
  void exit(){}

  // RenderInterface
  void render(ACG::GLState* _glState, Viewer::ViewerProperties& _properties);
  QString rendererName() {return QString("Deferred_Shading");}
  void supportedDrawModes(ACG::SceneGraph::DrawModes::DrawMode& _mode) {_mode = ACG::SceneGraph::DrawModes::DEFAULT;}
  
  void reloadShaders();

  QString checkOpenGL();


  void slotMSAASelection( QAction *  );

private:

  void loadShader();

  /// emissive color pass
  GLSL::Program* progEmissive_;

  /// emissive color pass with multisampling
  GLSL::Program* progEmissiveMS_;

  /// background color pass
  GLSL::Program* progBackground_;

  /// directional lighting pass
  GLSL::Program* progDirectional_;

  /// directional lighting pass with multisampling
  GLSL::Program* progDirectionalMS_;

  /// point lighting pass
  GLSL::Program* progPoint_;

  /// point lighting pass with multisampling
  GLSL::Program* progPointMS_;


  /// spot lighting pass
  GLSL::Program* progSpot_;

  /// spot lighting pass with multisampling
  GLSL::Program* progSpotMS_;

  /// Collection of fbos for each viewport
  struct ViewerResources
  {
    ViewerResources() : scene_(0) {}
    ~ViewerResources() {delete scene_;}

    void resize( int _newWidth, int _newHeight, int& _numSamples );

    ACG::FBO* scene_;
  };

  ACG::TextureBuffer filterWeightsBuffer_;
  int numSamples_;

  ACG::TextureBuffer materialBuffer_;
  std::vector<ACG::Vec3f> materialBufferData_;

  /**
  * Stores fbo resources for each viewport.
  * Mapping: viewerID -> ViewerResources
  */
  std::map<int, ViewerResources> viewerRes_;
};

