#pragma once

#include <algorithm>
#include <utility>

#include <QAbstractItemModel>

#include <ObjectTypes/HexahedralMesh/HexahedralMesh.hh>
#include <ObjectTypes/PolyhedralMesh/PolyhedralMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/DataTypes.hh>

struct Is_source_mesh {
	bool operator()(BaseObjectData *object) const
	{
		if (object->source()) {
			if (object->dataType(DATA_HEXAHEDRAL_MESH) ||
			    object->dataType(DATA_POLYHEDRAL_MESH) ||
			    object->dataType(DATA_TRIANGLE_MESH)) {
				return true;
			}
		}
		return false;
	}
};

struct Is_target_mesh {
	bool operator()(BaseObjectData *object) const
	{
		if (object->target()) {
			if (object->dataType(DATA_HEXAHEDRAL_MESH) ||
			    object->dataType(DATA_POLYHEDRAL_MESH) ||
			    object->dataType(DATA_TRIANGLE_MESH)) {
				return true;
			}
		}
		return false;
	}
};

template< typename TPredicate >
class ObjectInfoListModel : public QAbstractListModel
{
	typedef std::vector<std::pair<QString, int> > Object_container;

public:
	int rowCount(QModelIndex const&) const
	{
		return objects_.size() + 1;
	}

	QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const
	{
		switch (role) {
		case Qt::DisplayRole:
			if (index.row() <= 0) {
				return tr("<none>");
			}
			return objects_[index.row() - 1].first;
		case Qt::UserRole:
			if (index.row() <= 0) {
				return QVariant::fromValue<int>(-1);
			}
			return QVariant::fromValue(objects_[index.row() - 1].second);
		default:
			return QVariant::Invalid;
		}
	}

	void removeObject(int object_id)
	{
		bool found = false;
		for (unsigned int i = 0; i < objects_.size(); ++i) {
			if (objects_[i].second == object_id) {
				found = true;
				break;
			}
		}

		if (!found) {
			return;
		}

		beginResetModel();
		for (int i = objects_.size() - 1; i >= 0; --i) {
			if (objects_[i].second == object_id) {
				objects_.erase(objects_.begin() + i);
			}
		}
		endResetModel();
	}

	void refresh()
	{
		Object_container objects;
		for (PluginFunctions::ObjectIterator o_it; o_it != PluginFunctions::objectsEnd(); ++o_it) {
			if (predicate_(*o_it)) {
				objects.push_back(std::make_pair(o_it->name(), o_it->id()));
			}
		}
		std::sort(objects.begin(), objects.end());
		if (objects != objects_) {
			beginResetModel();
			objects.swap(objects_);
			endResetModel();
		}
	}

private:
	Object_container objects_;
	TPredicate predicate_;
};
