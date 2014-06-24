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
*   $Revision: 18129 $                                                       *
*   $LastChangedBy: moebius $                                                *
*   $Date: 2014-02-05 10:25:53 +0100 (Mi, 05. Feb 2014) $                     *
*                                                                            *
\*===========================================================================*/

#ifndef TANGENTSPACEPLUGIN_HH
#define TANGENTSPACEPLUGIN_HH

#include <QObject>
#include <QMenuBar>

#include <QLineEdit>

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/common/Types.hh>

#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>

class TangentSpace : public QObject, BaseInterface, ToolboxInterface
{
  Q_OBJECT
  Q_INTERFACES(BaseInterface)
  Q_INTERFACES(ToolboxInterface)

#if QT_VERSION >= 0x050000
  Q_PLUGIN_METADATA(IID "org.OpenFlipper.Plugins.Plugin-TangentSpace")
#endif

  signals:

    void updatedObject(int, const UpdateType&);

  // ToolboxInterface
  void addToolbox( QString _name  , QWidget* _widget );

  private slots:
    void pluginsInitialized();

  public :

    TangentSpace();
    ~TangentSpace();

    QString name() { return (QString("TangentSpace")); };
    QString description( ) { return (QString("Compute tangent space properties")); };

    // decomposition method: extract rotational part from tangent space matrix
    enum
    {
      DECOMP_GRAM_SCHMIDT = 0, // QR decomposition with gram-schmidt process: n is unchanged, b has no influence
      DECOMP_HALF_ANGLE, // QR decomposition with half angle rotation: n is unchanged, t and b equally weighted
      DECOMP_POLAR // polar decomposition, t,b,n all equally weighted  / requires eigen math lib
    };

    struct TangentBasis
    {
      ACG::Vec3f t,b,n;
      float parity;

      void setZero();
      void orthonormalize(int method = 0);

      void normalize();
      void add(const TangentBasis& _r);

      void computeParity();
    };


    // compute unnormalized triangle tangent, bitangent and normal vector
    //  returns parity
    float computeTriTBN(const ACG::Vec3f* _pos, const ACG::Vec2f* _texc, ACG::Vec3f* _outT, ACG::Vec3f* _outB, ACG::Vec3f* _outN, bool _divByDet = true);

    float computeTriTBN(TriMesh* mesh, TriMesh::FaceHandle _fh, ACG::Vec3f* _outT, ACG::Vec3f* _outB, ACG::Vec3f* _outN, bool _divByDet = true);
    float computeFaceTBN(PolyMesh* mesh, PolyMesh::FaceHandle _fh, ACG::Vec3f* _outT, ACG::Vec3f* _outB, ACG::Vec3f* _outN, bool _divByDet = true);


    // compute tangent matrix of triangle (optionally weighted by angle at incoming halfedge and/or area of triangle)
    void computeWeightedTangentSpace(TriMesh* mesh, TriMesh::HalfedgeHandle _h, TangentBasis* _out);
    void computeWeightedTangentSpace(PolyMesh* mesh, PolyMesh::HalfedgeHandle _h, TangentBasis* _out);

    
    // parity of tangent space matrix = sign(det(TBN))
    // returns +1: positive, -1: mirrored
    float computeParity(const ACG::Vec3f& t, const ACG::Vec3f& b, const ACG::Vec3f& n);

    // get pos array of triangle
    void getTriPos(TriMesh* _mesh, TriMesh::FaceHandle _h, ACG::Vec3f* _outPos);

    // area of triangle in texture space
    float computeUVArea(TriMesh* _mesh, TriMesh::HalfedgeHandle _h);

    void computePerVertexTangents(TriMesh* _mesh);
    void computePerHalfedgeTangents(TriMesh* _mesh);

    void getGUIConfig();

  private:

    // weighting of tangent space matrices within smoothing groups
    bool weightByAngle_;
    bool weightByArea_;
    bool weightByUVArea_;
    QCheckBox* weightByAngleGUI_;  // qt checkbox widgets
    QCheckBox* weightByAreaGUI_;
    QCheckBox* weightByUVAreaGUI_;

    // overwrite vertex normals with resulting normals from tangent basis
    bool overwriteVertexNormals_;
    QCheckBox* overwriteNormalsGUI_;

    // detect and preserve sharp edges at texture seams
    bool preserveTextureSeams_;
    QCheckBox* preserveTextureSeamsGUI_;

    // extraction method to get rotational part of tangent space matrix
    int decompMethod_;
    QComboBox* decompMethodGUI_; // qt combobox

    // property name of tangent vectors
    std::string propName_;
    QLineEdit* propNameGUI_; // qt editbox

  public slots:

    void slotComputePerVertex();
    void slotComputePerHalfedge();

    QString version() { return QString("1.0"); };

};





#endif //TANGENTSPACEPLUGIN_HH
