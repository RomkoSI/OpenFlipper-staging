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
*   $Revision: 13354 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2012-01-12 13:39:10 +0100 (Do, 12 Jan 2012) $                     *
*                                                                            *
\*===========================================================================*/

#pragma once

#include <QObject>
#include <QtGui>

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/AboutInfoInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>
#include <OpenFlipper/BasePlugin/ScriptInterface.hh>

#include "PoissonToolbox.hh"

class PoissonPlugin : public QObject, BaseInterface, ToolboxInterface, LoadSaveInterface, LoggingInterface, AboutInfoInterface, ScriptInterface
{
Q_OBJECT
Q_INTERFACES(BaseInterface)
Q_INTERFACES(ToolboxInterface)
Q_INTERFACES(LoggingInterface)
Q_INTERFACES(LoadSaveInterface)
Q_INTERFACES(AboutInfoInterface)
Q_INTERFACES(ScriptInterface)


  //BaseInterface
signals:
  void updateView();

  //LoggingInterface:
  void log( Logtype _type, QString _message );
  void log( QString _message );
  
  // Load/Save Interface
  void addEmptyObject (DataType _type, int& _id);
  void deleteObject( int _id );

  // ToolboxInterface
  //void addToolbox( QString _name  , QWidget* _widget, QIcon* _icon );

  void addToolbox( QString  _name  , QWidget* _widget );

  //AboutInfoInterface
  void addAboutInfo(QString _text, QString _tabName );

  //ScriptInterface
  void setSlotDescription(QString     _slotName,   QString     _slotDescription,
                              QStringList _parameters, QStringList _descriptions);

private slots:

  // BaseInterface
  void initializePlugin();
  void pluginsInitialized();
  
private slots:

  void slotPoissonReconstruct();

public slots:

  void poissionReconstruct(int _id);

  void poissionReconstruct(IdList _ids);

public :
  PoissonPlugin();
  ~PoissonPlugin() {};

  QString name() { return (QString("Poisson Reconstruction Plugin")); };
  QString description( ) { return (QString("Poisson reconstruction based on the Code by Michael Kazhdan and Matthew Bolitho")); };

private :
  PoissonToolBox* tool_;
  QIcon* toolIcon_;


public slots:
  QString version() { return QString("1.0"); };
};

