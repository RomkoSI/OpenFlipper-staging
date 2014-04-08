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
*   $Revision: 10745 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2011-01-26 10:23:50 +0100 (Mi, 26 Jan 2011) $                     *
*                                                                            *
\*===========================================================================*/


#include "SkyDomeNode.hh"
#include <ACG/GL/gl.hh>
#include <iostream>
#include <ObjectTypes/SkyDome/SkyDomeType.hh>
#include <OpenFlipper/common/GlobalOptions.hh>

#undef QT_NO_OPENGL
#include <QGLWidget>
#define QT_NO_OPENGL

//== IMPLEMENTATION ==========================================================


SkyDomeNode::SkyDomeNode(SkyDome& _dome, BaseNode *_parent, std::string _name)
:BaseNode(_parent, _name),
 dome_(_dome),
 vbo_(0),
 textureId_(0),
 updateBuffers_(true)
{
  vertexDecl_.addElement(GL_FLOAT, 3, ACG::VERTEX_USAGE_POSITION);
}

SkyDomeNode::~SkyDomeNode()
{
  if ( vbo_)
    glDeleteBuffers(1,&vbo_);

}

void SkyDomeNode::boundingBox(ACG::Vec3d& _bbMin, ACG::Vec3d& _bbMax)
{

//  ACG::Vec3d pos = plane_.position - plane_.xDirection * 0.5 - plane_.yDirection * 0.5;
//
//  //add a little offset in normal direction
//  ACG::Vec3d pos0 = ACG::Vec3d( pos + plane_.normal * 0.1 );
//  ACG::Vec3d pos1 = ACG::Vec3d( pos - plane_.normal * 0.1 );
//
//  ACG::Vec3d xDird = ACG::Vec3d( plane_.xDirection );
//  ACG::Vec3d yDird = ACG::Vec3d( plane_.yDirection );
//
//  _bbMin.minimize( pos0 );
//  _bbMin.minimize( pos0 + xDird);
//  _bbMin.minimize( pos0 + yDird);
//  _bbMin.minimize( pos0 + xDird + yDird);
//  _bbMax.maximize( pos1 );
//  _bbMax.maximize( pos1 + xDird);
//  _bbMax.maximize( pos1 + yDird);
//  _bbMax.maximize( pos1 + xDird + yDird);
}

//----------------------------------------------------------------------------

ACG::SceneGraph::DrawModes::DrawMode
SkyDomeNode::availableDrawModes() const
{
  return ( ACG::SceneGraph::DrawModes::SOLID_FLAT_SHADED |
           ACG::SceneGraph::DrawModes::SOLID_SMOOTH_SHADED );
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

void SkyDomeNode::drawSkyDome( ACG::GLState&  _state) {

//  const ACG::Vec3d xy = plane_.xDirection + plane_.yDirection;
//
//  // Array of coordinates for the plane
//  float vboData_[9 * 3 ] = { 0.0,0.0,0.0,
//                            (float)plane_.xDirection[0],(float)plane_.xDirection[1],(float)plane_.xDirection[2],
//                            (float)xy[0],(float)xy[1],(float)xy[2],
//                            (float)plane_.yDirection[0],(float)plane_.yDirection[1],(float)plane_.yDirection[2],
//                            0.0,0.0,0.0,
//                            (float)plane_.yDirection[0],(float)plane_.yDirection[1],(float)plane_.yDirection[2],
//                            (float)xy[0],(float)xy[1],(float)xy[2],
//                            (float)plane_.xDirection[0],(float)plane_.xDirection[1],(float)plane_.xDirection[2],
//                            0.0,0.0,0.0 };
//
//   // Enable the arrays
//  _state.enableClientState(GL_VERTEX_ARRAY);
//  _state.vertexPointer(3,GL_FLOAT,0,&vboData_[0]);
//
//  //first draw the lines
//  _state.set_color(ACG::Vec4f(1.0, 1.0, 1.0 , 1.0) );
//  glLineWidth(2.0);
//
//  glDrawArrays(GL_LINE_STRIP,0,5);
//
//  glLineWidth(1.0);
//
//  // Remember blending state
//  bool blending = _state.blending();
//
//  //then the red front side
//  ACG::GLState::enable (GL_BLEND);
//  ACG::GLState::blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//  _state.set_color(ACG::Vec4f( 0.6, 0.15, 0.2, 0.5));
//  glDrawArrays(GL_QUADS,0,4);
//
//
//  //finally the green back side
//  _state.set_color(ACG::Vec4f(0.1, 0.8, 0.2, 0.5 ));
//
//  glDrawArrays(GL_QUADS,4,4);
//
//  if ( !blending )
//    ACG::GLState::disable(GL_BLEND);
//
//  // deactivate vertex arrays after drawing
//  _state.disableClientState(GL_VERTEX_ARRAY);

}

//----------------------------------------------------------------

void SkyDomeNode::drawSkyDomePick( ACG::GLState&  _state) {

//  _state.pick_set_maximum(1);
//  _state.pick_set_name(0);
//
//  const ACG::Vec3d xy = plane_.xDirection + plane_.yDirection;
//
//  // Array of coordinates for the plane
//  float vboData_[4* 3 ] = { 0.0,0.0,0.0,
//                            (float)plane_.xDirection[0],(float)plane_.xDirection[1],(float)plane_.xDirection[2],
//                            (float)xy[0],(float)xy[1],(float)xy[2],
//                            (float)plane_.yDirection[0],(float)plane_.yDirection[1],(float)plane_.yDirection[2] };
//
//   // Enable the arrays
//  _state.enableClientState(GL_VERTEX_ARRAY);
//  _state.vertexPointer(3,GL_FLOAT,0,&vboData_[0]);
//
//  glDrawArrays(GL_QUADS,0,4);
//
//  // deactivate vertex arrays after drawing
//  _state.disableClientState(GL_VERTEX_ARRAY);

}

//----------------------------------------------------------------

void SkyDomeNode::draw(ACG::GLState&  _state  , const ACG::SceneGraph::DrawModes::DrawMode& /*_drawMode*/)
{

//  _state.push_modelview_matrix();
//  glPushAttrib(GL_COLOR_BUFFER_BIT);
//  glPushAttrib(GL_LIGHTING_BIT);
//
//  glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
//  ACG::GLState::enable(GL_COLOR_MATERIAL);
//
//  // plane_.position represents the center of the plane.
//  // Compute the corner position
//  ACG::Vec3d pos = plane_.position - plane_.xDirection*0.5 - plane_.yDirection*0.5;
//
//  // translate to corner position
//  _state.translate(pos[0], pos[1], pos[2]);
//
//  // draw the plane
//  drawPlane(_state);
//
//  glPopAttrib();
//  glPopAttrib();
//  _state.pop_modelview_matrix();
}


//----------------------------------------------------------------

void
SkyDomeNode::pick(ACG::GLState& _state, ACG::SceneGraph::PickTarget _target)
{
//  if (_target == ACG::SceneGraph::PICK_ANYTHING) {
//
//    _state.push_modelview_matrix();
//
//    ACG::Vec3d pos = plane_.position - plane_.xDirection*0.5 - plane_.yDirection*0.5;
//
//    _state.translate(pos[0], pos[1], pos[2]);
//
//    drawPlanePick(_state);
//
//    _state.pop_modelview_matrix();
//  }
}

//----------------------------------------------------------------

void
SkyDomeNode::
getRenderObjects(ACG::IRenderer* _renderer, ACG::GLState&  _state , const ACG::SceneGraph::DrawModes::DrawMode&  _drawMode , const ACG::SceneGraph::Material* _mat) {

  if ( updateBuffers_ ) {

    QFileInfo info(dome_.textureFileName());

    QString filename = dome_.textureFileName();

    // Fallback image
    if ( !info.exists() ) {
      std::cerr << "Did not find image!" << filename.toStdString() << std::endl;
      filename = OpenFlipper::Options::iconDirStr() + QDir::separator() + "EmptySkyDome.png";

    }

    // Test: Load texture image
    QImage texture(filename);

    GLuint textureId;

    QImage GL_formatted_image;
    GL_formatted_image = QGLWidget::convertToGLFormat(texture);

    if( GL_formatted_image.isNull() )
    {
      std::cerr << "error GL_formatted_image" << std::endl ;
    }

    //generate the texture name
    if ( !textureId_ )
      glGenTextures(1, &textureId_);

    //bind the texture ID
    glBindTexture(GL_TEXTURE_2D, textureId_);

    //generate the texture
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, GL_formatted_image.width(),
                  GL_formatted_image.height(),
                  0, GL_RGBA, GL_UNSIGNED_BYTE, GL_formatted_image.bits() );

    //texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D,0);

    updateBuffers_ = false;
  }

  // init base render object
  ACG::RenderObject ro;

  // We simply take the data from the texture! -> No Lighting
  _state.disable(GL_LIGHTING);
  ro.initFromState(&_state);
  ro.debugName = "SkyDome";
  ro.priority = 100;

  ACG::RenderObject::Texture tex;
  tex.id = textureId_;

  ro.addTexture(tex);

  // Render with depth test enabled
  ro.depthTest = true;
  ro.blending  = false;

  // Compute a screen aligned quad ( Depth doesn't matter, as we compute depth in shader anyway, but we set it to the far plane)
  ACG::Vec3d bottomLeft ( 0.0                     , 0.0                      , 0.99 );
  ACG::Vec3d bottomRight( _state.viewport_width() , 0.0                      , 0.99 );
  ACG::Vec3d topLeft    ( 0.0                     , _state.viewport_height() , 0.99 );
  ACG::Vec3d topRight   ( _state.viewport_width() , _state.viewport_height() , 0.99 );

  ACG::Vec3f unprojectedBottomLeft  = ACG::Vec3f(_state.unproject(bottomLeft));
  ACG::Vec3f unprojectedBottomRight = ACG::Vec3f(_state.unproject(bottomRight));
  ACG::Vec3f unprojectedtopLeft     = ACG::Vec3f(_state.unproject(topLeft));
  ACG::Vec3f unprojectedtopRight    = ACG::Vec3f(_state.unproject(topRight));

  // Array of coordinates for the quad
  float vboData_[4 * 3 * 4 ] = { unprojectedBottomLeft[0]  , unprojectedBottomLeft[1]  , unprojectedBottomLeft[2],
                                 unprojectedBottomRight[0] , unprojectedBottomRight[1] , unprojectedBottomRight[2],
                                 unprojectedtopRight[0]    , unprojectedtopRight[1]    , unprojectedtopRight[2],
                                 unprojectedtopLeft[0]     , unprojectedtopLeft[1]     , unprojectedtopLeft[2]
                               };

  if ( ! vbo_ ) {
    glGenBuffersARB(1, &vbo_);
  }

  // Bind buffer
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_);

  // Upload to buffer ( 4 vertices with 3 coordinates for point and normal of 4 byte sized floats)
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4 * 3 * 4, &vboData_[0], GL_STATIC_DRAW_ARB);

  // Set the buffers for rendering
  ro.shaderDesc.shadeMode            = ACG::SG_SHADE_UNLIT;
  ro.shaderDesc.vertexTemplateFile   = OpenFlipper::Options::shaderDirStr() + QDir::separator() + "SkyDome" + QDir::separator() + "SkyDomeVertexShader.glsl";
  ro.shaderDesc.fragmentTemplateFile = OpenFlipper::Options::shaderDirStr() + QDir::separator() + "SkyDome" + QDir::separator() + "SkyDomeFragmentShader.glsl";
  ro.vertexBuffer                    = vbo_;
  ro.vertexDecl                      = &vertexDecl_;

  ACG::SceneGraph::Material localMaterial = *_mat;
  localMaterial.baseColor(ACG::Vec4f(1.0,0.0,0.0,1.0) );

  ro.setMaterial(&localMaterial);
  ro.glDrawArrays(GL_QUADS, 0, 4);

  ro.setUniform("uUpperCutOff"   , dome_.topOffset() );
  ro.setUniform("uHorizontalFOV" , dome_.horizontalFOV() );
  ro.setUniform("uVerticalFOV"   , dome_.verticalFOV() );

  ro.setUniform("uVP_width"   , float(_state.viewport_width()) );
  ro.setUniform("uVP_height"  , float(_state.viewport_height()) );

  _renderer->addRenderObject(&ro);
}


//=============================================================================
