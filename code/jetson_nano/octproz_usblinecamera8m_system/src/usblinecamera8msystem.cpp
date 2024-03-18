#include "usblinecamera8msystem.h"
#include <QMetaObject>


USBLineCamera8MSystem::USBLineCamera8MSystem()
	: AcquisitionSystem(),
	  systemDialog{new USBLineCamera8MSystemSettingsDialog},
	  camera{new USBLineCamera8M()}
{
	this->settingsDialog = static_cast<QDialog*>(this->systemDialog);
	this->name = "USB Line Sensor M8 System";
	this->setType((PLUGIN_TYPE)SYSTEM);

	this->octSerialCom = this->systemDialog->findChild<OCTSerialCom*>();
	connect(this->octSerialCom, &OCTSerialCom::startRecordingRequested, this, &Plugin::startRecordingRequest);

	qRegisterMetaType<usblinecamera8mSettings >("usblinecamera8mSettings");

	this->camera->moveToThread(&cameraThread);
	connect(this->camera, &USBLineCamera8M::cameraError, this, &Plugin::error);
	connect(this->camera, &USBLineCamera8M::cameraInfo, this, &Plugin::info);
	connect(&cameraThread, &QThread::finished, this->camera, &USBLineCamera8M::deleteLater);
	connect(this->camera, &USBLineCamera8M::currentDeviceSettingsReady, this->systemDialog, &USBLineCamera8MSystemSettingsDialog::updateCameraSettingsInGui);
	connect(this->systemDialog, &USBLineCamera8MSystemSettingsDialog::enumerateClicked, this->camera, &USBLineCamera8M::enumerateDevices);
	connect(this->camera, &USBLineCamera8M::devicesEnumerated, this->systemDialog, &USBLineCamera8MSystemSettingsDialog::updateDeviceComboBox);
	connect(this->systemDialog, &USBLineCamera8MSystemSettingsDialog::openClicked, this, [&](){
		QMetaObject::invokeMethod(this->camera, "openDevice", Qt::QueuedConnection, Q_ARG(qint32, this->systemDialog->getDeviceIndex()));
	});
	connect(this->systemDialog, &USBLineCamera8MSystemSettingsDialog::saveSettingsClicked, this->camera, &USBLineCamera8M::saveSettingsToEEPROM);
	connect(this->camera, &USBLineCamera8M::deviceInformationReady, this->systemDialog, &USBLineCamera8MSystemSettingsDialog::showDeviceInformation);
//	connect(this->camera, &USBLineCamera8M::acquisitionStarted, this, [&](){
//		emit acquisitionStarted(this);
//	});
	connect(this->camera, &USBLineCamera8M::acquisitionStarted, this, [&](){
		this->systemDialog->enableGui(false);
	});
	connect(this->camera, &USBLineCamera8M::acquisitionStopped, this, [&](){
		this->systemDialog->enableGui(true);
	});
	connect(this->camera, &USBLineCamera8M::acquisitionStopped, this, &AcquisitionSystem::acquisitionStopped);
	connect(this->camera, &USBLineCamera8M::gainChanged, this->systemDialog, &USBLineCamera8MSystemSettingsDialog::updateGainString);
	connect(this->camera, &USBLineCamera8M::offsetChanged, this->systemDialog, &USBLineCamera8MSystemSettingsDialog::updateOffsetString);
	cameraThread.start();

	connect(this->systemDialog, &USBLineCamera8MSystemSettingsDialog::acquisitionSettingsChanged, this, &USBLineCamera8MSystem::onOctDimensionsChanged);
	connect(this->systemDialog, &USBLineCamera8MSystemSettingsDialog::cameraSettingsChanged, this, &USBLineCamera8MSystem::onCameraSettingsChanged);
}

USBLineCamera8MSystem::~USBLineCamera8MSystem() {
	cameraThread.quit();
	cameraThread.wait();
}

bool USBLineCamera8MSystem::init() {
	bool success = false;

	//allocate buffer memory
	size_t bufferSize = this->currOctParams.samplesPerLine*this->currOctParams.ascansPerBscan*this->currOctParams.bscansPerBuffer*ceil((double)this->currOctParams.bitDepth / 8.0);
	success = this->buffer->allocateMemory(2, bufferSize);

	//set acquisition buffer pointer in camera object
	QMetaObject::invokeMethod(this->camera, "setOctAcquisitionBufferPointer", Qt::QueuedConnection, Q_ARG(AcquisitionBuffer*, this->buffer));

	return success;
}

void USBLineCamera8MSystem::uninit()
{
	this->buffer->releaseMemory();
}

void USBLineCamera8MSystem::startAcquisition(){
	//init acquisition
	bool initSuccessfull = this->init();
	if(!initSuccessfull){
		this->uninit();
		return;
	}

	//start acquisition
	QMetaObject::invokeMethod(this->camera, "startAcquisition", Qt::QueuedConnection);
	QCoreApplication::processEvents();
	this->acqusitionRunning = true;
	emit acquisitionStarted(this);
}

void USBLineCamera8MSystem::stopAcquisition(){
	//stop acquisition and unitialize acquisition system
	this->acqusitionRunning = false;
	QMetaObject::invokeMethod(this->camera, "stopAcquisition", Qt::BlockingQueuedConnection); //BlockingQueuedConnection is necessary here to ensure that acquisition is stopped before deleting buffer in uninit()
	this->uninit();
}

void USBLineCamera8MSystem::settingsLoaded(QVariantMap settings){
	this->systemDialog->setSettings(settings);
}


void USBLineCamera8MSystem::onCameraSettingsChanged(usblinecamera8mSettings newParams){
	this->currCameraParams = newParams;

	//oct dimensions
	this->params->params.bitDepth = newParams.bitMode*8+8; //camera bit detph can be 8 or 16. the value of newParams.bitMode is 0 or 1
	this->params->slot_updateParams(this->params->params);

	//pass camera settings to camera
	this->currCameraParams = newParams;
	QMetaObject::invokeMethod(this->camera, "updateDeviceSettings", Qt::QueuedConnection, Q_ARG(usblinecamera8mSettings, this->currCameraParams));
}

void USBLineCamera8MSystem::onOctDimensionsChanged(AcquisitionParams newOctParams)
{
	this->currOctParams = newOctParams;
	AcquisitionParams params;
	params.samplesPerLine = newOctParams.samplesPerLine;
	params.ascansPerBscan = newOctParams.ascansPerBscan;
	params.bscansPerBuffer = newOctParams.bscansPerBuffer;
	params.buffersPerVolume = newOctParams.buffersPerVolume;
	params.bitDepth = newOctParams.bitDepth;
	this->params->slot_updateParams(params);

	//update packet length multiplier of camera (this tells the camera how many lines to transfer per read)
	this->camera->setNumberOfLinesToGetPerRead(params.ascansPerBscan);

	//store settings, so settings can be reloaded into gui at next start of application
	this->systemDialog->getSettings(&this->settingsMap);
	emit storeSettings(this->name, this->settingsMap);
}
