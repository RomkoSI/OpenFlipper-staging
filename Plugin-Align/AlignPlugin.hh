#pragma once

/**
 * @author Vladimir Chalupecky (vladimir.chalupecky@gmail.com)
 */

#include <set>

#include <OpenFlipper/BasePlugin/BackupInterface.hh>
#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/common/Types.hh>

#include <Eigen/Geometry>

#include "ObjectInfoListModel.hh"

class AlignPlugin : public QObject,
	BackupInterface,
	BaseInterface,
	LoadSaveInterface,
	LoggingInterface,
	ToolboxInterface
{
	Q_OBJECT
	Q_INTERFACES(BackupInterface)
	Q_INTERFACES(BaseInterface)
	Q_INTERFACES(LoadSaveInterface)
	Q_INTERFACES(LoggingInterface)
	Q_INTERFACES(ToolboxInterface)

#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.OpenFlipper.Plugins.Plugin-Align")
#endif

public:
	AlignPlugin();

signals:
	// BackupInterface
	void createBackup(int objectId, QString name, UpdateType type = UPDATE_ALL);

	// BaseInterface
	void updateView();
	void updatedObject(int objectId, UpdateType const& type);

	// LoggingInterface
	void log(Logtype type, QString message);
	void log(QString message);

	// ToolboxInterface
	void addToolbox(QString name, QWidget *widget, QIcon *icon);

private slots:
	// BaseInterface
	void initializePlugin();
	void pluginsInitialized();
	void slotObjectUpdated(int, UpdateType const&);
	void slotObjectSelectionChanged(int);

	// LoadSaveInterface
	void objectDeleted(int objectId);

private slots:
	void slotAlignMeshes();

	void slotSourceMeshChanged(int objectId);
	void slotTargetMeshChanged(int objectId);

private:
	void updateGui();

	QComboBox *sourceMeshCombo_;
	ObjectInfoListModel<Is_source_mesh> sourceMeshList_;

	QComboBox *targetMeshCombo_;
	ObjectInfoListModel<Is_target_mesh> targetMeshList_;

	QCheckBox *doScaleCheckBox_;
	QPushButton *alignMeshesButton_;

private:
	typedef std::pair<Eigen::Vector3d, Eigen::Vector3d> BBox;
	typedef std::vector<Eigen::Vector3d> Point_container;

	Point_container getPoints(BaseObjectData *object);
	void transformObject(BaseObjectData *object, Eigen::Affine3d const& t);
	void transformPoints(Point_container& points, Eigen::Affine3d const& t);
	BBox computeBoundingBox(Point_container const& points);
	Eigen::Vector3d computeCentroid(Point_container const& points);
	bool computePCA(Point_container const& points, Eigen::Vector3d& centroid, Eigen::Matrix3d& principal_directions);

	int sourceId_;
	int targetId_;

public:
	// BaseInterface
	QString name()
	{
		return QString("Mesh alignment plugin");
	}

	QString description()
	{
		return QString("Align two meshes by using PCA and scaling");
	}

public slots:
	QString version()
	{
		return QString("1.0");
	}
};
