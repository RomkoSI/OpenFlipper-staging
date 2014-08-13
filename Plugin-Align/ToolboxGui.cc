/**
 * @author Vladimir Chalupecky (vladimir.chalupecky@gmail.com)
 */

#include "AlignPlugin.hh"

void AlignPlugin::initializePlugin()
{
	sourceMeshCombo_ = new QComboBox;
	sourceMeshCombo_->setModel(&sourceMeshList_);
	connect(sourceMeshCombo_, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(slotSourceMeshChanged(int)));

	targetMeshCombo_ = new QComboBox;
	targetMeshCombo_->setModel(&targetMeshList_);
	connect(targetMeshCombo_, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(slotTargetMeshChanged(int)));

	doScaleCheckBox_ = new QCheckBox(tr("Scale to target's bounding box"));
	doScaleCheckBox_->setChecked(true);

	alignMeshesButton_ = new QPushButton(tr("Align meshes"));
	connect(alignMeshesButton_, SIGNAL(clicked()),
	        this, SLOT(slotAlignMeshes()));

	QFormLayout *layout = new QFormLayout;
	layout->addRow(new QLabel("Source mesh"), sourceMeshCombo_);
	layout->addRow(new QLabel("Target mesh"), targetMeshCombo_);
	layout->addRow(doScaleCheckBox_);
	layout->addRow(alignMeshesButton_);

	QWidget *toolbox = new QWidget;
	toolbox->setLayout(layout);

	updateGui();

	emit addToolbox(tr("Align"), toolbox, NULL);
}

void AlignPlugin::pluginsInitialized()
{
}

void AlignPlugin::slotSourceMeshChanged(int objectId)
{
	sourceId_ = sourceMeshCombo_->itemData(sourceMeshCombo_->currentIndex()).toInt();
	updateGui();
}

void AlignPlugin::slotTargetMeshChanged(int objectId)
{
	targetId_ = targetMeshCombo_->itemData(targetMeshCombo_->currentIndex()).toInt();
	updateGui();
}

void AlignPlugin::slotObjectSelectionChanged(int)
{
	sourceMeshList_.refresh();
	targetMeshList_.refresh();
	updateGui();
}

void AlignPlugin::slotObjectUpdated(int, UpdateType const&)
{
	sourceMeshList_.refresh();
	targetMeshList_.refresh();
	updateGui();
}

void AlignPlugin::objectDeleted(int objectId)
{
	sourceMeshList_.removeObject(objectId);
	targetMeshList_.removeObject(objectId);

	if (objectId == sourceId_) {
		sourceId_ = -1;
		sourceMeshCombo_->setCurrentIndex(-1);
	}

	if (objectId == targetId_) {
		targetId_ = -1;
		targetMeshCombo_->setCurrentIndex(-1);
	}

	updateGui();
}

void AlignPlugin::updateGui()
{
	alignMeshesButton_->setEnabled(sourceId_ != -1 && targetId_ != -1);
}
