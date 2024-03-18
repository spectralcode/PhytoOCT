#include "usblinecamera8m.h"

extern "C"
{
	#include "usblc8mjtn.h"
}


USBLineCamera8M::USBLineCamera8M(QObject* parent)
	: QObject(parent),
	  packetLength{DEFAULT_PIXEL_COUNT*2*PACKETLENGTH_MULTIPLIER},
	  octBuffer{nullptr},
	  bytesWritten{0},
	  acquisitionRunning{false},
	  pipe{nullptr}
{
	qint32 ringBufferSizeInBytes = 16*1024*1024;
	ls_initialize(ringBufferSizeInBytes, this->packetLength);
	this->autoOpenDevice();
}

USBLineCamera8M::~USBLineCamera8M()
{
	if (this->acquisitionRunning) {
		this->stopAcquisition();
	}
	this->closeDevice();
}

void USBLineCamera8M::autoOpenDevice()
{
	this->enumerateDevices();
	if(ls_devicecount() > 0){
		this->openDevice(0);
	}
}

void USBLineCamera8M::enumerateDevices()
{
	QStringList deviceList;
	this->closeDevice();
	quint16 cnt = ls_enumdevices();
	if (cnt > 0){
		for(int i=0;i<cnt;i++){
			deviceList.append(QString(ls_getproductname(i))+" "+QString(ls_getserialnumber(i)));
		}
	} else {
		emit cameraError(tr("No devices found!"));
	}
	emit devicesEnumerated(deviceList);
}

void USBLineCamera8M::openDevice(qint32 id)
{
	this->closeDevice();
	int deviceCount = ls_devicecount();
	if(deviceCount > 0 && id >= 0 && id < deviceCount){
		qint32 ierr = ls_opendevicebyindex(id);
		if (ierr == 0){
			this->checkPacketLength(id);
			this->requestDeviceInformation(id);
			this->requestCurrentDeviceSettings();
			emit deviceOpened(true);
		} else {
			emit deviceOpened(false);
			emit cameraError(tr("Could not open device: ") + QString(ls_geterrorstring(ierr)));
		}
	}
}

void USBLineCamera8M::closeDevice()
{
	if (ls_currentdeviceindex()>= 0) {
		this->stoppipethread();
		ls_closedevice();
		emit deviceOpened(false);
		emit deviceInformationReady("");
	}
}

void USBLineCamera8M::startAcquisition()
{
	quint8 state;
	quint8 mode;
	qint32 ierr;

	this->acquisitionRunning = true;
	this->startpipethread(this->currentSettings.pixelCount, this->currentSettings.bitMode);

	ls_getstate(state, 1000);
	if (state == 0) {
		ierr = ls_setstate(0x01, 1000);
		if (ierr == 0) {
			ls_getmode(mode,1000);
			if ((mode == 0)) { //Software trigger (one shot mode)
				return;
			}
			emit acquisitionStarted();
			emit cameraInfo(tr("Acquisition started."));
		} else {
			emit cameraError(tr("Start acquisition error: ") + QString(ls_geterrorstring(ierr)));
		}
	} else {
		emit cameraInfo(tr("Acquisition already started!"));
	}
}

void USBLineCamera8M::stopAcquisition()
{
	quint8 state;
	qint32 ierr;

	ls_getstate(state,1000);
	if (state != 0) {
		this->acquisitionRunning = false;
		QCoreApplication::processEvents();
		ierr = ls_setstate(0x00, 1000);
		if (ierr == 0) {
			emit acquisitionStopped();
			emit cameraInfo(tr("Acquisition stopped."));
			this->stoppipethread();
		} else {
			emit cameraError(tr("Stop acquisition error: ") + QString(ls_geterrorstring(ierr)));
		}
	} else {
		emit cameraInfo(tr("Acquisition already stopped!"));
	}
}

void USBLineCamera8M::setIntegrationTime(qint32 intTimeMicroSeconds)
{
	qint32 ierr;
	quint32 u32tmp;

	ierr = ls_setinttime(intTimeMicroSeconds, 1000);
	if (ierr == 0) {
		if (ls_getinttime(u32tmp, 1000) == 0) {
			this->currentSettings.integrationTimeMicroSeconds = intTimeMicroSeconds;
			emit integrationTimeChanged(u32tmp);
		}
	} else {
		emit cameraError(tr("Could not change integration time: ") + QString(ls_geterrorstring(ierr)));
	}
}

void USBLineCamera8M::setGain(qint16 gainRegisterValue)
{
	float fgain;
	quint16 wPGA;
	int ierr;

	ierr = ls_setadcpga1(gainRegisterValue, 1000);
	if (ierr == 0){
		this->currentSettings.gainRegisterValue = gainRegisterValue;
	}
	ls_getadcpga1(wPGA, 1000);
	fgain = 6 / (1 + (5 *((63 - (float)wPGA)/63)));
	QString gainString = QString::number(fgain,'f',4)+" V/V";
	this->currentSettings.gainInVperV = gainString;
	gainChanged(gainString);
}

void USBLineCamera8M::setOffset(qint16 offsetRegisterValue)
{
	float newValue;
	quint16 wvalue;
	quint16 sign;
	qint32 ierr;

	newValue = (float)offsetRegisterValue * 1.2; //todo: why *1.2 and not *1.17 ?
	newValue = qBound(-300.0f, newValue, 300.0f);

	if (offsetRegisterValue < 0) {
		sign = 0x0100;
	} else {
		sign = 0;
	}
	wvalue = abs(offsetRegisterValue) | sign;

	ierr = ls_setadcoffset1(wvalue,1000);
	if(ierr == 0){
		this->currentSettings.offsetRegisterValue = offsetRegisterValue;
		QString offsetString = QString::number(newValue,'f',1)+" mV";
		this->currentSettings.offsetInmV = offsetString;
		offsetChanged(offsetString);
	} else {
		emit cameraError(tr("Could not set offset: ") + QString(ls_geterrorstring(ierr)));
	}
}

void USBLineCamera8M::setAcquisitionMode(qint32 modeId)
{
	qint32 ierr;
	ierr = ls_setmode(modeId, 1000);
	if ((ierr != 0)){
		emit cameraError(tr("Could not set acquisition mode: ") + QString(ls_geterrorstring(ierr)));
	} else {
		this->currentSettings.acquisitionMode = modeId;
	}
}

void USBLineCamera8M::setInputRange(qint16 rangeId)
{
	qint32 ierr;
	ierr = ls_setadcconfig(rangeId << 7, 1000);
	if ((ierr != 0)){
		emit cameraError(tr("Could not set input range: ") + QString(ls_geterrorstring(ierr)));
	} else {
		this->currentSettings.adcRangeId = rangeId;
	}
}

void USBLineCamera8M::setNumberOfLinesToGetPerRead(qint32 packetLengthMultiplier)
{
	qint32 ierr ;
	qint32 recommendedPacketLength; //size of one single line in byters (number of sensor pixels * bitDepth/2)
	qint32 newPacketLength;

	ierr = ls_getpacketlength(recommendedPacketLength, 1000);
	if (ierr == 0) {
		newPacketLength = recommendedPacketLength * packetLengthMultiplier;
		int currentDevice = ls_currentdeviceindex();
		if(currentDevice >= 0){
			ls_closedevice();
		}

		ls_setpacketlength(newPacketLength);

		if(currentDevice >= 0){
			ierr = ls_opendevicebyindex(currentDevice);
			if (ierr == 0) {
				this->packetLength = newPacketLength;
				this->currentSettings.packetLengthMultiplier = packetLengthMultiplier;
			} else {
				emit deviceOpened(false);
				emit cameraError(tr("Could not open device: ") + QString(ls_geterrorstring(ierr)));
				return;
			}
		}
	} else {
		emit cameraError(tr("GET PACKETLENGTH error while opening device: ") + QString(ls_geterrorstring(ierr)));
	}

}

void USBLineCamera8M::saveSettingsToEEPROM()
{
	qint32 ierr;
	ierr = ls_savesettings(1000);
	if (ierr != 0){
		emit cameraError(tr("Could not save Settings to EEPROM: ") + QString(ls_geterrorstring(ierr)));
	} else {
		emit cameraInfo(tr("Settings saved to EEPROM."));
	}
}

void USBLineCamera8M::requestDeviceInformation(qint32 id)
{
	qint32 ierr;
	quint16 u16tmp;
	qint32 i32tmp;
	quint32 u32tmp;
	quint16 sensor;
	quint16 pixel;

	QString vendorName = ls_getvendorname(id);
	QString productName = ls_getproductname(id);
	QString serial = ls_getserialnumber(id);

	u16tmp = ls_getfwversion(id);
	QString firmwareVersion = QString::number((u16tmp >> 8) & 0xFF)+"."+QString::number(u16tmp & 0xFF);

	QString sensorName = "";
	QString pixelCount = "";
	ierr = ls_getsensortype(sensor, pixel, 1000);
	if (ierr == 0) {
		sensorName = "0x"+QString::number(sensor,16)+" <"+ls_getsensorname(sensor)+">";
		pixelCount = QString::number(pixel);
	}

	QString mcu2sensor = "";
	ierr = ls_getmcusensortype(sensor, 1000);
	if (ierr == 0) {
		mcu2sensor = "0x"+QString::number(sensor,16)+" <"+ls_getsensorname(sensor)+">";
	}

	QString recommendedPacketLength = "";
	ierr = ls_getpacketlength(i32tmp, 1000);
	if (ierr == 0) {
		recommendedPacketLength = QString::number(i32tmp)+" Bytes";
	}

	QString customFirmware = "";
	ierr = ls_customfirmware(u32tmp, 1000);
	if (ierr == 0) {
		customFirmware = QString::number(u32tmp);
	}

	u16tmp = ls_libversion();
	QString libVersion = QString::number((u16tmp >> 8) & 0xFF)+"."+QString::number(u16tmp & 0xFF);

	QString infoString = tr("Vendor Name: ") + vendorName + "\n"
							+ tr("Product name: ") + productName + "\n"
							+ tr("Serial number: ") + serial+ "\n"
							+ tr("Firmware version: ") + firmwareVersion + "\n"
							+ tr("Sensor: ") + sensorName + "\n"
							+ tr("MCU Sensor: ") + mcu2sensor + "\n"
							+ tr("Pixel count: ") + pixelCount + "\n"
							+ tr("Reccommended packet length: ") + recommendedPacketLength + "\n"
							+ tr("Custom firmware: ") + customFirmware + "\n"
							+ tr("Library version: ") +libVersion + "\n";

	emit deviceInformationReady(infoString);
}

void USBLineCamera8M::setExternalDelayTime(qint32 timeMicroSeconds)
{
	qint32 ierr;
	quint32 u32tmp;

	ierr = ls_setextdelay(timeMicroSeconds, 1000);
	if (ierr == 0) {
		if (ls_getextdelay(u32tmp, 1000) == 0) {
			this->currentSettings.externalDelayMicroSeconds = timeMicroSeconds;
			emit externalDelayTimeChanged(QString::number(u32tmp));
		}
	} else {
		emit cameraError(tr("Could not set external delay time: ") + QString(ls_geterrorstring(ierr)));
	}
}

void USBLineCamera8M::setCFG1(qint32 useImageNumbering, qint32 bitMode, qint32 numberOfFramesToBeBuffered)
{
	qint32 ierr;
	quint8 uccfg1;

	uccfg1 = (numberOfFramesToBeBuffered & 0x1F)-1;
	uccfg1 = (uccfg1 << 1) | (bitMode & 0x01);
	uccfg1 = (uccfg1 << 1) | (useImageNumbering & 0x01);
	ierr = ls_setcfg1(uccfg1, 1000);
	if (ierr == 0) {
		this->currentSettings.useImageNumbering = useImageNumbering;
		this->currentSettings.bitMode = bitMode;
		this->currentSettings.numberOfFramesToBeBuffered = numberOfFramesToBeBuffered;
		emit cameraInfo(tr("Changes takes effect after reboot - Power OFF/ON")); //todo: save to eeprom needed?
	} else {
		emit cameraError(tr("Could not set CFG1: ") + QString(ls_geterrorstring(ierr)));
	}
}

void USBLineCamera8M::setInternalSoftwareTriggerTime(qint32 trigTimeMicroSeconds)
{
	qint32 ierr;
	quint32 u32tmp;

	ierr = ls_setsofttrigtime(trigTimeMicroSeconds,0);
	if (ierr == 0) {
		if (ls_getsofttrigtime(u32tmp,1000) == 0) {
			this->currentSettings.softTrigTime = trigTimeMicroSeconds;
			emit internalSoftwareTriggerTimeChanged(QString::number(u32tmp));
		}
	} else {
		emit cameraError(tr("Could not set internal software trigger time: ") + QString(ls_geterrorstring(ierr)));

	}
}

void USBLineCamera8M::requestCurrentDeviceSettings()
{
	quint16 bcdversion;
	quint8 u8tmp;
	quint16 u16tmp;
	quint16 u16tmp2;
	quint32 u32tmp;
	qint16  i16tmp;
	float ftmp;

	//get sensor id, name and pixel count
	this->currentSettings.deviceId = ls_currentdeviceindex();
	if (ls_getsensortype(u16tmp, u16tmp2, 1000) == 0) {
		this->currentSettings.sensorType = u16tmp;
		this->currentSettings.pixelCount = u16tmp2;
	} else {
		this->currentSettings.sensorType = -1;
		this->currentSettings.pixelCount = -1;
	}

	//get internal software trigger time (firmware 1v20 and later)
	bcdversion = ls_getfwversion(ls_currentdeviceindex());
	if (bcdversion >= 0x0114) {
		if (ls_getsofttrigtime(u32tmp, 1000) == 0) {
			this->currentSettings.softTrigTime = u32tmp;
		}
	} else {
		this->currentSettings.softTrigTime = -1;
	}

	//get acquisition state
	if (ls_getstate(u8tmp, 1000) == 0) {
		if (u8tmp == 0) {
			this->currentSettings.acquisitionRunning = false;
		} else {
			this->currentSettings.acquisitionRunning = true;
		}
	}

	//get acquisition mode
	if (ls_getmode(u8tmp, 1000) == 0) {
		this->currentSettings.acquisitionMode = u8tmp;
	}

	//get integration time
	if (ls_getinttime(u32tmp,1000) == 0) {
		this->currentSettings.integrationTimeMicroSeconds = u32tmp;
	}

	//get external delay
	if (ls_getextdelay(u32tmp, 1000) == 0) {
		this->currentSettings.externalDelayMicroSeconds = u32tmp;
	}

	//get input range of adc
	if (ls_getadcconfig(u16tmp, 1000) == 0) {
		this->currentSettings.adcRangeId = (u16tmp >> 7) & 0x01;
	}

	//get gain of programmable gain amplifier (PGA)
	if (ls_getadcpga1(u16tmp, 1000) == 0) {
		this->currentSettings.gainRegisterValue = u16tmp;
		this->currentSettings.gainInVperV = QString::number(6 / (1+(5*((63- (float)u16tmp)/63))),'f',4)+" V/V";
	}

	//get offset
	if (ls_getadcoffset1(u16tmp,1000) == 0) {
		i16tmp = u16tmp & 0xFF;
		if ((u16tmp & 0x0100) == 0x0100) {
			i16tmp = i16tmp * (-1);
		}
		this->currentSettings.offsetRegisterValue = i16tmp;
		ftmp = (float)i16tmp * 1.2;
		ftmp = qBound(-300.0f, ftmp, 300.0f);
		this->currentSettings.offsetInmV = QString::number(ftmp,'f',1)+" mV";
	}

	//get values of configuration register 1 (CFG1): use image numbering, bit mode, number of images to be bufferede before transferring to host
	if (ls_getcfg1(u8tmp,1000) == 0) {
		this->currentSettings.useImageNumbering = (u8tmp & 0x01);
		this->currentSettings.bitMode = (u8tmp & 0x02) >> 1;
		this->currentSettings.numberOfFramesToBeBuffered = (((u8tmp & 0x7C) >> 2)+1);
	}

	emit currentDeviceSettingsReady(currentSettings);
}

void USBLineCamera8M::handleIncommingCameraData(quint16* buf, qint32 bytesRead, qint32 bytesAvailable, quint32 fps)
{
	Q_UNUSED(bytesAvailable);
	Q_UNUSED(fps);

	if(!this->acquisitionRunning){
		QCoreApplication::processEvents();
		return;
	}

	//check if oct buffer is available
	if (this->octBuffer == nullptr) {
		emit cameraError(tr("Acquisition buffer not set."));
		QCoreApplication::processEvents();
		return;
	}

	//if one acquisition buffer was filled bytesWritten will be 0 and we need to update the index of the acquisition buffer
	int bufferIndex = this->octBuffer->currIndex;
	if (this->bytesWritten == 0) {
		//calculate index of next buffer
		 bufferIndex = (this->octBuffer->currIndex+1)%2;

		//set acquisition buffer index, so that processing thread knows where to find the new data
		this->octBuffer->currIndex = bufferIndex;
	}

	//check bufferReadyArray flag to see if proccessing thread is still using it. we only want to modify the buffer if the buffer is not used by the processing thread anymore, in this case the bufferReadyArray is false and it is safe to write new data to the buffer
	if (this->octBuffer->bufferReadyArray[bufferIndex] == false) {

		//ensure that we do not copy more data than is available in the acquisition buffer
		size_t bytesAvailable = this->octBuffer->bytesPerBuffer - this->bytesWritten;
		size_t bytesToWrite = qMin((size_t)bytesRead, bytesAvailable);
		if (bytesToWrite < bytesRead) {
			emit cameraError(tr("Data loss during data acquisition occured."));
			QCoreApplication::processEvents();
		}

		//copy new data to oct buffer
		memcpy(static_cast<char*>(this->octBuffer->bufferArray[bufferIndex]) + this->bytesWritten, buf, bytesToWrite);

		//update bytesWritten. bytesWritten will be 0 as soon as the acquisition buffer is completely filled
		this->bytesWritten = (this->bytesWritten + bytesToWrite) % this->octBuffer->bytesPerBuffer;

		//check if acquisition buffer is completely filled
		if (this->bytesWritten == 0){

			//acquisition buffer is now complete. To indicate that it can be used by the processing thread we need to set the bufferReadyArray flag to true
			this->octBuffer->bufferReadyArray[bufferIndex] = true;

		}
	}
	QCoreApplication::processEvents();
}

void USBLineCamera8M::setOctAcquisitionBufferPointer(AcquisitionBuffer* buffer)
{
	this->octBuffer = buffer;
}

void USBLineCamera8M::updateDeviceSettings(const usblinecamera8mSettings newSettings)
{
	if (this->currentSettings != newSettings){
		if(this->currentSettings.softTrigTime != newSettings.softTrigTime) {
			this->setInternalSoftwareTriggerTime(newSettings.softTrigTime);
		}
		if(this->currentSettings.acquisitionMode != newSettings.acquisitionMode) {
			this->setAcquisitionMode(newSettings.acquisitionMode);
		}
		if(this->currentSettings.integrationTimeMicroSeconds != newSettings.integrationTimeMicroSeconds) {
			this->setIntegrationTime(newSettings.integrationTimeMicroSeconds);
		}
		if(this->currentSettings.externalDelayMicroSeconds != newSettings.externalDelayMicroSeconds) {
			this->setExternalDelayTime(newSettings.externalDelayMicroSeconds);
		}
		if(this->currentSettings.adcRangeId != newSettings.adcRangeId) {
			this->setInputRange(newSettings.adcRangeId);
		}
		if(this->currentSettings.gainRegisterValue != newSettings.gainRegisterValue) {
			this->setGain(newSettings.gainRegisterValue);
		}
		if(this->currentSettings.offsetRegisterValue != newSettings.offsetRegisterValue) {
			this->setOffset(newSettings.offsetRegisterValue);
		}
		if(this->currentSettings.bitMode != newSettings.bitMode || this->currentSettings.useImageNumbering != newSettings.useImageNumbering || this->currentSettings.numberOfFramesToBeBuffered != newSettings.numberOfFramesToBeBuffered) {
			this->setCFG1(newSettings.useImageNumbering, newSettings.bitMode, newSettings.numberOfFramesToBeBuffered);
		}
		if(this->currentSettings.packetLengthMultiplier != newSettings.packetLengthMultiplier) {
			this->setNumberOfLinesToGetPerRead(newSettings.packetLengthMultiplier);
		}
	}
}

void USBLineCamera8M::startpipethread(quint16 pixelCount, qint8 bitMode)
{
	this->pipe = new PipeThread(this);
	this->pipe->is16bit = bitMode;
	//this->pipe->BufSize = pixelCount * (bitMode + 1);
	this->pipe->BufSize = this->packetLength;
	connect(this->pipe, &PipeThread::DataChanged, this, &USBLineCamera8M::handleIncommingCameraData); //todo: figure out why this plulgin cant be loades by OCTproZ if this connect is used but with the old-stlyle connect it works
	//connect(this->pipe,SIGNAL(DataChanged(quint16*,qint32,qint32,quint32)),this,SLOT(handleIncommingCameraData(quint16*,qint32,qint32,quint32)));
	connect(this->pipe, &PipeThread::finished, this->pipe, &PipeThread::deleteLater);
	this->pipe->start();
}

void USBLineCamera8M::stoppipethread()
{
	if (this->pipe != nullptr) {
		if (this->pipe->isRunning()) {
			this->pipe->stopthread();
			this->pipe->wait();
		}
	}
}

void USBLineCamera8M::checkPacketLength(int deviceId)
{
	qint32 ierr ;
	qint32 tmpPacketLength;

	ierr = ls_getpacketlength(tmpPacketLength, 1000);
	if (ierr == 0) {
		if (tmpPacketLength != (this->packetLength / PACKETLENGTH_MULTIPLIER)) {
			this->packetLength = (tmpPacketLength * PACKETLENGTH_MULTIPLIER);
			ls_closedevice();
			ls_setpacketlength(this->packetLength);
			ierr = ls_opendevicebyindex(deviceId);
			if (ierr != 0) {
				emit deviceOpened(false);
				emit cameraError(tr("Could not open device: ") + QString(ls_geterrorstring(ierr)));
				return;
			}
		}
	} else {
		emit cameraError(tr("GET PACKETLENGTH error while opening device: ") + QString(ls_geterrorstring(ierr)));
	}
}



