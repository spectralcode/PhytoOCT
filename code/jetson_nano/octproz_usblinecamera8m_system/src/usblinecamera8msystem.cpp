#include "usblinecamera8msystem.h"
#include <QMetaObject>
#include <QTimer>


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
	connect(this->camera, &USBLineCamera8M::deviceOpened, this, [this](bool opened){
		if(opened && this->currOctParams.ascansPerBscan > 0){
			QMetaObject::invokeMethod(this->camera, "setNumberOfLinesToGetPerRead",
				Qt::QueuedConnection, Q_ARG(qint32, this->currOctParams.ascansPerBscan));
		}
	});
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
	this->sendSerialCommand(this->shutdownCmd);
	cameraThread.quit();
	cameraThread.wait();
}

void USBLineCamera8MSystem::sendSerialCommand(const QString &cmd){
	if(cmd.isEmpty() || !this->octSerialCom){
		return;
	}
	// AutoConnection: direct if same thread, queued if different thread.
	// Required because OCTSerialCom lives on the GUI thread but this system
	// is moved to the acquisition thread when the user activates the plugin.
	QMetaObject::invokeMethod(this->octSerialCom, "sendCommand",
		Qt::AutoConnection, Q_ARG(QString, cmd));
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
	// Re-fire the prestart command (e.g. "sld=1") on every Start so the SLD comes
	// back on after a previous Stop, and as a safety net in case the launch-time
	// send was lost. cuda_init_delay_ms below doubles as the SLD warmup window.
	this->sendSerialCommand(this->prestartCmd);

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

	// Fire the external trigger (e.g. "run=1") after CUDA has had time to finish
	// initialising on the processing thread. Workaround until the main app
	// exposes a synchronous cudaReady() signal we can wait on.
	QTimer::singleShot(this->cudaInitDelayMs, this, [this](){
		if(this->acqusitionRunning){
			this->sendSerialCommand(this->triggerStartCmd);
		}
	});
}

void USBLineCamera8MSystem::stopAcquisition(){
	// Stop external triggers first so no new frames arrive after the camera halts.
	this->sendSerialCommand(this->triggerStopCmd);

	//stop acquisition and unitialize acquisition system
	this->acqusitionRunning = false;
	QMetaObject::invokeMethod(this->camera, "stopAcquisition", Qt::BlockingQueuedConnection); //BlockingQueuedConnection is necessary here to ensure that acquisition is stopped before deleting buffer in uninit()
	this->uninit();

	// Turn off the SLD now that triggers have stopped and buffers are released.
	// The destructor still sends this as a fallback if the user closes without Stop first.
	this->sendSerialCommand(this->shutdownCmd);
}

void USBLineCamera8MSystem::settingsLoaded(QVariantMap settings){
	// Restore auto-flow config with sensible defaults so a fresh install still works.
	this->autoConnect     = settings.value(AUTO_CONNECT,       true).toBool();
	this->prestartCmd     = settings.value(PRESTART_CMD,       "sld=1").toString();
	this->triggerStartCmd = settings.value(TRIGGER_START_CMD,  "run=1").toString();
	this->triggerStopCmd  = settings.value(TRIGGER_STOP_CMD,   "run=0").toString();
	this->shutdownCmd     = settings.value(SHUTDOWN_CMD,       "sld=0").toString();
	this->cudaInitDelayMs = settings.value(CUDA_INIT_DELAY_MS, 500).toInt();

	// Preserve the full map so non-UI keys (auto_connect etc.) survive future
	// storeSettings round-trips driven by onOctDimensionsChanged.
	this->settingsMap = settings;

	this->systemDialog->setSettings(settings);

	if(!this->autoConnect){
		return;
	}

	// Auto-connect serial port + send pre-start command (e.g. "sld=1" to enable SLD).
	const QString port = settings.value(COM_PORT).toString();
	qint32 baud = settings.value(BAUD_RATE).toInt();
	if(!port.isEmpty() && baud > 0 && this->octSerialCom){
		QMetaObject::invokeMethod(this->octSerialCom, "connectToComPort",
			Qt::AutoConnection, Q_ARG(QString, port), Q_ARG(qint32, baud));
	}

	// Auto-open the saved camera. If none saved yet, at least enumerate so the
	// user sees the device list in the combo without clicking Enumerate first.
	const QString cameraName = settings.value(CAMERA_SELECTION).toString();
	if(cameraName.isEmpty()){
		QMetaObject::invokeMethod(this->camera, "enumerateDevices", Qt::QueuedConnection);
	} else {
		QMetaObject::invokeMethod(this->camera, "openDeviceByName", Qt::QueuedConnection, Q_ARG(QString, cameraName));
	}
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
	// Queued because this->camera lives on cameraThread.
	QMetaObject::invokeMethod(this->camera, "setNumberOfLinesToGetPerRead",
		Qt::QueuedConnection, Q_ARG(qint32, params.ascansPerBscan));

	//store settings, so settings can be reloaded into gui at next start of application
	this->systemDialog->getSettings(&this->settingsMap);
	emit storeSettings(this->name, this->settingsMap);
}
