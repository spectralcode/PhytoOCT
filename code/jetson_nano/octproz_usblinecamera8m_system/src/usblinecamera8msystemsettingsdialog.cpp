
#include "usblinecamera8msystemsettingsdialog.h"

USBLineCamera8MSystemSettingsDialog::USBLineCamera8MSystemSettingsDialog(QWidget *parent)
	: ui(new Ui::USBLineCamera8MSystemSettingsDialog) //QDialog(parent)
{
	ui->setupUi(this);
	initGui();
}

USBLineCamera8MSystemSettingsDialog::~USBLineCamera8MSystemSettingsDialog()
{
}

void USBLineCamera8MSystemSettingsDialog::setSettings(QVariantMap settings){
	this->ui->spinBox_width->setValue(settings.value(WIDTH).toInt());
	this->ui->spinBox_height->setValue(settings.value(HEIGHT).toInt());
	this->ui->spinBox_depth->setValue(settings.value(DEPTH).toInt());
	this->ui->spinBox_buffersPerVolume->setValue(settings.value(BUFFERS_PER_VOLUME).toInt());
	this->getAcquisitionSettingsFromGui();
}

void USBLineCamera8MSystemSettingsDialog::getSettings(QVariantMap* settings) {
	settings->insert(WIDTH, this->ui->spinBox_width->value());
	settings->insert(HEIGHT, this->ui->spinBox_height->value());
	settings->insert(DEPTH, this->ui->spinBox_depth->value());
	settings->insert(BUFFERS_PER_VOLUME, this->ui->spinBox_buffersPerVolume->value());
}

qint32 USBLineCamera8MSystemSettingsDialog::getDeviceIndex()
{
	return this->ui->comboBox_devices->currentIndex();
}

void USBLineCamera8MSystemSettingsDialog::initGui(){
	this->setWindowTitle(tr("USB Line Camera 8M OCT System"));
	connect(this->ui->okButton, &QPushButton::clicked, this, &USBLineCamera8MSystemSettingsDialog::getAcquisitionSettingsFromGui);
	connect(this->ui->pushButton_enumerate, &QPushButton::clicked, this, &USBLineCamera8MSystemSettingsDialog::enumerateClicked);
	connect(this->ui->pushButton_open, &QPushButton::clicked, this, &USBLineCamera8MSystemSettingsDialog::openClicked);
	connect(this->ui->pushButton_savesettings, &QPushButton::clicked, this, &USBLineCamera8MSystemSettingsDialog::saveSettingsClicked);
	this->connectAllGuiElementsToGetCameraSettingsFromGuiSlot();
}

void USBLineCamera8MSystemSettingsDialog::connectAllGuiElementsToGetCameraSettingsFromGuiSlot()
{
	connect(this->ui->spinBox_integrationTime, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	connect(this->ui->spinBox_delay, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	connect(this->ui->spinBox_triggerTime, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	connect(this->ui->spinBox_framesDeviceBuffer, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);

	connect(this->ui->comboBox_acquisitionMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	connect(this->ui->comboBox_adcRange, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	connect(this->ui->comboBox_imageNumbering, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	connect(this->ui->comboBox_bitDepth, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);

	connect(this->ui->horizontalSlider_gain, &QSlider::valueChanged, this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	connect(this->ui->horizontalSlider_offset, &QSlider::valueChanged, this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
}

void USBLineCamera8MSystemSettingsDialog::disconnectAllGuiElementsToGetCameraSettingsFromGuiSlot()
{
	disconnect(this->ui->spinBox_integrationTime, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	disconnect(this->ui->spinBox_delay, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	disconnect(this->ui->spinBox_triggerTime, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	disconnect(this->ui->spinBox_framesDeviceBuffer, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);

	disconnect(this->ui->comboBox_acquisitionMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	disconnect(this->ui->comboBox_adcRange, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	disconnect(this->ui->comboBox_imageNumbering, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	disconnect(this->ui->comboBox_bitDepth, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);

	disconnect(this->ui->horizontalSlider_gain, &QSlider::valueChanged, this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
	disconnect(this->ui->horizontalSlider_offset, &QSlider::valueChanged, this, &USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui);
}

void USBLineCamera8MSystemSettingsDialog::updateCameraSettingsInGui(const usblinecamera8mSettings cameraSettings)
{
	this->disconnectAllGuiElementsToGetCameraSettingsFromGuiSlot();

	this->ui->spinBox_integrationTime->setValue(cameraSettings.integrationTimeMicroSeconds);
	QCoreApplication::processEvents();

	this->ui->spinBox_triggerTime->setValue(cameraSettings.softTrigTime);
	QCoreApplication::processEvents();

	this->ui->spinBox_delay->setValue(cameraSettings.externalDelayMicroSeconds);
	QCoreApplication::processEvents();

	this->ui->comboBox_acquisitionMode->setCurrentIndex(cameraSettings.acquisitionMode);
	QCoreApplication::processEvents();

	this->ui->comboBox_adcRange->setCurrentIndex(cameraSettings.adcRangeId);
	QCoreApplication::processEvents();

	this->ui->horizontalSlider_gain->setValue(cameraSettings.gainRegisterValue);
	this->ui->label_gain->setText(cameraSettings.gainInVperV);
	QCoreApplication::processEvents();

	this->ui->horizontalSlider_offset->setValue(cameraSettings.offsetRegisterValue);
	this->ui->label_offset->setText(cameraSettings.offsetInmV);
	QCoreApplication::processEvents();

	this->ui->comboBox_bitDepth->setCurrentIndex(cameraSettings.bitMode);
	QCoreApplication::processEvents();

	this->ui->spinBox_framesDeviceBuffer->setValue(cameraSettings.numberOfFramesToBeBuffered);
	QCoreApplication::processEvents();

	this->ui->comboBox_imageNumbering->setCurrentIndex(cameraSettings.useImageNumbering);

	this->connectAllGuiElementsToGetCameraSettingsFromGuiSlot();
}

void USBLineCamera8MSystemSettingsDialog::updateDeviceComboBox(QStringList devices)
{
	this->ui->comboBox_devices->clear();;
	this->ui->comboBox_devices->addItems(devices);
}

void USBLineCamera8MSystemSettingsDialog::enableGui(bool enable){
	this->ui->spinBox_width->setEnabled(enable);
	this->ui->spinBox_height->setEnabled(enable);
	this->ui->spinBox_depth->setEnabled(enable);
	this->ui->spinBox_buffersPerVolume->setEnabled(enable);
}

void USBLineCamera8MSystemSettingsDialog::showDeviceInformation(QString infoString)
{
	this->ui->textEdit_deviceInfo->clear();
	this->ui->textEdit_deviceInfo->setText(infoString);
}

void USBLineCamera8MSystemSettingsDialog::updateOffsetString(QString offset)
{
	this->ui->label_offset->setText(offset);
}

void USBLineCamera8MSystemSettingsDialog::updateGainString(QString gain)
{
	this->ui->label_gain->setText(gain);
}

void USBLineCamera8MSystemSettingsDialog::getAcquisitionSettingsFromGui()
{
	this->acqParams.bitDepth = 8+8*this->ui->comboBox_bitDepth->currentIndex();
	this->acqParams.samplesPerLine = this->ui->spinBox_width->value();
	this->acqParams.ascansPerBscan = this->ui->spinBox_height->value();
	this->acqParams.bscansPerBuffer = this->ui->spinBox_depth->value();
	this->acqParams.buffersPerVolume = this->ui->spinBox_buffersPerVolume->value();
	emit acquisitionSettingsChanged(this->acqParams);
}

void USBLineCamera8MSystemSettingsDialog::getCameraSettingsFromGui()
{
	this->cameraParams.integrationTimeMicroSeconds = this->ui->spinBox_integrationTime->value();
	this->cameraParams.softTrigTime = this->ui->spinBox_triggerTime->value();
	this->cameraParams.externalDelayMicroSeconds = this->ui->spinBox_delay->value();
	this->cameraParams.acquisitionMode = this->ui->comboBox_acquisitionMode->currentIndex();
	this->cameraParams.adcRangeId =this->ui->comboBox_adcRange->currentIndex();
	this->cameraParams.gainRegisterValue = this->ui->horizontalSlider_gain->value();
	this->cameraParams.gainInVperV = this->ui->label_gain->text();
	this->cameraParams.offsetRegisterValue = this->ui->horizontalSlider_offset->value();
	this->cameraParams.offsetInmV = this->ui->label_offset->text();
	this->cameraParams.bitMode = this->ui->comboBox_bitDepth->currentIndex();
	this->cameraParams.numberOfFramesToBeBuffered = this->ui->spinBox_framesDeviceBuffer->value();
	this->cameraParams.packetLengthMultiplier = this->ui->spinBox_height->value();
	this->cameraParams.useImageNumbering = this->ui->comboBox_imageNumbering->currentIndex();
	emit cameraSettingsChanged(this->cameraParams);
}
