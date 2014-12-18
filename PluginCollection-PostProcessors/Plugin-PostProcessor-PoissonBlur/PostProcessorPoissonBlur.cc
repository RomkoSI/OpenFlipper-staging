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

#include "PostProcessorPoissonBlur.hh"

#include <ACG/GL/ScreenQuad.hh>
#include <ACG/GL/ShaderCache.hh>
#include <ACG/GL/GLFormatInfo.hh>

#include <QDialog>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QPushButton>


PostProcessorPoissonBlur::PostProcessorPoissonBlur()
  : blur_(new ACG::PoissonBlurFilter(1.0f, 0.5f)), radius_(1.0f), minDist_(0.5f)
{
}


PostProcessorPoissonBlur::~PostProcessorPoissonBlur()
{
  delete blur_;
}

QString PostProcessorPoissonBlur::checkOpenGL()
{
  if (!ACG::openGLVersion(3,0))
    return QString("SSAO plugin requires OpenGL 3.0!");

  return QString("");
}


void PostProcessorPoissonBlur::postProcess( ACG::GLState* _glstate, const std::vector<const PostProcessorInput*>& _input, const PostProcessorOutput& _output )
{
  glBindFramebuffer(GL_FRAMEBUFFER, _output.fbo_);
  glDrawBuffer(_output.drawBuffer_);

  glDepthMask(1);
  glColorMask(1,1,1,1);



  float scale = 1.0f / std::min(_input[0]->width, _input[0]->height);

  blur_->execute(_input[0]->colorTex_, scale);
}

QAction* PostProcessorPoissonBlur::optionsAction()
{
  QAction * action = new QAction("Gaussian Blur Options" , this );

  connect(action,SIGNAL(triggered( bool )),this,SLOT(optionDialog( bool )));

  return action;
}

void PostProcessorPoissonBlur::optionDialog( bool )
{
  //generate widget
  QDialog* optionsDlg = new QDialog();
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setAlignment(Qt::AlignTop);

  QColor curColor;
  curColor.setRgbF(0.3f, 0.2f, 0.7f);

  QLabel* label = new QLabel(tr("Radius [0, 32]:"));
  layout->addWidget(label);

  QSlider* radiusSlider = new QSlider(Qt::Horizontal);
  radiusSlider->setRange(1, 32);
  radiusSlider->setValue(1);
  radiusSlider->setTracking(true);
  layout->addWidget(radiusSlider);


  label = new QLabel(tr("MinDistance [0.1, 10.0]:"));
  layout->addWidget(label);

  QSlider* minDistSlider = new QSlider(Qt::Horizontal);
  minDistSlider->setRange(1, 100);
  minDistSlider->setValue(5);
  minDistSlider->setTracking(true);
  layout->addWidget(minDistSlider);


  QPushButton* btn = new QPushButton("Recompute");
  layout->addWidget(btn);

  optionsDlg->setLayout(layout);


  connect(btn, SIGNAL(clicked()), this, SLOT(recompute()));
  connect(radiusSlider, SIGNAL(sliderMoved(int)), this, SLOT(radiusChanged(int)));
  connect(minDistSlider, SIGNAL(sliderMoved(int)), this, SLOT(minDistChanged(int)));


  optionsDlg->show();
}


void PostProcessorPoissonBlur::recompute(  )
{
  delete blur_;

  blur_ = new ACG::PoissonBlurFilter(radius_, minDist_);
}

void PostProcessorPoissonBlur::radiusChanged(int r)
{
  radius_ = float(r);
}

void PostProcessorPoissonBlur::minDistChanged(int d)
{
  minDist_ = float(d) * 0.1f;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2( postprocessorpoissonblurplugin , PostProcessorPoissonBlur );
#endif

