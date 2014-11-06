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
*   $Revision$                                                       *
*   $LastChangedBy$                                                *
*   $Date$                     *
*                                                                            *
\*===========================================================================*/


#include "OpenFlipper/BasePlugin/PluginFunctions.hh"

#include "TangentSpace.hh"

#include <OpenFlipper/common/GlobalOptions.hh>


#if QT_VERSION >= 0x050000 
  #include <QtWidgets>
#else
  #include <QtGui>
#endif
#include <QVBoxLayout>
#include <QHash>


#include <map>

#ifdef USE_OPENMP
#include <omp.h>
#endif


#ifdef ENABLE_EIGEN3
#include <Eigen/SVD>
#endif // ENABLE_EIGEN3

#include <limits>


const float cmp_eps = std::numeric_limits<float>::epsilon();


/*
features:
- tangents are stored as float4, property name chosen by user (default: "inTangent" based on vertex shader input naming)

- per vertex tangents computed as weighted average of TBN matrices of adjacent triangles

- high quality per halfedge tangents are computed via smoothing groups
    - smoothing group = group of incoming halfedges with the same normal, texcoord and parity
    - weighted average only within the same smoothing group
    - texture seams are preserved

- 4th component stores parity of tangent space matrix
   reconstruction in shader:
     bitangent = cross(normal, tangent) * parity

shader: transform from tangent space to object space:
   vec3 t,b,n = tangent space matrix computed by this plugin
   vec3 normalTS = sample normal map
   vec3 normalOS = normalize( t * normalTS.x  +  b * normalTS.y  +  n * normalTS.z );


note: propvis plugin does not work with float4 props

possible improvements
- compute custom smoothing groups via angle threshold (normals and uv angle, might require welding threshold)
- PolyMesh support

reference: http://www.crytek.com/download/Triangle_mesh_tangent_space_calculation.pdf
*/

TangentSpace::TangentSpace()
  : weightByAngle_(true), weightByArea_(false), weightByUVArea_(false),
  weightByAngleGUI_(0), weightByAreaGUI_(0), weightByUVAreaGUI_(0),
  overwriteVertexNormals_(true), overwriteNormalsGUI_(0),
  preserveTextureSeams_(true), preserveTextureSeamsGUI_(0),
  decompMethod_(DECOMP_HALF_ANGLE), decompMethodGUI_(0),
  propName_("inTangent"),  propNameGUI_(0)
{
}


TangentSpace::~TangentSpace()
{
}




/** \brief Set the scripting slot descriptions
 *
 */
void
TangentSpace::pluginsInitialized()
{
  QWidget* tool_ = new QWidget();
  QSize size(100, 100);
  tool_->resize(size);

  QVBoxLayout* layout = new QVBoxLayout(tool_);

  QPushButton* button = new QPushButton("Compute per vertex",tool_);
  QPushButton* buttonH = new QPushButton("Compute per halfedge",tool_);

  weightByAngleGUI_ = new QCheckBox("Weight by angle", tool_);
  weightByAngleGUI_->setChecked(weightByAngle_);
  weightByAngleGUI_->setToolTip("inner angle at vertices");

  weightByAreaGUI_ = new QCheckBox("Weight by area", tool_);
  weightByAreaGUI_->setChecked(weightByArea_);
  weightByAreaGUI_->setToolTip("area of triangles in object-space");

  weightByUVAreaGUI_ = new QCheckBox("Weight by texture area", tool_);
  weightByUVAreaGUI_->setChecked(weightByUVArea_);
  weightByUVAreaGUI_->setToolTip("area of triangles in texture-space");

  decompMethodGUI_ = new QComboBox(tool_);

  decompMethodGUI_->addItem("Gram-Schmidt", DECOMP_GRAM_SCHMIDT);
  decompMethodGUI_->addItem("Half angle", DECOMP_HALF_ANGLE);
#ifdef ENABLE_EIGEN3
  decompMethodGUI_->addItem("Polar decomposition", DECOMP_POLAR);
#endif // ENABLE_EIGEN3
  decompMethodGUI_->setCurrentIndex(decompMethod_);
  decompMethodGUI_->setToolTip("Orthogonalization method");

  overwriteNormalsGUI_ = new QCheckBox("Overwrite normals", tool_);
  overwriteNormalsGUI_->setChecked(overwriteVertexNormals_);
  overwriteNormalsGUI_->setToolTip("Discard current normals of mesh and use results from tbn matrix");

  preserveTextureSeamsGUI_ = new QCheckBox("Preserve texture seams", tool_);
  preserveTextureSeamsGUI_->setChecked(preserveTextureSeams_);
  preserveTextureSeamsGUI_->setToolTip("Don't smooth along texture seams (only for per-halfedge tangents)");

  propNameGUI_ = new QLineEdit(tool_);
  propNameGUI_->setText(propName_.c_str());
  propNameGUI_->setToolTip("Property name");

  layout->addWidget(weightByAngleGUI_);
  layout->addWidget(weightByAreaGUI_);
  layout->addWidget(weightByUVAreaGUI_);
  layout->addWidget(preserveTextureSeamsGUI_);
  layout->addWidget(overwriteNormalsGUI_);
  layout->addWidget(decompMethodGUI_);
  layout->addWidget(propNameGUI_);
  layout->addWidget(button);
  layout->addWidget(buttonH);

  // connect signals->slots
  connect(button,SIGNAL(clicked() ),this,SLOT(slotComputePerVertex()));
  connect(buttonH,SIGNAL(clicked() ),this,SLOT(slotComputePerHalfedge()));

  QIcon* icon = new QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator() + "tangent_space_icon.png");
  emit addToolbox( tr("TangentSpace") , tool_ , icon );


}


void TangentSpace::getGUIConfig()
{
  weightByAngle_ = weightByAngleGUI_->isChecked();
  weightByArea_ = weightByAreaGUI_->isChecked();
  weightByUVArea_ = weightByUVAreaGUI_->isChecked();
  decompMethod_ = decompMethodGUI_->itemData( decompMethodGUI_->currentIndex() ).toInt();
  overwriteVertexNormals_ = overwriteNormalsGUI_->isChecked();
  preserveTextureSeams_ = preserveTextureSeamsGUI_->isChecked();
  propName_ = propNameGUI_->text().toStdString();
}


void TangentSpace::slotComputePerVertex( ) 
{
  // get configuration from gui elements
  getGUIConfig();

  // process all tri-meshes
  for ( PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS,DATA_TRIANGLE_MESH) ;
      o_it != PluginFunctions::objectsEnd(); ++o_it)  
  {
    TriMeshObject* object = PluginFunctions::triMeshObject(*o_it);

    if ( object == 0 ) 
    {
      std::cerr << "No mesh!" << std::endl;
      continue;
    }

    // Get triangle mesh
    TriMesh* mesh = PluginFunctions::triMesh(*o_it);

    if (mesh == 0)
    {
      std::cerr << "No mesh!" << std::endl;
      return;
    }

    if (!mesh->has_halfedge_texcoords2D() && !mesh->has_vertex_texcoords2D())
    {
      std::cerr << "No texcoords!" << std::endl;
      return;
    }

    computePerVertexTangents(mesh);

    emit updatedObject(o_it->id(),UPDATE_ALL);
  }
}

void TangentSpace::slotComputePerHalfedge( ) 
{
  // get configuration from gui elements
  getGUIConfig();

  // process all tri-meshes
  for ( PluginFunctions::ObjectIterator o_it(PluginFunctions::TARGET_OBJECTS,DATA_TRIANGLE_MESH) ;
    o_it != PluginFunctions::objectsEnd(); ++o_it)  
  {
    TriMeshObject* object = PluginFunctions::triMeshObject(*o_it);

    if ( object == 0 ) 
    {
      std::cerr << "No mesh!" << std::endl;
      continue;
    }

    // Get triangle mesh
    TriMesh* mesh = PluginFunctions::triMesh(*o_it);

    if (mesh == 0)
    {
      std::cerr << "No mesh!" << std::endl;
      return;
    }

    if (!mesh->has_halfedge_texcoords2D() && !mesh->has_vertex_texcoords2D())
    {
      std::cerr << "No texcoords!" << std::endl;
      return;
    }

    if (!mesh->has_halfedge_normals())
    {
      std::cerr << "No halfedge normals!" << std::endl;
      return;
    }

    computePerHalfedgeTangents(mesh);

    emit updatedObject(o_it->id(),UPDATE_ALL);
  }
}

void TangentSpace::computePerVertexTangents( TriMesh* mesh )
{
  OpenMesh::VPropHandleT< ACG::Vec4f > vtangentPropHandle;

  if (!mesh->get_property_handle( vtangentPropHandle, propName_))
    mesh->add_property( vtangentPropHandle, propName_ );

  const int numVertices = mesh->n_vertices();

  // computation can be parallel
#ifdef  USE_OPENMP
#pragma omp parallel for
#endif
  for ( int curVertex = 0; curVertex < numVertices; ++curVertex )
  {
    TriMesh::VertexHandle curVertexHandle = mesh->vertex_handle(curVertex);
    ACG::Vec3f vertexNormal(0.0f, 0.0f, 0.0f); 
    
    if ( mesh->has_vertex_normals() )
      vertexNormal = ACG::Vec3f(mesh->normal(curVertexHandle)[0], mesh->normal(curVertexHandle)[1], mesh->normal(curVertexHandle)[2]);

    TangentBasis averageTBN;
    averageTBN.setZero();

    for ( TriMesh::VertexIHalfedgeIter voh_it = mesh->vih_begin(curVertexHandle); voh_it != mesh->vih_end(curVertexHandle); ++voh_it )
    {
      if (!mesh->is_boundary(*voh_it))
      {
        TriMesh::FaceHandle fh = mesh->face_handle(*voh_it);

        TangentBasis triangleTBN;
        computeWeightedTangentSpace(mesh, *voh_it, &triangleTBN);

        averageTBN.add(triangleTBN);
      }
    }

    if (!overwriteVertexNormals_)
    {
      // use precomputed normal instead of tangent space normal before orthogonalization
      averageTBN.n = vertexNormal;
    }

    averageTBN.orthonormalize(decompMethod_);

    // store tangent and parity
    mesh->property(vtangentPropHandle, curVertexHandle) =  ACG::Vec4f(averageTBN.t[0], averageTBN.t[1], averageTBN.t[2], averageTBN.parity);
//    mesh->property(vtangentPropHandle, curVertexHandle) =  ACG::Vec3d(averageTBN.t[0], averageTBN.t[1], averageTBN.t[2]);

    if (overwriteVertexNormals_ && mesh->has_vertex_normals())
      mesh->set_normal(curVertexHandle, ACG::Vec3d(averageTBN.n[0], averageTBN.n[1], averageTBN.n[2]));
  }
}

// key for hashtable of smoothing groups
struct TangentSpace_SmoothingGroupKey
{
  // normal
  ACG::Vec3f n;
  // texcoord
  ACG::Vec2f uv;

  TangentSpace_SmoothingGroupKey(const ACG::Vec3f& _n, ACG::Vec2f _uv = ACG::Vec2f(0.0f, 0.0f))
    : n(_n), uv(_uv)
  {
  }

  bool operator == (const TangentSpace_SmoothingGroupKey& rhs) const
  {
    float cosTheta = n | rhs.n;

    if (cosTheta > 0.998f)
    {
      ACG::Vec2f d = uv - rhs.uv;
      if ( fabsf(d[0]) < cmp_eps && fabsf(d[1]) < cmp_eps )
        return true;
    }

    return false;
  }
};

uint qHash(const TangentSpace_SmoothingGroupKey& k)
{
  // compute hash of each float
  uint x[5];

  for (int i = 0; i < 5; ++i)
  {
    uint u;

    // get float from normal/uv, interpret as uint
    if (i < 3)
      u = *reinterpret_cast<const uint*>(k.n.data() + i);
    else
      u = *reinterpret_cast<const uint*>(k.uv.data() + i - 3);

    // discard high precision mantissa bits
    u &= 0xfffffe00;

    x[i] = ::qHash(u);
  }

  // combine hash values with prime numbers
  uint p[5] = {439, 3373, 1033, 6673, 53};

  uint result = 0;

  for (int i = 0; i < 5; ++i)
    result += x[i] * p[i];

  return ((result >> 22) ^ (result >> 11) ^ result);
}

void TangentSpace::computePerHalfedgeTangents( TriMesh* mesh )
{
  /* 
  tangent smoothing strategies:
  - no smoothing across different halfedge normals
      based on the assumption that normals have been computed via smoothing groups already

  - no smoothing across texture seams (parity check)

  implementation via smoothing groups:
  */

  OpenMesh::HPropHandleT< ACG::Vec4f > htangentPropHandle;

  if (!mesh->get_property_handle( htangentPropHandle, propName_))
    mesh->add_property( htangentPropHandle, propName_ );

  const int numVertices = mesh->n_vertices();

  // computation can be parallel
#ifdef  USE_OPENMP
#pragma omp parallel for
#endif
  for (int curVertex = 0; curVertex < numVertices; ++curVertex )
  {
    TriMesh::VertexHandle curVertexHandle = mesh->vertex_handle(curVertex);

    // map from normal,texc combination to first halfedge for that combination
    QHash<TangentSpace_SmoothingGroupKey, TriMesh::HalfedgeHandle> UniqueNormals;

    // maps from halfedge -> smoothing group
    std::map<TriMesh::HalfedgeHandle, int> SmoothingGroups;
    int numNormalGroups = 0;

    for ( TriMesh::VertexIHalfedgeIter voh_it = mesh->vih_begin(curVertexHandle); voh_it != mesh->vih_end(curVertexHandle); ++voh_it )
    {
      // openmesh stores halfedges at boundaries, which do not belong to any face
      // opposite halfedge should belong to a face, so its fine to skip empty halfedges
      if ( mesh->is_boundary(*voh_it) )
        continue; 

      // halfedge normal
      ACG::Vec3f hn = ACG::Vec3f(mesh->normal(*voh_it)[0], mesh->normal(*voh_it)[1], mesh->normal(*voh_it)[2]);

      // halfedge texcoord
      ACG::Vec2f ht(0.0f, 0.0f);

      if (preserveTextureSeams_)
      {
        if (mesh->has_halfedge_texcoords2D())
          ht = mesh->texcoord2D(*voh_it);
        else
          ht = mesh->texcoord2D( mesh->to_vertex_handle(*voh_it) );
      }

      TangentSpace_SmoothingGroupKey key(hn, ht);

      QHash<TangentSpace_SmoothingGroupKey, TriMesh::HalfedgeHandle>::iterator itNormalGroup = UniqueNormals.find(key);

      if (itNormalGroup == UniqueNormals.end())
      {
        // normal has not been encountered yet -> new smoothing group
        SmoothingGroups[*voh_it] = numNormalGroups++; 
        UniqueNormals[key] = *voh_it;
      }
      else
      {
        // normal has been references already, use same smoothing group
        TriMesh::HalfedgeHandle firstHalfedge = *itNormalGroup;
        SmoothingGroups[*voh_it] = SmoothingGroups[firstHalfedge];
      }
    }

    // tangent space per smoothing group
    //  smoothing group with flipped parity at index: smGroup + numNormalGroups
    std::vector<TangentBasis> TangentSpaces(numNormalGroups * 2);

    // set zero
    for (size_t i = 0; i < TangentSpaces.size(); ++i)
      TangentSpaces[i].setZero();

    int numSmoothingGroups = numNormalGroups;

    for ( TriMesh::VertexIHalfedgeIter voh_it = mesh->vih_begin(curVertexHandle); voh_it != mesh->vih_end(curVertexHandle); ++voh_it )
    {
      // skip empty boundary halfedge
      if (mesh->is_boundary(*voh_it))
        continue;

      TriMesh::FaceHandle fh = mesh->face_handle(*voh_it);

      // get halfedge normal
      ACG::Vec3f hn = ACG::Vec3f(mesh->normal(*voh_it)[0], mesh->normal(*voh_it)[1], mesh->normal(*voh_it)[2]);

      // get smoothing group of current halfedge
      int smGroup = SmoothingGroups[*voh_it];

      if (fh == mesh->InvalidFaceHandle)
      {
        std::cerr << "warning: face_handle invalid for halfedge " << *voh_it << std::endl;
        continue;
      }

      TangentBasis triTbn;
      computeWeightedTangentSpace(mesh, *voh_it, &triTbn);

      float parity = triTbn.parity;
      float groupParity = TangentSpaces[smGroup].parity;


      if (groupParity == 0.0f || groupParity == parity)
      {
        // compute weighted average inside smoothing group
        TangentSpaces[smGroup].add(triTbn);
        TangentSpaces[smGroup].parity = parity;
      }
      else
      {
        // mirrored uv coords
        // create new smoothing group along seam

        // group id for flipped parity = groupId + numNormalGroups
        smGroup += numNormalGroups;
        SmoothingGroups[*voh_it] = smGroup;

        // make sure flipped parity is correct
        if (TangentSpaces[smGroup].parity == 0.0f)
        {
          ++numSmoothingGroups;
          TangentSpaces[smGroup].parity = triTbn.parity;
        }
        else
          assert(TangentSpaces[smGroup].parity == triTbn.parity);

        TangentSpaces[smGroup].add(triTbn);
      }

      // use precomputed halfedge normal instead of tangent space normal
      TangentSpaces[smGroup].n = hn;
    }


    // orthonormalize 
    int numTBNMatrices = 0;

    for (size_t i = 0; i < TangentSpaces.size(); ++i)
    {
      TangentBasis* tbn = &TangentSpaces[i];

      if (tbn->parity != 0.0f)
      {
        tbn->orthonormalize(decompMethod_);
        ++numTBNMatrices;
      }
    }

    if (!numTBNMatrices)
    {
      std::cerr << "error: could not compute halfedge tangents for vertex " << curVertexHandle << std::endl;
    }
    else
    {
      if (numTBNMatrices != numSmoothingGroups)
        std::cerr << "warning: could not compute tangents for all smoothing groups of vertex" << curVertexHandle << std::endl;

      // set per halfedge property

      for ( TriMesh::VertexIHalfedgeIter voh_it = mesh->vih_begin(curVertexHandle); voh_it != mesh->vih_end(curVertexHandle); ++voh_it )
      {
        // get matrix in smoothing group of halfedge
        int smGroup = SmoothingGroups[*voh_it];
        TangentBasis* tbn = &TangentSpaces[smGroup];

        if (tbn->parity == 0.0f)
        {
          // something went wrong, choose matrix from other smoothing group

          tbn = &TangentSpaces[0];

          while (tbn->parity == 0.0f)
            ++tbn;
        }

        // update normals after polar decomp
        if (decompMethod_ == DECOMP_POLAR)
        {
          if (overwriteVertexNormals_)
            mesh->set_normal(*voh_it, ACG::Vec3d(tbn->n[0], tbn->n[1], tbn->n[2]));
          else
          {
            // normal may not be changed, rotate from back to normal
            ACG::Vec3f storedNormal = ACG::Vec3f(mesh->normal(*voh_it)[0], mesh->normal(*voh_it)[1], mesh->normal(*voh_it)[2]);

            if (tbn->n != storedNormal)
            {
              tbn->n = storedNormal;
              tbn->orthonormalize(DECOMP_HALF_ANGLE);
            }
          }
        }

        mesh->property(htangentPropHandle, *voh_it) =  ACG::Vec4f(tbn->t[0], tbn->t[1], tbn->t[2], tbn->parity);
//        mesh->property(htangentPropHandle, *voh_it) =  ACG::Vec3d(tbn->t[0], tbn->t[1], tbn->t[2]);
      }
    }

  }
}


float TangentSpace::computeTriTBN( const ACG::Vec3f* _pos, const ACG::Vec2f* _texc, ACG::Vec3f* _outT, ACG::Vec3f* _outB, ACG::Vec3f* _outN, bool _divByDet )
{
  ACG::Vec3f q1 = _pos[1] - _pos[0];
  ACG::Vec3f q2 = _pos[2] - _pos[0];

  ACG::Vec2f r1 = _texc[1] - _texc[0];
  ACG::Vec2f r2 = _texc[2] - _texc[0];

  // solve q = r * TB

  // invert 2x2 matrix r
  float det = r1[0]*r2[1] - r2[0]*r1[1];

  float detInv = 0.0f;

  if (_divByDet)
  {
    if (fabsf(det) < 1e-6f)
      detInv = 1.0f / (det + 1e-5f);
    else
      detInv = 1.0f / det;
  }
  else
  {
    // if the tbn matrix gets normalized anyway there is no need to divide by the determinant
    // -> more robust
    detInv = det >= 0.0f ? 1.0f : -1.0f;
  }

  // TB = inv(r) * q

  ACG::Vec3f t = (q1 * r2[1] - q2 * r1[1]) * detInv;
  ACG::Vec3f b = (q2 * r1[0] - q1 * r2[0]) * detInv;
//  ACG::Vec3f n = t % b; // % = cross product
  ACG::Vec3f n = q1 % q2; // gives better results than cross(t,b)

  if (_outT) *_outT = t;
  if (_outB) *_outB = b;
  if (_outN) *_outN = n;

  return computeParity(t,b,n);
}

float TangentSpace::computeTriTBN(  TriMesh* mesh, TriMesh::FaceHandle _fh, ACG::Vec3f* _outT, ACG::Vec3f* _outB, ACG::Vec3f* _outN, bool _divByDet )
{
  ACG::Vec3f pos[3]; // position
  ACG::Vec2f texc[3]; // uv texcoord
  ACG::Vec3f t,b,n; // tangent space matrix

  // get pos, texc array of triangle
  int triV = 0;
  for (TriMesh::FaceHalfedgeIter fh_it = mesh->fh_begin(_fh) ; fh_it != mesh->fh_end(_fh); ++fh_it)
  {
    TriMesh::HalfedgeHandle hh = *fh_it;
    TriMesh::VertexHandle vh = mesh->to_vertex_handle(hh);
    pos[triV] = mesh->point(vh);

    if (mesh->has_halfedge_texcoords2D())
      texc[triV] = mesh->texcoord2D(hh);
    else
      texc[triV] = mesh->texcoord2D(vh);

    ++triV;
  }

  // compute unnormalized tangent space
  return computeTriTBN(pos, texc, _outT, _outB, _outN, _divByDet);
}

float TangentSpace::computeFaceTBN( PolyMesh* mesh, PolyMesh::FaceHandle _fh,  ACG::Vec3f* _outT, ACG::Vec3f* _outB, ACG::Vec3f* _outN, bool _divByDet)
{
  // tangent space matrix is well defined for a face if
  //  - planar polygon
  //  - linear uv field  (ie. constant uv gradient inside polygon)
  // faces of TriMesh always satisfy these conditions

  ACG::Vec3f pos[3]; // position
  ACG::Vec2f texc[3]; // uv texcoord
  ACG::Vec3f t,b,n; // tangent space matrix

  // get pos, texc array of triangle
  int triV = 0;
  for (PolyMesh::FaceHalfedgeIter fh_it = mesh->fh_begin(_fh) ; fh_it != mesh->fh_end(_fh) && triV < 3; ++fh_it)
  {
    PolyMesh::HalfedgeHandle hh = *fh_it;
    PolyMesh::VertexHandle vh = mesh->to_vertex_handle(hh);
    pos[triV] = mesh->point(vh);

    if (mesh->has_halfedge_texcoords2D())
      texc[triV] = mesh->texcoord2D(hh);
    else
      texc[triV] = mesh->texcoord2D(vh);

    ++triV;
  }

  // compute unnormalized tangent space
  return computeTriTBN(pos, texc, _outT, _outB, _outN, _divByDet);
}



float TangentSpace::computeUVArea( TriMesh* _mesh, TriMesh::HalfedgeHandle _h )
{
  if (_mesh->is_boundary(_h))
    return 0.0f;

  TriMesh::FaceHandle fh = _mesh->face_handle(_h);

  ACG::Vec2f texc[3]; // uv texcoord

  // get texcoords of tri
  int triV = 0;
  for (TriMesh::FaceHalfedgeIter fh_it = _mesh->fh_begin(fh) ; fh_it != _mesh->fh_end(fh); ++fh_it)
  {
    TriMesh::HalfedgeHandle hh = *fh_it;
    TriMesh::VertexHandle vh = _mesh->to_vertex_handle(hh);
    if (_mesh->has_halfedge_texcoords2D())
      texc[triV] = _mesh->texcoord2D(hh);
    else
      texc[triV] = _mesh->texcoord2D(vh);

    ++triV;
  }

  ACG::Vec2f q1 = texc[1] - texc[0];
  ACG::Vec2f q2 = texc[2] - texc[0];

  // area = det(q1, q2) / 2
  float area = fabsf( q1[0]*q2[1] - q1[1]*q2[0] ) * 0.5f;

  return area;
}



void TangentSpace::computeWeightedTangentSpace( TriMesh* mesh, TriMesh::HalfedgeHandle _h, TangentBasis* _out )
{
  _out->setZero();
  if (mesh->is_boundary(_h))
    return;

  TriMesh::FaceHandle fh = mesh->face_handle(_h);

  bool weighted = weightByAngle_ || weightByArea_;
  _out->parity = computeTriTBN(mesh, fh, &_out->t, &_out->b, &_out->n, !weighted);

  if (weighted)
  {
    _out->t.normalize();
    _out->b.normalize();
    _out->n.normalize();
  }

  float weight = 1.0f;

  // weight by inner angle to get tessellation independent tangents
  if (weightByAngle_)
    weight *= mesh->calc_sector_angle(_h);

  // weight by area to prefer bigger faces over small faces
  if (weightByArea_)
    weight *= mesh->calc_sector_area(_h);  // angle and area equally weighted

  if (weightByUVArea_)
    weight *= computeUVArea(mesh, _h);

//   if (weight == 0.0f) 
//     weight = 1.0f; // no weighting

  _out->t *= weight;
  _out->b *= weight;
  _out->n *= weight;
}


void TangentSpace::computeWeightedTangentSpace( PolyMesh* mesh, PolyMesh::HalfedgeHandle _h, TangentBasis* _out )
{
  _out->setZero();
  if (mesh->is_boundary(_h))
    return;

  PolyMesh::FaceHandle fh = mesh->face_handle(_h);

  bool weighted = weightByAngle_ || weightByArea_;
  _out->parity = computeFaceTBN(mesh, fh, &_out->t, &_out->b, &_out->n, !weighted);

  if (weighted)
  {
    _out->t.normalize();
    _out->b.normalize();
    _out->n.normalize();
  }

  float weight = 1.0f;

  // weight by inner angle to get tessellation independent tangents
  if (weightByAngle_)
    weight *= mesh->calc_sector_angle(_h);

  // weight by area to prefer bigger faces over small faces
  if (weightByArea_)
    weight *= mesh->calc_sector_area(_h);  // angle and area equally weighted

//   if (weight == 0.0f) 
//     weight = 1.0f; // no weighting

  _out->t *= weight;
  _out->b *= weight;
  _out->n *= weight;
}


float TangentSpace::computeParity( const ACG::Vec3f& t, const ACG::Vec3f& b, const ACG::Vec3f& n )
{
  return ((b | (n % t)) < 0.0f) ? -1.0f : 1.0f; // | = dot, % = cross
}

void TangentSpace::TangentBasis::computeParity()
{
  parity = ((b | (n % t)) < 0.0f) ? -1.0f : 1.0f; // | = dot, % = cross
}

void TangentSpace::TangentBasis::orthonormalize(int method)
{
  normalize();

  // check for linear independence
  if ( fabsf( fabsf(t | b) - 1.0f ) < cmp_eps )
  {
    std::cerr << "warning: degenerated uv mapping" << std::endl;
    b = n % t;
  }

  if ( fabsf( fabsf(n | t) - 1.0f ) < cmp_eps )
  {
    std::cerr << "warning: degenerated uv mapping" << std::endl;
    t = b % n;
  }

  // check parity after orthogonalization with current parity
  computeParity();
  const float prevParity = parity; 



  // extract orthogonal matrix via given method

  if (method == TangentSpace::DECOMP_GRAM_SCHMIDT)
  {
    // Gram-Schmidt orthogonalization weights: n > t > b

    t = t - (n | t) * n;
    t.normalize();

    b = (n % t) * parity;
  }
  else if (method == TangentSpace::DECOMP_HALF_ANGLE)
  {
    // Gram-Schmidt orthogonalization weights: n > t = b

    // make t and b orthogonal to n

    t = t - (n | t) * n;
    t.normalize();

    b = b - (n | b) * n;
    b.normalize();


    // make t and b orthogonal to each other with equal weighting

    // half vector between t and b
    ACG::Vec3f h = t + b;
    h.normalize();

    // vector orthogonal to h and n
    ACG::Vec3f tH = h % n; 

    tH.normalize();

    // new tangent is half vector between h and tH
    // this is the same as rotating h by 45 deg in the direction of t

    float handedness = (tH | t) > 0.0f ? 1.0f : -1.0f;

    t = h +  tH * handedness;
    t.normalize();



    b = (n % t) * parity;
  }
  else if (method == TangentSpace::DECOMP_POLAR)
  {
#ifdef ENABLE_EIGEN3

    // polar decomposition,  t,b,n all equally weighted

    // M = R * H,  where R is orthogonal
    // given svd M = U * S * V, the polar decomposition of M is given as
    //  R = U * transpose(V)
    //  H = V * S * transpose(V)

    // M = tangent space matrix
    Eigen::Matrix3f M;

    for (int r = 0; r < 3; ++r)
    {
      M(r, 0) = t[r];
      M(r, 1) = b[r];
      M(r, 2) = n[r];
    }

    Eigen::JacobiSVD<Eigen::Matrix3f, Eigen::FullPivHouseholderQRPreconditioner> svd(M, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::Matrix3f R = svd.matrixU() * svd.matrixV().transpose();

    for (int r = 0; r < 3; ++r)
    {
      t[r] = R(r, 0);
      b[r] = R(r, 1);
      n[r] = R(r, 2);
    }

    // check result
//     assert( fabsf(t | b) < 1e-6f );
//     assert( fabsf(t | n) < 1e-6f );
//     assert( fabsf(n | b) < 1e-6f );
// 
//     assert( fabsf(t.norm() - 1.0f) < 1e-6f);
//     assert( fabsf(b.norm() - 1.0f) < 1e-6f);
//     assert( fabsf(n.norm() - 1.0f) < 1e-6f);
// 
//     computeParity();
//     assert(prevParity == parity);

#else
    // use qr decomposition with equal tangent weighting instead
    orthonormalize(TangentSpace::DECOMP_HALF_ANGLE);
#endif // ENABLE_EIGEN3
  }

  // parity check
  computeParity();
  if (parity != prevParity)
  {
    // parity flip indicates degenerated triangle or degenerated uv parameterization
    std::cerr << "warning: parity flip after orthogonalization of tbn matrix" << std::endl;
  }
}

void TangentSpace::TangentBasis::setZero()
{
  t = b = n = ACG::Vec3f(0.0f, 0.0f, 0.0f);
  parity = 0.0f;
}

void TangentSpace::TangentBasis::normalize()
{
  t.normalize();
  b.normalize();
  n.normalize();
}

void TangentSpace::TangentBasis::add( const TangentBasis& _r )
{
  t += _r.t;
  b += _r.b;
  n += _r.n;
}


#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2( tangentspaceplugin , TangentSpace );
#endif
