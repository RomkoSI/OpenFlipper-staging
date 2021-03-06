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
*   $Revision: 13354 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2012-01-12 13:39:10 +0100 (Do, 12 Jan 2012) $                     *
*                                                                            *
\*===========================================================================*/

#pragma once

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>

#include "PanoramaToolbox.hh"

#include <QObject>
#include <QtGui>



class PanoramaControlPlugin : public QObject, BaseInterface, ToolboxInterface, LoggingInterface, LoadSaveInterface
{
Q_OBJECT
Q_INTERFACES(BaseInterface)
Q_INTERFACES(ToolboxInterface)
Q_INTERFACES(LoggingInterface)
Q_INTERFACES(LoadSaveInterface)

  #if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.OpenFlipper.Plugins.Plugin-PanoramaControl")
  #endif

signals:

  //BaseInterface
  void updateView();
  void updatedObject(int _identifier, const UpdateType& _type);
  void setSlotDescription(QString     _slotName,   QString     _slotDescription,
                          QStringList _parameters, QStringList _descriptions);

  void setRenderer(unsigned int _viewer, QString _rendererName);

  //LoggingInterface:
  void log( Logtype _type, QString _message );
  void log( QString _message );
  
  // ToolboxInterface
  void addToolbox( QString _name  , QWidget* _widget , QIcon* _icon );

  // Load/Save Interface
  void addEmptyObject (DataType _type, int& _id);


private slots:

  // BaseInterface
  void initializePlugin();
  
private slots:

  /// Button slot iterating over all targets and passing them to the correct functions
  void slotLoadImage();

  /// Spinboxes changed
  void slotValuesChanged(double _unused);

public :
  PanoramaControlPlugin();
  ~PanoramaControlPlugin() {};

  QString name() { return (QString("Panorama Control Plugin")); };
  QString description( ) { return (QString("Visualize panorama images via a sky dome")); };

private :
  PanoramaToolBox* tool_;


public slots:
  QString version() { return QString("1.0"); };
};

