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


#ifndef SKYDOME_NODE_HH
#define SKYDOME_NODE_HH

//== INCLUDES =================================================================

#include <ObjectTypes/SkyDome/SkyDomeType.hh>
#include <ACG/Scenegraph/BaseNode.hh>
#include <ACG/Scenegraph/DrawModes.hh>
#include <OpenFlipper/common/GlobalDefines.hh>
#include <ACG/GL/VertexDeclaration.hh>
#include <ACG/GL/IRenderer.hh>
#include <ACG/GL/GLPrimitives.hh>
#include <ObjectTypes/SkyDome/SkyDomeType.hh>

//== NAMESPACES ===============================================================

//== CLASS DEFINITION =========================================================

class DLLEXPORT SkyDomeNode : public ACG::SceneGraph::BaseNode
{
public:

    /** \brief Construct a SkyDome rendering node
     *
     * @param _parent The parent node in the scenegraph
     * @param _name   The name of the new node (visible in the scenegraph dialogs)
     * @param _dome  A pointer to an existing SkyDome object
     */
    SkyDomeNode(SkyDome& _dome, BaseNode *_parent = 0, std::string _name = "<PlaneNode>");

    /// destructor
    ~SkyDomeNode();

    /// static name of this class
    ACG_CLASSNAME(SkyDomeNode);

    /// return available draw modes
    ACG::SceneGraph::DrawModes::DrawMode availableDrawModes() const;

    /// update bounding box
    void boundingBox(ACG::Vec3d & _bbMin, ACG::Vec3d & _bbMax);

    /// draw SkyDome
    void draw(ACG::GLState & _state, const ACG::SceneGraph::DrawModes::DrawMode & _drawMode);

    /// draw SkyDome for object picking
    void pick(ACG::GLState & _state, ACG::SceneGraph::PickTarget _target);

    /// Get the currently rendered plane
    SkyDome& getSkyDome();

    /// Set a new plane for rendering
    void setSkyDome(SkyDome _dome);

    /** \brief Add the objects to the given renderer
     *
     * @param _renderer The renderer which will be used. Add your geometry into this class
     * @param _state    The current GL State when this object is called
     * @param _drawMode The active draw mode
     * @param _mat      Current material
     */
    void getRenderObjects(ACG::IRenderer* _renderer, ACG::GLState&  _state , const ACG::SceneGraph::DrawModes::DrawMode&  _drawMode , const ACG::SceneGraph::Material* _mat);

private:
    void drawSkyDome(ACG::GLState & _state);
    void drawSkyDomePick(ACG::GLState & _state);


    SkyDome& dome_;

    /// VBO used to render the dome
    unsigned int vbo_;

    ACG::VertexDeclaration vertexDecl_;

};

#endif // SKYDOME_NODE_HH
