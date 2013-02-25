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
*   $Date: 2011-01-26 10:23:50 +0100 (Mi, 26. Jan 2011) $                     *
*                                                                            *
\*===========================================================================*/

//=============================================================================
//
//  SkyDome Object
//
//=============================================================================

//== INCLUDES =================================================================

#include <OpenFlipper/common/Types.hh>
#include "SkyDomeObject.hh"
#include "SkyDome.hh"

//== DEFINES ==================================================================

//== TYPEDEFS =================================================================

//== CLASS DEFINITION =========================================================

/** Constructor for SkyDome Objects. This object class gets a Separator Node giving
*  the root node to which it should be connected. The plane is generated internally
*  and all nodes for visualization will be added below the scenegraph node.\n
*  You don't need to create an object of this type manually. Use
*  PluginFunctions::addSkyDome instead. ( see Types.hh::DataType )
*/
SkyDomeObject::SkyDomeObject( ) :
  BaseObjectData( ),
  skyDomeNode_(NULL)
{
  setDataType(DATA_SKYDOME);
  init();
}

//=============================================================================


/**
 * Copy Constructor - generates a copy of the given object
 */
SkyDomeObject::SkyDomeObject(SkyDomeObject & _object) :
  BaseObjectData(_object)
{

    init( &_object.skyDome_ );

    setName( name() );
}

/** Destructor for SkyDome Objects. The destructor deletes the Line and all
*  Scenegraph nodes associated with the SkyDome or the object.
*/
SkyDomeObject::~SkyDomeObject()
{
  // Delete the data attached to this object ( this will remove all perObject data)
  // Not the best way to do it but it will work.
  // This is only necessary if people use references to the plane below and
  // they do something with the plane in the destructor of their
  // perObjectData.
  deleteData();

  // No need to delete the scenegraph Nodes as this will be managed by base plugin
  skyDomeNode_    = NULL;
}

/** Cleanup Function for SkyDome Objects. Deletes the contents of the whole object and
* calls SkyDomeObject::init afterwards.
*/
void SkyDomeObject::cleanup() {

  BaseObjectData::cleanup();

  skyDomeNode_   = NULL;

  setDataType(DATA_SKYDOME);

  init();

}

/**
 * Generate a copy
 */
BaseObject* SkyDomeObject::copy() {
    SkyDomeObject* object = new SkyDomeObject(*this);
    return dynamic_cast< BaseObject* >(object);
}

/** This function initializes the plane object. It creates the scenegraph nodes.
*/
void SkyDomeObject::init( SkyDome* _skyDome) {

  if ( materialNode() == NULL)
    std::cerr << "Error when creating SkyDome Object! materialNode is NULL!" << std::endl;

  skyDomeNode_ = new SkyDomeNode( skyDome_, materialNode() , "NEW SkyDomeNode" );

  if (_skyDome){
    skyDome_ = *_skyDome;
  } else {
    // Leave at default values
  }
  
}

// ===============================================================================
// Name/Path Handling
// ===============================================================================

/** Set the name of an object. All Scenegraph nodes are renamed too. It also calls
* BaseObjectData::setName.
*/
void SkyDomeObject::setName( QString _name ) {
  BaseObjectData::setName(_name);

  std::string nodename = std::string("SkyDomeNode for SkyDome "     + _name.toUtf8() );
  skyDomeNode_->name( nodename );
}

// ===============================================================================
// Data
// ===============================================================================

void SkyDomeObject::update(UpdateType _type) {
  skyDomeNode_->update();
}

// ===============================================================================
// Visualization
// ===============================================================================

SkyDomeNode* SkyDomeObject::skyDomeNode() {
  return skyDomeNode_;
}

// ===============================================================================
// Object information
// ===============================================================================

/** Returns a string containing all information about the current object. This also
* includes the information provided by BaseObjectData::getObjectinfo
*
* @return String containing the object information
*/
QString SkyDomeObject::getObjectinfo() {
  QString output;

  output += "========================================================================\n";
  output += BaseObjectData::getObjectinfo();

  if ( dataType( DATA_SKYDOME ) )
    output += "Object Contains SkyDome : ";

  output += " Horizontal FOV: ( " + QString::number(skyDome_.horizontalFOV()) + ")";

  output += "========================================================================\n";
  return output;
}

SkyDome& SkyDomeObject::getSkyDome() {
  return skyDome_;
}



// ===============================================================================
// Picking
// ===============================================================================

/** Given an node index from PluginFunctions::scenegraphPick this function can be used to
* check if the planeNode of the object has been picked.
*
* @param _node_idx Index of the picked plane node
* @return if the planeNode of this object is the picking target.
*/
bool SkyDomeObject::picked( uint _node_idx ) {
  return ( _node_idx == skyDomeNode_->id() );
}

//
//void SkyDomeObject::enablePicking( bool _enable ) {
//  SkyDomeNode_->enablePicking( _enable );
//}
//
//bool SkyDomeObject::pickingEnabled() {
//  return SkyDomeNode_->pickingEnabled();
//}

//=============================================================================

