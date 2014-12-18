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

#include "PostProcessorGaussBlur.hh"

#include <ACG/GL/ScreenQuad.hh>
#include <ACG/GL/ShaderCache.hh>
#include <ACG/GL/GLFormatInfo.hh>

#include <QDialog>
#include <QColor>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <ACG/QtWidgets/QtColorChooserButton.hh>



PostProcessorGaussBlur::PostProcessorGaussBlur()
  : blurGauss_(32, 32, 4, 2.0f, GL_RGBA)
{
}


PostProcessorGaussBlur::~PostProcessorGaussBlur()
{
}

QString PostProcessorGaussBlur::checkOpenGL()
{
  if (!ACG::openGLVersion(3,0))
    return QString("SSAO plugin requires OpenGL 3.0!");

  return QString("");
}


void PostProcessorGaussBlur::postProcess( ACG::GLState* _glstate, const std::vector<const PostProcessorInput*>& _input, const PostProcessorOutput& _output )
{
  glBindFramebuffer(GL_FRAMEBUFFER, _output.fbo_);
  glDrawBuffer(_output.drawBuffer_);

  glDepthMask(1);
  glColorMask(1,1,1,1);



  blurGauss_.resizeInput(_input[0]->width, _input[0]->height);

  blurGauss_.execute(_input[0]->colorTex_, 0, 0, 0);
}

QAction* PostProcessorGaussBlur::optionsAction()
{
  QAction * action = new QAction("Gaussian Blur Options" , this );

  connect(action,SIGNAL(triggered( bool )),this,SLOT(optionDialog( bool )));

  return action;
}

void PostProcessorGaussBlur::optionDialog( bool )
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


  label = new QLabel(tr("Sigma [0.1, 20.0]:"));
  layout->addWidget(label);

  QSlider* sigmaSlider = new QSlider(Qt::Horizontal);
  sigmaSlider->setRange(1, 200);
  sigmaSlider->setValue(10);
  sigmaSlider->setTracking(true);
  layout->addWidget(sigmaSlider);

  optionsDlg->setLayout(layout);


  connect(radiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(radiusChanged(int)));
  connect(sigmaSlider, SIGNAL(sliderMoved(int)), this, SLOT(sigmaChanged(int)));
//   connect(outlineColorBtn, SIGNAL(colorChanged(QColor)), this, SLOT(outlineColorChanged(QColor)));


  optionsDlg->show();
}


void PostProcessorGaussBlur::radiusChanged( int _radius )
{
  blurGauss_.setKernel(_radius, blurGauss_.sigma());
}

void PostProcessorGaussBlur::sigmaChanged( int _val )
{
  float sigma = float(_val) * 0.1f;
  blurGauss_.setKernel(blurGauss_.radius(), sigma);
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( postprocessorgaussblurplugin , PostProcessorGaussBlur );
#endif

