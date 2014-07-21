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

#include <ACG/GL/acg_glew.hh>

#include "DeferredShading.hh"

#include <OpenFlipper/common/GlobalOptions.hh>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <ACG/GL/ShaderCache.hh>
#include <ACG/GL/ScreenQuad.hh>
#include <ACG/GL/GLError.hh>
#include <ACG/QtWidgets/QtColorChooserButton.hh>

// =================================================

#define GBUFFER_INCLUDE_FILE            "DeferredShading/GBufferAccess.glsl"

#define SCREENQUAD_VERTEXSHADER_FILE    "DeferredShading/screenquad.glsl"

#define PASS_EMISSIVE_FILE              "DeferredShading/pass_emissive.glsl"
#define PASS_EMISSIVE_MS_FILE              "DeferredShading/pass_emissiveMS.glsl"

#define PASS_DIRECTIONAL_FILE           "DeferredShading/pass_directional.glsl"
#define PASS_DIRECTIONAL_MS_FILE        "DeferredShading/pass_directionalMS.glsl"
#define PASS_POINT_FILE                 "DeferredShading/pass_point.glsl"
#define PASS_POINT_MS_FILE              "DeferredShading/pass_pointMS.glsl"
#define PASS_SPOT_FILE                  "DeferredShading/pass_spot.glsl"
#define PASS_SPOT_MS_FILE               "DeferredShading/pass_spotMS.glsl"

#define PASS_BACKGROUND_FILE            "DeferredShading/pass_background.glsl"

class DeferredShadingInitPass : public ACG::ShaderModifier{
public:

  void modifyVertexIO( ACG::ShaderGenerator* _shader )  {
    _shader->addOutput("vec3 outVertexNormal");
    _shader->addOutput("vec4 outVertexPosVS");

    // force normal input
    _shader->addInput("vec3 inNormal");
  }

  void modifyFragmentIO( ACG::ShaderGenerator* _shader )  {

    // include GBuffer  functions
    _shader->addIncludeFile(ACG::ShaderProgGenerator::getShaderDir() + QDir::separator() + QString(GBUFFER_INCLUDE_FILE));

    _shader->addInput("vec3 outVertexNormal");
    _shader->addInput("vec4 outVertexPosVS");

    // write position, normal and material to mrt fbo
    _shader->addOutput("vec3 outNormal");


    _shader->addUniform("float materialID");
  }

  void modifyVertexEndCode(QStringList* _code)  {
    _code->push_back("outVertexPosVS = sg_vPosVS;");
    _code->push_back("outVertexNormal = g_mWVIT * inNormal;");
  }

  void modifyFragmentEndCode(QStringList* _code)  {
    _code->push_back("outFragment = WritePositionVS(outVertexPosVS, materialID);");
    _code->push_back("outNormal = WriteNormalVS(outVertexNormal);");
  }

  // modifier replaces default lighting
  bool replaceDefaultLightingCode() {return true;}

  void modifyLightingCode(QStringList* _code, int _lightId, ACG::ShaderGenLightType _lightType)  {
    // do nothing, lighting is deferred
  }
  

  static DeferredShadingInitPass instance;
};


DeferredShadingInitPass DeferredShadingInitPass::instance;

// =================================================

DeferredShading::DeferredShading() 
  : progEmissive_(0), progEmissiveMS_(0),
  progBackground_(0),
  progDirectional_(0), progDirectionalMS_(0), 
  progPoint_(0), progPointMS_(0), 
  progSpot_(0), progSpotMS_(0)
{
  numSamples_ = 0;
  ACG::ShaderProgGenerator::registerModifier(&DeferredShadingInitPass::instance);
}


DeferredShading::~DeferredShading() {
  delete progEmissive_;
  delete progEmissiveMS_;
  delete progDirectional_;
  delete progDirectionalMS_;
  delete progPoint_;
  delete progPointMS_;
  delete progSpot_;
  delete progSpotMS_;
  delete progBackground_;
}


void DeferredShading::initializePlugin() {
  ACG::ShaderProgGenerator::setShaderDir(OpenFlipper::Options::shaderDirStr());
}

QString DeferredShading::renderObjectsInfo(bool _outputShaderInfo) {
  std::vector<ACG::ShaderModifier*> modifiers;
  modifiers.push_back(&DeferredShadingInitPass::instance);
  return dumpCurrentRenderObjectsToString(&sortedObjects_[0], _outputShaderInfo, &modifiers);
}

void DeferredShading::render(ACG::GLState* _glState, Viewer::ViewerProperties& _properties) {

#ifdef GL_ARB_texture_buffer_object
  // collect renderobjects + prepare OpenGL state
  prepareRenderingPipeline(_glState, _properties.drawMode(), PluginFunctions::getSceneGraphRootNode());

  // init/update fbos
  ViewerResources* viewRes = &viewerRes_[_properties.viewerId()];
  viewRes->resize(_glState->viewport_width(), _glState->viewport_height(), numSamples_);
  
  // update material buffer

  materialBufferData_.resize((getNumRenderObjects() + 1) * 5);

  // material 0 is used for the background
  materialBufferData_[0] = ACG::Vec3f(.0f,.0f,.0f);
  materialBufferData_[1] = ACG::Vec3f(.0f,.0f,.0f);
  materialBufferData_[2] = ACG::Vec3f(.0f,.0f,.0f);
  materialBufferData_[3] = ACG::Vec3f(.0f,.0f,.0f);
  materialBufferData_[4] = ACG::Vec3f(1.0f,1.0f,0.0f);

  for (int i = 0; i < getNumRenderObjects(); ++i) {
    int offset = (i + 1) * 5;

    materialBufferData_[offset + 0] = sortedObjects_[i]->emissive;
    materialBufferData_[offset + 1] = sortedObjects_[i]->ambient;
    materialBufferData_[offset + 2] = sortedObjects_[i]->diffuse;
    materialBufferData_[offset + 3] = sortedObjects_[i]->specular;
    materialBufferData_[offset + 4] = ACG::Vec3f(sortedObjects_[i]->alpha, sortedObjects_[i]->shininess, 1.0f);
  }

  int matBufSize = (getNumRenderObjects() + 1) * 5 * 12;
  if (matBufSize)
    materialBuffer_.setBufferData(matBufSize, &materialBufferData_[0], GL_RGB32F, GL_DYNAMIC_DRAW);


  // --------------------------------------------------
  // 1st pass: draw scene to G-buffer

  // enable color/depth write access
  glDepthMask(1);
  glColorMask(1,1,1,1);

  // bind G-buffer fbo
  viewRes->scene_->bind();
  glViewport(0, 0, _glState->viewport_width(), _glState->viewport_height());

  //  attachment0: depth view space, material id
  //  attachment1: normals
  const GLenum colorTarget[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

  // set G-buffer to zero
  for (int i = 0; i < 2; ++i)  {
    glDrawBuffer(colorTarget[i]);

    ACG::Vec4f clearColor(0.0f, 0.0f, 0.0f, 0.0f); 
    
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // initialize G-buffer
  glDrawBuffers(2, colorTarget);

  ACG::GLMatrixd matProj;
  matProj.clear();
  

  // render every object
  for (int i = 0; i < getNumRenderObjects(); ++i) {

    // Take original shader and modify the output to output position, normal and material
    GLSL::Program* prog = ACG::ShaderCache::getInstance()->getProgram(&sortedObjects_[i]->shaderDesc, DeferredShadingInitPass::instance);

    int bRelink = 0;
    if (prog->getFragDataLocation("outFragment") != 0) {
      prog->bindFragDataLocation(0, "outFragment");
      bRelink++;
    }
    if (prog->getFragDataLocation("outNormal") != 1) {
      prog->bindFragDataLocation(1, "outNormal");
      bRelink++;
    }
    if (bRelink)
      prog->link();

    prog->use();

    prog->setUniform("materialID", GLfloat(i+1));


    renderObject(sortedObjects_[i], prog);


    std::string objName = sortedObjects_[i]->debugName;

    if (matProj(0,0) == 0.0)
      matProj = sortedObjects_[i]->proj;

    // prefer to use the projection matrix of a mesh node
    if (objName.compare("MeshNode") == 0)
        matProj = sortedObjects_[i]->proj;
  }

  // extract near and far clipping planes
  const ACG::Vec2f clipPlanes(float(matProj(2,3) / (matProj(2,2) - 1.0)), float(matProj(2,3) / (matProj(2,2) + 1.0)));

  viewRes->scene_->unbind();

  // ----------------------------------------------------------
  // lighting passes

  // enable/disable multisampling
  if (numSamples_)  {
    ACG::MSFilterWeights filterWeights(numSamples_);
    filterWeights.asTextureBuffer(filterWeightsBuffer_);
  }

  // load lighting shaders if not initialized
  if (!progDirectional_)
    reloadShaders();


  // restore previous fbo
  restoreInputFbo();

  // enable color/depth write access
  glDepthMask(1);
  glColorMask(1,1,1,1);

  // note: using glDisable(GL_DEPTH_TEST) not only disables depth testing,
  //  but actually discards any write operations to the depth buffer.
  // However, we can provide scene depth for further post-processing. 
  //   -> Enable depth testing with func=always
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_ALWAYS);
  
  // enable additive blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);


  // set input fbo to zero, as we use additive blending from now on
  clearInputFbo(ACG::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));


  // emission color pass
  // maybe do check if necessary here, to avoid drawing black screenquad with no effect

  {
    GLSL::Program* passShader = numSamples_ ? progEmissiveMS_ : progEmissive_;

    // setup emissive shader
    passShader->use();
    passShader->setUniform("samplerPos", 0);
    passShader->setUniform("samplerMaterial", 6);

    passShader->setUniform("projParams", ACG::Vec4f(matProj(0,0), matProj(1,1), matProj(2,2), matProj(2,3)) );
    passShader->setUniform("clipPlanes", clipPlanes);

    if (numSamples_) {
      passShader->setUniform("numSamples", numSamples_);
      passShader->setUniform("samplerFilterWeights", 7);

      filterWeightsBuffer_.bind(GL_TEXTURE7);
    }

    materialBuffer_.bind(GL_TEXTURE6);

    for (int i = 1; i >= 0; --i){
      glActiveTexture(GL_TEXTURE0 + i);

      glBindTexture(numSamples_ ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, viewRes->scene_->getAttachment(GL_COLOR_ATTACHMENT0 + i));
    }


    ACG::ScreenQuad::draw(passShader);

    passShader->disable();
  }


  for (int lightID = 0; lightID < numLights_; ++lightID) {
    const LightData* lgt = lights_ + lightID;

    // choose shader based on light type and multisampling
    GLSL::Program* passShader = 0;
    
    if (lgt->ltype == ACG::SG_LIGHT_DIRECTIONAL)
      passShader = numSamples_ ? progDirectionalMS_ : progDirectional_;
    else if (lgt->ltype == ACG::SG_LIGHT_SPOT)
      passShader = numSamples_ ? progSpotMS_ : progSpot_;
    else
      passShader = numSamples_ ? progPointMS_ : progPoint_;

    // setup lighting shader
    passShader->use();
    passShader->setUniform("samplerPos", 0);
    passShader->setUniform("samplerNormal", 1);
    passShader->setUniform("samplerMaterial", 6);

    passShader->setUniform("lightDir", lgt->dir);
    passShader->setUniform("lightPos", lgt->pos);

    passShader->setUniform("lightAtten", lgt->atten);

    if (lgt->ltype == ACG::SG_LIGHT_SPOT) {
      passShader->setUniform("lightSpotDir", lgt->dir);
      passShader->setUniform("lightSpotParam", lgt->spotCutoffExponent);
    }


    passShader->setUniform("lightAmbient", lgt->ambient);
    passShader->setUniform("lightDiffuse", lgt->diffuse);
    passShader->setUniform("lightSpecular", lgt->specular);

    passShader->setUniform("projParams", ACG::Vec4f(matProj(0,0), matProj(1,1), matProj(2,2), matProj(2,3)) );
    passShader->setUniform("clipPlanes", clipPlanes);
    

    if (numSamples_) {
      passShader->setUniform("numSamples", numSamples_);
      passShader->setUniform("samplerFilterWeights", 7);

      filterWeightsBuffer_.bind(GL_TEXTURE7);
    }

    materialBuffer_.bind(GL_TEXTURE6);

    for (int i = 1; i >= 0; --i){
      glActiveTexture(GL_TEXTURE0 + i);

      glBindTexture(numSamples_ ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, viewRes->scene_->getAttachment(GL_COLOR_ATTACHMENT0 + i));
    }


    ACG::ScreenQuad::draw(passShader);

    passShader->disable();
  }

  // draw background color with z-culling
  glDepthFunc(GL_LEQUAL);

  progBackground_->use();
  progBackground_->setUniform("bkColor", _properties.backgroundColor());
  ACG::ScreenQuad::draw(progBackground_);
  progBackground_->disable();



  // reset depth func and blending to opengl default
  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ZERO);
  
  ACG::glCheckErrors();

  // restore common opengl state
  // log window remains hidden otherwise
  finishRenderingPipeline();

#endif

}



void DeferredShading::reloadShaders() {
  const int numShaders = 9;

  GLSL::Program** ptrShaders[numShaders] = 
  {
    &progEmissive_, &progEmissiveMS_,
    &progBackground_,
    &progDirectional_, &progDirectionalMS_, 
    &progPoint_, &progPointMS_,
    &progSpot_, &progSpotMS_
  };

  const char* shaderFiles[numShaders] = 
  {
    PASS_EMISSIVE_FILE, PASS_EMISSIVE_MS_FILE,
    PASS_BACKGROUND_FILE,
    PASS_DIRECTIONAL_FILE, PASS_DIRECTIONAL_MS_FILE,
    PASS_POINT_FILE, PASS_POINT_MS_FILE,
    PASS_SPOT_FILE, PASS_SPOT_MS_FILE,
  };

  for (int i = 0; i < numShaders; ++i){
    GLSL::Program* prog = GLSL::loadProgram(SCREENQUAD_VERTEXSHADER_FILE, shaderFiles[i]); 

    if (prog){
      delete *(ptrShaders[i]);
      *(ptrShaders[i]) = prog;
    }
  }
}

void DeferredShading::ViewerResources::resize( int _newWidth, int _newHeight, int& _numSamples ) {
  if (!_newHeight || !_newWidth) return;

//  if (!scene_)  {
  if (!scene_ || scene_->getMultisamplingCount() != _numSamples)  {
    delete scene_;
    // scene fbo with 2 color attachments + depth buffer
    //  attachment0: view space depth, material id  (RG32F)
    //  attachment1: normals    (r8g8b8 color coded normal texture)
    scene_ = new ACG::FBO();
    scene_->init();
    scene_->setMultisampling(_numSamples, GL_TRUE);

    scene_->attachTexture2D(GL_COLOR_ATTACHMENT0, _newWidth, _newHeight, GL_RG32F, GL_RG);
    scene_->attachTexture2D(GL_COLOR_ATTACHMENT1, _newWidth, _newHeight, GL_RGB, GL_RGB);

//    scene_->attachTexture2DDepth(_newWidth, _newHeight);
    scene_->addDepthBuffer(_newWidth, _newHeight);
  }

  _numSamples = scene_->setMultisampling(_numSamples, GL_TRUE);
  scene_->resize(_newWidth, _newHeight);

}

void DeferredShading::slotMSAASelection( QAction *  _action) {
  if ( _action->text() == "0x MSAA") {
    numSamples_ = 0;
  } else if ( _action->text() == "2x MSAA") {
    numSamples_ = 2;
  } else if ( _action->text() == "4x MSAA") {
    numSamples_ = 4;
  } else if ( _action->text() == "8x MSAA") {
    numSamples_ = 8;
  } else {
    std::cerr << "Error : optionHandling unable to find MSAA selection!!! " << _action->text().toStdString() << std::endl;
    numSamples_ = 0;
  }

  // also reload shaders
  reloadShaders();
}


#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2( deferredshading , DeferredShading );
#endif

