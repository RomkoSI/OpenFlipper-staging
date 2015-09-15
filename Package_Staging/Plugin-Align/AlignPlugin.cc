/**
 * @author Vladimir Chalupecky (vladimir.chalupecky@gmail.com)
 */

#include <ObjectTypes/PolyhedralMesh/PolyhedralMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>

#include <Eigen/Dense>

#include "AlignPlugin.hh"

AlignPlugin::AlignPlugin()
	: sourceMeshCombo_()
	, targetMeshCombo_()
	, doScaleCheckBox_()
	, alignMeshesButton_()
	, sourceId_(-1)
	, targetId_(-1)
{
}

void AlignPlugin::slotAlignMeshes()
{
	using namespace Eigen;

	BaseObjectData *sourceObject = NULL;
	if (!PluginFunctions::getObject(sourceId_, sourceObject)) {
		emit log(LOGERR, tr("Cannot get source mesh object %1").arg(sourceId_));
		return;
	}

	BaseObjectData *targetObject = NULL;
	if (!PluginFunctions::getObject(targetId_, targetObject)) {
		emit log(LOGERR, tr("Cannot get target mesh object %1").arg(targetId_));
		return;
	}

	Vector3d sc;
	Matrix3d spd;
	if (!computePCA(getPoints(sourceObject), sc, spd)) {
		emit log(LOGERR, "PCA of source object failed");
		return;
	}

	Vector3d tc;
	Matrix3d tpd;
	Point_container targetPoints = getPoints(targetObject);
	if (!computePCA(targetPoints, tc, tpd)) {
		emit log(LOGERR, "PCA of target object failed");
		return;
	}

	Affine3d t = spd.transpose() * Translation3d(-sc);
	transformObject(sourceObject, t);

	t = tpd.transpose() * Translation3d(-tc);
	transformPoints(targetPoints, t);

	t = t.inverse();
	if (doScaleCheckBox_->isChecked()) {
		BBox sbbox = computeBoundingBox(getPoints(sourceObject));
		BBox tbbox = computeBoundingBox(targetPoints);
		t = t * Scaling((tbbox.second - tbbox.first).cwiseQuotient(sbbox.second - sbbox.first));
	}
	transformObject(sourceObject, t);

	emit updatedObject(sourceId_, UPDATE_GEOMETRY);
	emit createBackup(sourceId_, "Align", UPDATE_GEOMETRY);
}

AlignPlugin::Point_container AlignPlugin::getPoints(BaseObjectData *object)
{
	using namespace Eigen;

	Point_container points;

	if (object->dataType(DATA_HEXAHEDRAL_MESH)) {
		HexahedralMesh *mesh = PluginFunctions::hexahedralMesh(object);
		for (OpenVolumeMesh::VertexIter it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
			if (mesh->is_boundary(*it)) {
				Vector3d p = Map<const Vector3d>(mesh->vertex(*it).data());
				points.push_back(p);
			}
		}
	} else if (object->dataType(DATA_POLYHEDRAL_MESH)) {
		PolyhedralMesh *mesh = PluginFunctions::polyhedralMesh(object);
		for (OpenVolumeMesh::VertexIter it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
			if (mesh->is_boundary(*it)) {
				Vector3d p = Map<const Vector3d>(mesh->vertex(*it).data());
				points.push_back(p);
			}
		}
	} else { // DATA_TRIANGLE_MESH
		TriMesh *mesh = PluginFunctions::triMesh(object);
		points.reserve(mesh->n_vertices());
		for (TriMesh::VertexIter it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
			Vector3d p = Map<const Vector3d>(mesh->point(*it).data());
			points.push_back(p);
		}
	}

	return points;
}

void AlignPlugin::transformObject(BaseObjectData *object, Eigen::Affine3d const& t)
{
	using namespace Eigen;

	if (object->dataType(DATA_HEXAHEDRAL_MESH)) {
		HexahedralMesh *mesh = PluginFunctions::hexahedralMesh(object);
		for (OpenVolumeMesh::VertexIter it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
			ACG::Vec3d const& p = mesh->vertex(*it);
			Vector3d ep(p[0], p[1], p[2]);
			Vector3d tep = t * ep;
			mesh->set_vertex(*it, ACG::Vec3d(tep[0], tep[1], tep[2]));
		}
	} else if (object->dataType(DATA_POLYHEDRAL_MESH)) {
		PolyhedralMesh *mesh = PluginFunctions::polyhedralMesh(object);
		for (OpenVolumeMesh::VertexIter it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
			ACG::Vec3d const& p = mesh->vertex(*it);
			Vector3d ep(p[0], p[1], p[2]);
			Vector3d tep = t * ep;
			mesh->set_vertex(*it, ACG::Vec3d(tep[0], tep[1], tep[2]));
		}
	} else { // DATA_TRIANGLE_MESH
		TriMesh *mesh = PluginFunctions::triMesh(object);
		for (TriMesh::VertexIter it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
			ACG::Vec3d const& p = mesh->point(*it);
			Vector3d ep(p[0], p[1], p[2]);
			Vector3d tep = t * ep;
			mesh->set_point(*it, ACG::Vec3d(tep[0], tep[1], tep[2]));
		}
		mesh->update_normals();
	}
}

void AlignPlugin::transformPoints(Point_container& points, Eigen::Affine3d const& t)
{
	for (unsigned int i = 0; i < points.size(); ++i) {
		points[i] = t * points[i];
	}
}

AlignPlugin::BBox AlignPlugin::computeBoundingBox(Point_container const& points)
{
	using namespace Eigen;

	double inf = std::numeric_limits<double>::infinity();
	Vector3d minp = Vector3d::Constant(inf);
	Vector3d maxp = Vector3d::Constant(-inf);
	for (unsigned int i = 0; i < points.size(); ++i) {
		minp = minp.cwiseMin(points[i]);
		maxp = maxp.cwiseMax(points[i]);
	}

	return BBox(minp, maxp);
}

Eigen::Vector3d AlignPlugin::computeCentroid(Point_container const& points)
{
	using namespace Eigen;

	Vector3d centroid = Vector3d::Zero();
	for (unsigned int i = 0; i < points.size(); ++i) {
		centroid += points[i];
	}
	if (points.size() > 0) {
		centroid /= points.size();
	}
	return centroid;
}

bool AlignPlugin::computePCA(Point_container const& points, Eigen::Vector3d& centroid, Eigen::Matrix3d& principal_directions)
{
	using namespace Eigen;

	centroid = computeCentroid(points);

	Matrix3d cov = Matrix3d::Zero();
	for (unsigned int i = 0; i < points.size(); ++i) {
		cov.noalias() += (points[i] - centroid) * (points[i] - centroid).transpose();
	}
	cov /= points.size() - 1;

	SelfAdjointEigenSolver<Matrix3d> eigensolver(cov);
	principal_directions = eigensolver.eigenvectors();

	return eigensolver.info() == Success;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(alignplugin, AlignPlugin)
#endif
