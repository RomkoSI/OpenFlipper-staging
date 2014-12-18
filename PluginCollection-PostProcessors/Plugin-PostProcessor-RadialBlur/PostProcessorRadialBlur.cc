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

#include "PostProcessorRadialBlur.hh"

#include <ACG/GL/ScreenQuad.hh>
#include <ACG/GL/ShaderCache.hh>
#include <ACG/GL/GLFormatInfo.hh>

#include <QDialog>
#include <QColor>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <ACG/QtWidgets/QtColorChooserButton.hh>



PostProcessorRadialBlur::PostProcessorRadialBlur()
  : blur_(16), radius_(1.0f), intensity_(0.0025f), center_(0.5f, 0.5f)
{
}


PostProcessorRadialBlur::~PostProcessorRadialBlur()
{
}

QString PostProcessorRadialBlur::checkOpenGL()
{
  if (!ACG::openGLVersion(3,0))
    return QString("Radial Blur plugin requires OpenGL 3.0!");

  return QString("");
}


void PostProcessorRadialBlur::postProcess( ACG::GLState* _glstate, const std::vector<const PostProcessorInput*>& _input, const PostProcessorOutput& _output )
{
  glBindFramebuffer(GL_FRAMEBUFFER, _output.fbo_);
  glDrawBuffer(_output.drawBuffer_);

  glDepthMask(1);
  glColorMask(1,1,1,1);


  blur_.execute(_input[0]->colorTex_, radius_, intensity_, center_);
}

QAction* PostProcessorRadialBlur::optionsAction()
{
  QAction * action = new QAction("Radial Blur Options" , this );

  connect(action,SIGNAL(triggered( bool )),this,SLOT(optionDialog( bool )));

  return action;
}

void PostProcessorRadialBlur::optionDialog( bool )
{
  //generate widget
  QDialog* optionsDlg = new QDialog();
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setAlignment(Qt::AlignTop);

  QColor curColor;
  curColor.setRgbF(0.3f, 0.2f, 0.7f);

  QLabel* label = new QLabel(tr("Samples [1, 32]:"));
  layout->addWidget(label);

  QSlider* radiusSlider = new QSlider(Qt::Horizontal);
  radiusSlider->setRange(1, 32);
  radiusSlider->setValue(8);
  radiusSlider->setTracking(true);
  layout->addWidget(radiusSlider);


  label = new QLabel(tr("Radius [0, 2]:"));
  layout->addWidget(label);

  QSlider* sigmaSlider = new QSlider(Qt::Horizontal);
  sigmaSlider->setRange(1, 100);
  sigmaSlider->setValue(50);
  sigmaSlider->setTracking(true);
  layout->addWidget(sigmaSlider);


  label = new QLabel(tr("Intensity [0.001, 0.01]:"));
  layout->addWidget(label);

  QSlider* intensitySlider = new QSlider(Qt::Horizontal);
  intensitySlider->setRange(0, 100);
  intensitySlider->setValue(2);
  intensitySlider->setTracking(true);
  layout->addWidget(intensitySlider);


  label = new QLabel(tr("CenterU [0, 1]:"));
  layout->addWidget(label);

  QSlider* centerUSlider = new QSlider(Qt::Horizontal);
  centerUSlider->setRange(0, 100);
  centerUSlider->setValue(50);
  centerUSlider->setTracking(true);
  layout->addWidget(centerUSlider);


  label = new QLabel(tr("CenterV [0, 1]:"));
  layout->addWidget(label);

  QSlider* centerVSlider = new QSlider(Qt::Horizontal);
  centerVSlider->setRange(0, 100);
  centerVSlider->setValue(50);
  centerVSlider->setTracking(true);
  layout->addWidget(centerVSlider);



  optionsDlg->setLayout(layout);


  connect(radiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(samplesChanged(int)));
  connect(sigmaSlider, SIGNAL(sliderMoved(int)), this, SLOT(radiusChanged(int)));
  connect(intensitySlider, SIGNAL(sliderMoved(int)), this, SLOT(intensityChanged(int)));
  connect(centerUSlider, SIGNAL(sliderMoved(int)), this, SLOT(centerXChanged(int)));
  connect(centerVSlider, SIGNAL(sliderMoved(int)), this, SLOT(centerYChanged(int)));



  optionsDlg->show();
}


void PostProcessorRadialBlur::samplesChanged( int _n )
{
  blur_.setKernel(_n);
}

void PostProcessorRadialBlur::radiusChanged( int _r )
{
  radius_ = float(_r) * 0.01f * 2.0f;
}

void PostProcessorRadialBlur::intensityChanged( int _r )
{
  float t = float(_r) * 0.01f;
  intensity_ = 0.001f * (1.0f - t) + 0.01 * t;
}

void PostProcessorRadialBlur::centerXChanged( int _r )
{
  center_[0] = float(_r) * 0.01f;
}

void PostProcessorRadialBlur::centerYChanged( int _r )
{
  center_[1] = float(_r) * 0.01f;
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( postprocessorradialblurplugin , PostProcessorRadialBlur );
#endif

