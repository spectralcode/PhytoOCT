#ifndef USBLINECAMERA8MSETTINGS_H
#define USBLINECAMERA8MSETTINGS_H

#include <QString>
#include <QtCore>


struct usblinecamera8mSettings
{
	Q_GADGET

public:
	int deviceId = -1;
	int sensorType = -1;
	int pixelCount = -1;
	int softTrigTime = -1;
	int acquisitionMode = -1;
	int integrationTimeMicroSeconds = -1;
	int externalDelayMicroSeconds = -1;
	int adcRangeId = -1;
	int gainRegisterValue = -1;
	int offsetRegisterValue = -1;
	int bitMode = -1;
	int numberOfFramesToBeBuffered = -1;
	int packetLengthMultiplier = 320; //todo: figure out how numberOfFramesToBeBuffered is diffrent from packetLengthMultiplier
	bool useImageNumbering = false;
	bool acquisitionRunning = false;
	QString gainInVperV = "";
	QString offsetInmV = "";

	bool operator==(const usblinecamera8mSettings& other) const {
		return deviceId == other.deviceId &&
				sensorType == other.sensorType &&
				pixelCount == other.pixelCount &&
				softTrigTime == other.softTrigTime &&
				acquisitionMode == other.acquisitionMode &&
				integrationTimeMicroSeconds == other.integrationTimeMicroSeconds &&
				externalDelayMicroSeconds == other.externalDelayMicroSeconds &&
				adcRangeId == other.adcRangeId &&
				gainRegisterValue == other.gainRegisterValue &&
				offsetRegisterValue == other.offsetRegisterValue &&
				bitMode == other.bitMode &&
				numberOfFramesToBeBuffered == other.numberOfFramesToBeBuffered &&
				packetLengthMultiplier == other.packetLengthMultiplier &&
				useImageNumbering == other.useImageNumbering &&
				acquisitionRunning == other.acquisitionRunning &&
				gainInVperV == other.gainInVperV &&
				offsetInmV == other.offsetInmV;
	}

	bool operator!=(const usblinecamera8mSettings& other) const {
		return !operator==(other);
	}
};

Q_DECLARE_METATYPE(usblinecamera8mSettings)

#endif // USBLINECAMERA8MSETTINGS_H
