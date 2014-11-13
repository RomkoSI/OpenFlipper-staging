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


#include <ACG/GL/acg_glew.hh>

#include "dof.hh"

#include <ACG/GL/ScreenQuad.hh>
#include <ACG/GL/ShaderCache.hh>
//#include <ACG/GL/Debug.hh>

#include "sat.hh"

using namespace ACG;


PostProcessorDoF::PostProcessorDoF()
  : plan2D_(0)
{

}

PostProcessorDoF::~PostProcessorDoF()
{
  delete plan2D_;
}

QString PostProcessorDoF::checkOpenGL()
{
  if (!ACG::openGLVersion(4,3))
    return QString("Depth of Field plugin requires OpenGL 4.3!");

  return QString("");
}


void PostProcessorDoF::postProcess( ACG::GLState* _glstate, const std::vector<const PostProcessorInput*>& _input, const PostProcessorOutput& _output )
{
  // ======================================================================================================
  // Setup render states
  // ======================================================================================================

  glDepthMask(0);
  glColorMask(0,0,0,0);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);


  // ======================================================================================================
  // Convert to rgba32f
  // ======================================================================================================

  int w = _input[0]->width;
  int h = _input[0]->height;

  GLSL::Program* imgConvF = ACG::ShaderCache::getInstance()->getProgram("ScreenQuad/screenquad.glsl", "DoF/conv.glsl");

  if (imgConvF)
  {
    if (scene32F_.getWidth() != w || scene32F_.getHeight() != h)
    {
      scene32F_.del();
      scene32F_.setStorage(1, GL_RGBA32F, w, h);
      scene32F_.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      scene32F_.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      sat32F_.del();
      sat32F_.setStorage(1, GL_RGBA32F, w, h);
      sat32F_.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      sat32F_.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    scene32F_.bindAsImage(0, GL_READ_WRITE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _input[0]->colorTex_);

    ACG::ScreenQuad::draw(imgConvF);
  }

  if (!plan2D_ || plan2D_->getRowPlan()->width() != w || plan2D_->getRowPlan()->height() != h)
  {
    delete plan2D_;
    plan2D_ = new SATPlan(w,h, GL_RGBA32F, 128);
  }
  plan2D_->execute(&scene32F_, &sat32F_);


  // ======================================================================================================
  // Bind output FBO
  // ======================================================================================================

  glBindFramebuffer(GL_FRAMEBUFFER, _output.fbo_);
  glDrawBuffer(_output.drawBuffer_);

  glDepthMask(1);
  glColorMask(1,1,1,1);


  GLSL::Program* progDoF = ACG::ShaderCache::getInstance()->getProgram("ScreenQuad/screenquad.glsl", "DoF/dof.glsl");

  if (progDoF)
  {
    progDoF->use();
    progDoF->setUniform("g_P", _input[0]->proj_);

    ACG::Vec2f clipPlanes = _input[0]->proj_.extract_planes();
    progDoF->setUniform("g_ClipPlanes", clipPlanes);
    printf("n/f: %f %f \n", clipPlanes[0], clipPlanes[1]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _input[0]->depthTex_);

    scene32F_.bind(GL_TEXTURE2);

    sat32F_.bind(GL_TEXTURE0);



    ScreenQuad::draw(progDoF);
  }
  else
    ScreenQuad::drawTexture2D(_input[0]->colorTex_);
}



#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( postprocessordofplugin , PostProcessorDoF );
#endif

