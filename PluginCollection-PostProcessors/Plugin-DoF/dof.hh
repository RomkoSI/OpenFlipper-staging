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


#include <QObject>

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/PostProcessorInterface.hh>

#include <ACG/GL/globjects.hh>
#include <ACG/ShaderUtils/GLSLShader.hh>
#include "sat.hh"


class PostProcessorDoF : public QObject, BaseInterface, PostProcessorInterface
{
  Q_OBJECT
    Q_INTERFACES(BaseInterface)
    Q_INTERFACES(PostProcessorInterface)

#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.OpenFlipper.Plugins.Plugin-PostProcessorDoF")
#endif

public:
  PostProcessorDoF();
  ~PostProcessorDoF();

public :
  QString name() { return (QString("Depth of Field Postprocessor Plugin")); };
  QString description( ) { return (QString(tr("Approximate depth of field via SAT"))); };


  public slots:
    QString version() { return QString("1.0"); };

    private slots:

      void postProcess(ACG::GLState* _glstate, const std::vector<const PostProcessorInput*>& _input, const PostProcessorOutput& _output);

      QString postProcessorName() {return QString("Depth of Field");}

      QString checkOpenGL();

private:

  ACG::TextureBuffer testBuffer_;
  ACG::TextureBuffer testBufferOut_;

  ACG::Texture2D testImg2D_;
  ACG::Texture2D testImg2DOut_;

  ACG::Texture2D scene32F_;
  ACG::Texture2D sat32F_;


  SATPlan* plan2D_;

  // prefixsum of individual blocksums
  ACG::TextureBuffer blockSums_;
};

