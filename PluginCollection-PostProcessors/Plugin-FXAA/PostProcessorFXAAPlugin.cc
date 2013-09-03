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
*   $Revision: 17080 $                                                       *
*   $LastChangedBy: moeller $                                                *
*   $Date: 2013-07-19 12:58:31 +0200 (Fri, 19 Jul 2013) $                     *
*                                                                            *
\*===========================================================================*/

#if QT_VERSION >= 0x050000 
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

#include "PostProcessorFXAAPlugin.hh"

#include <iostream>
#include <ACG/GL/GLState.hh>
#include <ACG/GL/gl.hh>
#include <ACG/GL/ScreenQuad.hh>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/GlobalOptions.hh>


PostProcessorFXAAPlugin::PostProcessorFXAAPlugin() :
fxaa_(0)
{
}

PostProcessorFXAAPlugin::~PostProcessorFXAAPlugin()
{
  delete fxaa_;
}


QString PostProcessorFXAAPlugin::postProcessorName() {
  return QString("FXAA");
}



void PostProcessorFXAAPlugin::postProcess(ACG::GLState* _glstate, const PostProcessorInput& _input, GLuint _targetFBO) {

  // ======================================================================================================
  // Load shader if needed
  // ======================================================================================================
  if (!fxaa_)
    fxaa_ = GLSL::loadProgram("FXAA/screenquad.glsl", "FXAA/fxaa.glsl");

  // ======================================================================================================
  // Bind input texture
  // ======================================================================================================

  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, _input.colorTex_);

  // ======================================================================================================
  // Bind output FBO
  // ======================================================================================================

  glBindFramebuffer(GL_FRAMEBUFFER, _targetFBO);

  // ======================================================================================================
  // Clear rendering buffer
  // ======================================================================================================
   _glstate->clearBuffers();

  // ======================================================================================================
  // Setup render states
  // ======================================================================================================

  glDepthMask(1);
  glColorMask(1,1,1,1);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  // ======================================================================================================
  // Setup shader
  // ======================================================================================================

  fxaa_->use();
  fxaa_->setUniform("textureSampler", 0);

  ACG::Vec2f texcoordOffset(1.0f / float(_input.width), 1.0f / float(_input.height));
  fxaa_->setUniform("texcoordOffset", texcoordOffset);

  // ======================================================================================================
  // Execute
  // ======================================================================================================

  ACG::ScreenQuad::draw(fxaa_);

  
  fxaa_->disable();
}


#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2( postprocessorfxaaplugin , PostProcessorFXAAPlugin );
#endif

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
*   $Revision: 17080 $                                                       *
*   $LastChangedBy: moeller $                                                *
*   $Date: 2013-07-19 12:58:31 +0200 (Fri, 19 Jul 2013) $                     *
*                                                                            *
\*===========================================================================*/

#if QT_VERSION >= 0x050000 
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

#include "PostProcessorFXAAPlugin.hh"

#include <iostream>
#include <ACG/GL/GLState.hh>
#include <ACG/GL/gl.hh>
#include <ACG/GL/ScreenQuad.hh>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/GlobalOptions.hh>


PostProcessorFXAAPlugin::PostProcessorFXAAPlugin() :
sceneTexWidth_(0),
sceneTexHeight_(0),
fxaa_(0)
{

}

PostProcessorFXAAPlugin::~PostProcessorFXAAPlugin()
{
  delete fxaa_;
}


QString PostProcessorFXAAPlugin::postProcessorName() {
  return QString("FXAA");
}



void PostProcessorFXAAPlugin::updateDepthStencilTextureBuffer(ACG::GLState* _glstate) {

  if ( !ACG::checkExtensionSupported("GL_ARB_texture_rectangle") ) {
    std::cerr << "GL_ARB_texture_rectangle not supported! " << std::endl;
    return;
  }

  int vp_l, vp_b, vp_w, vp_h;
  _glstate->get_viewport (vp_l, vp_b, vp_w, vp_h);

  // Does color texture exist?
  if (!pSceneTexture_.is_valid()) {
    // ======================================================================================================
    // creating an 24-bit depth + 8-bit stencil texture
    // ======================================================================================================

    pSceneTexture_.enable();
    pSceneTexture_.bind();
    GLenum texTarget = GL_TEXTURE_2D;
    GLenum texInternalFormat = GL_RGB;
    GLenum texFormat = GL_RGB;
    GLenum texType = GL_UNSIGNED_BYTE;
    GLenum texFilterMode = GL_LINEAR;
    glTexImage2D(texTarget, 0, texInternalFormat, vp_w, vp_h, 0, texFormat, texType, NULL);

    glTexParameterf(texTarget, GL_TEXTURE_MIN_FILTER, texFilterMode);
    glTexParameterf(texTarget, GL_TEXTURE_MAG_FILTER, texFilterMode);
    glTexParameterf(texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  }

  // Resize target texture
  if (_glstate->viewport_width() != sceneTexWidth_ || _glstate->viewport_height() != sceneTexHeight_) {

    pSceneTexture_.bind();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _glstate->viewport_width(), _glstate->viewport_height(), 0,
        GL_RGB, GL_UNSIGNED_BYTE, 0);

    ACG::GLState::bindTexture(GL_TEXTURE_2D, 0);

    sceneTexWidth_  = _glstate->viewport_width();
    sceneTexHeight_ = _glstate->viewport_height();
  }

  ACG::glCheckErrors();

}


void PostProcessorFXAAPlugin::postProcess(ACG::GLState* _glstate) {

  // ======================================================================================================
  // Load shader if needed
  // ======================================================================================================
  if (!fxaa_)
    fxaa_ = GLSL::loadProgram("FXAA/screenquad.glsl", "FXAA/fxaa.glsl");

  // ======================================================================================================
  // Adjust buffer to correct size
  // ======================================================================================================
  updateDepthStencilTextureBuffer(_glstate);

  // ======================================================================================================
  // Get current viewport size
  // ======================================================================================================
  int vp_l, vp_b, vp_w, vp_h;
  _glstate->get_viewport(vp_l, vp_b, vp_w, vp_h);

  // ======================================================================================================
  // Bind color texture
  // ======================================================================================================

  // ======================================================================================================
  // Copy color component of rendered image to texture
  // ======================================================================================================

//  if (!_input || !_input->colorTex_)
  {
    glActiveTexture(GL_TEXTURE0);
    pSceneTexture_.enable();
    pSceneTexture_.bind();

    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vp_l, vp_b, vp_w , vp_h, 0);
  }
/*  else
  {
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _input->colorTex_);
  }
*/
  ACG::glCheckErrors();

  // ======================================================================================================
  // Clear rendering buffer
  // ======================================================================================================
   _glstate->clearBuffers();

  // ======================================================================================================
  // Render a simple quad (rest is done by the texture)
  // ======================================================================================================

  glDepthMask(1);
  glColorMask(1,1,1,1);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  fxaa_->use();
  fxaa_->setUniform("textureSampler", 0);

  ACG::Vec2f texcoordOffset(1.0f / float(vp_w), 1.0f / float(vp_h));
  fxaa_->setUniform("texcoordOffset", texcoordOffset);

  ACG::ScreenQuad::draw(fxaa_);

  // Disable depth stencil buffer
  pSceneTexture_.disable();

}


#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2( postprocessorfxaaplugin , PostProcessorFXAAPlugin );
#endif

