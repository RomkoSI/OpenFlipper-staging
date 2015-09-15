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

#include "PostProcessorBilateralBlur.hh"

#include <ACG/GL/ScreenQuad.hh>
#include <ACG/GL/ShaderCache.hh>
#include <ACG/GL/GLFormatInfo.hh>

#include <QDialog>
#include <QColor>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <ACG/QtWidgets/QtColorChooserButton.hh>



PostProcessorBilateralBlur::PostProcessorBilateralBlur()
  : blur_(32, 32, 4, 2.0f, 2.0f, GL_RGBA)
{
}


PostProcessorBilateralBlur::~PostProcessorBilateralBlur()
{
}

QString PostProcessorBilateralBlur::checkOpenGL()
{
  if (!ACG::openGLVersion(3,0))
    return QString("Bilateral blur plugin requires OpenGL 3.0!");

  return QString("");
}


void PostProcessorBilateralBlur::postProcess( ACG::GLState* _glstate, const std::vector<const PostProcessorInput*>& _input, const PostProcessorOutput& _output )
{
  glBindFramebuffer(GL_FRAMEBUFFER, _output.fbo_);
  glDrawBuffer(_output.drawBuffer_);

  glDepthMask(1);
  glColorMask(1,1,1,1);



  blur_.resizeInput(_input[0]->width, _input[0]->height);


  blur_.setParams(_glstate->projection(), _input[0]->depthTex_);


  blur_.execute(_input[0]->colorTex_, 0, 0, 0);
}

QAction* PostProcessorBilateralBlur::optionsAction()
{
  QAction * action = new QAction("Gaussian Blur Options" , this );

  connect(action,SIGNAL(triggered( bool )),this,SLOT(optionDialog( bool )));

  return action;
}

void PostProcessorBilateralBlur::optionDialog( bool )
{
  //generate widget
  QDialog* optionsDlg = new QDialog();
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setAlignment(Qt::AlignTop);

  QColor curColor;
  curColor.setRgbF(0.3f, 0.2f, 0.7f);

  QLabel* label = new QLabel(tr("Radius [1, 32]:"));
  layout->addWidget(label);

  QSlider* radiusSlider = new QSlider(Qt::Horizontal);
  radiusSlider->setRange(1, 32);
  radiusSlider->setValue(8);
  radiusSlider->setTracking(true);
  layout->addWidget(radiusSlider);


  label = new QLabel(tr("spatial sigma [0.1, 20.0]:"));
  layout->addWidget(label);

  QSlider* sigmaSlider = new QSlider(Qt::Horizontal);
  sigmaSlider->setRange(1, 200);
  sigmaSlider->setValue(10);
  sigmaSlider->setTracking(true);
  layout->addWidget(sigmaSlider);


  label = new QLabel(tr("linear depth sigma [0.1, 20.0]:"));
  layout->addWidget(label);

  QSlider* sigmaRSlider = new QSlider(Qt::Horizontal);
  sigmaRSlider->setRange(1, 200);
  sigmaRSlider->setValue(10);
  sigmaRSlider->setTracking(true);
  layout->addWidget(sigmaRSlider);


  optionsDlg->setLayout(layout);


  connect(radiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(radiusChanged(int)));
  connect(sigmaSlider, SIGNAL(sliderMoved(int)), this, SLOT(sigmaSChanged(int)));
  connect(sigmaRSlider, SIGNAL(sliderMoved(int)), this, SLOT(sigmaRChanged(int)));


  optionsDlg->show();
}


void PostProcessorBilateralBlur::radiusChanged( int _radius )
{
  ACG::Vec2f sigma = blur_.sigma();
  blur_.setKernel(_radius, sigma[0], sigma[1]);
}

void PostProcessorBilateralBlur::sigmaSChanged( int _val )
{
  ACG::Vec2f sigma = blur_.sigma();
  sigma[0] = float(_val) * 0.1f;
  blur_.setKernel(blur_.radius(), sigma[0], sigma[1]);
}


void PostProcessorBilateralBlur::sigmaRChanged( int _val )
{
  ACG::Vec2f sigma = blur_.sigma();
  sigma[1] = float(_val) * 0.1f;
  blur_.setKernel(blur_.radius(), sigma[0], sigma[1]);
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( postprocessorbilateralblurplugin , PostProcessorBilateralBlur );
#endif

