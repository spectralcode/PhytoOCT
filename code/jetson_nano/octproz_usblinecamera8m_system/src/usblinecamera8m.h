#ifndef USBLINECAMERA8M_H
#define USBLINECAMERA8M_H

#include <QObject>
#include <cstdint>
#include "pipethread.h"
#include "acquisitionbuffer.h"
#include "usblinecamera8msettings.h"

#define PACKETLENGTH_MULTIPLIER 512
#define DEFAULT_PIXEL_COUNT 4096

class USBLineCamera8M : public QObject
{
	Q_OBJECT

public:
	explicit USBLineCamera8M(QObject *parent = nullptr);
	~USBLineCamera8M();
	PipeThread* pipe;

public slots:
	void autoOpenDevice();
	void enumerateDevices();
	void openDevice(qint32 id);
	void closeDevice();
	void startAcquisition();
	void stopAcquisition();
	void setIntegrationTime(qint32 intTimeMicroSeconds);
	void setGain(qint16 gainRegisterValue); //gainRegisterValue must be a value from 0 to 63. registerValue is approximately linear in dB: 0 is 0 dB, 1 is 0.12 db, 62 is 14.9 db, 63 is 15.56 db
	void setOffset(qint16 offsetRegisterValue); //offsetRegisterValue must be a value from -255 to +255. the actual offset is in the range from -300 mV to +300 mV
	void setAcquisitionMode(qint32 modeId); //0 is software triggered, 1 is external trigger, 2 is free running, 3 is external trigger with integration time determined by time betweed negative edges of trigger signal, 4 is software trigger
	void setInputRange(qint16 rangeId); //0 is 2 V, 1 is 4 V
	void setNumberOfLinesToGetPerRead(qint32 packetLengthMultiplier);
	void saveSettingsToEEPROM();
	void requestDeviceInformation(qint32 id);
	void setExternalDelayTime(qint32 timeMicroSeconds);
	void setCFG1(qint32 useImageNumbering, qint32 bitMode, qint32 numberOfFramesToBeBuffered);//useImageNumbering 0: image numbering is not used, 1: A 4 byte image number is appended at the end of an image data. In 16 bit mode the image number replaces the last 2 pixel values. In 8 bit the last 4 pixel values are replaced //bitMode 0 is 8 bit, 1 is 16 bit //numberOfFramesToBeBuffered number of images to be buffered before transferring to the host. the max number of images depends on the number of pixels. e.g. for 2048 pixel the max number of images is 4 @ 16bit or 8 @ 8bit
	void setInternalSoftwareTriggerTime(qint32 trigTimeMicroSeconds);
	void requestCurrentDeviceSettings();
	void handleIncommingCameraData(quint16* buf, qint32 BytesRead, qint32 BytesAvailable, quint32 fps);
	void setOctAcquisitionBufferPointer(AcquisitionBuffer* buffer);
	void updateDeviceSettings(const usblinecamera8mSettings newSettings);

private:
	qint32 packetLength;
	AcquisitionBuffer* octBuffer;
	size_t bytesWritten;
	usblinecamera8mSettings currentSettings;
	bool acquisitionRunning;

	void startpipethread(quint16 pixelCount, qint8 bitMode);
	void stoppipethread();
	void checkPacketLength(int deviceId);

signals:
	void cameraInfo(QString);
	void cameraError(QString);
	void devicesEnumerated(QStringList);
	void deviceOpened(bool);
	void acquisitionStarted();
	void acquisitionStopped();
	void integrationTimeChanged(qint32 integrationTime);
	void gainChanged(QString);
	void offsetChanged(QString);
	void deviceInformationReady(QString);
	void externalDelayTimeChanged(QString);
	void internalSoftwareTriggerTimeChanged(QString);
	void currentDeviceSettingsReady(usblinecamera8mSettings currSettings);
};

#endif // USBLINECAMERA8M_H
