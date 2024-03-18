
#ifndef USBLINECAMERA8MPLUGIN_H
#define USBLINECAMERA8MPLUGIN_H

#define STREAM_BUFFER_SIZE 2097152

#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QDebug>
#include "math.h"
#include "usblinecamera8msystemsettingsdialog.h"
#include "octproz_devkit.h"
#include "usblinecamera8m.h"
#include "usblinecamera8msettings.h"
#include "serialcompanel/octserialcom.h"


class USBLineCamera8MSystem : public AcquisitionSystem
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID AcquisitionSystem_iid)
	Q_INTERFACES(AcquisitionSystem)
	QThread cameraThread;

public:
	explicit USBLineCamera8MSystem();
	~USBLineCamera8MSystem();

	virtual void startAcquisition() override;
	virtual void stopAcquisition() override;
	virtual void settingsLoaded(QVariantMap settings) override;

private:
	USBLineCamera8MSystemSettingsDialog* systemDialog;
	AcquisitionParams currOctParams;
	usblinecamera8mSettings currCameraParams;
	USBLineCamera8M* camera;
	OCTSerialCom* octSerialCom;


	bool init();
	void uninit();

public slots:
	void onCameraSettingsChanged(usblinecamera8mSettings newParams);
	void onOctDimensionsChanged(AcquisitionParams newOctParams);

signals:
	void enableGui(bool enable);
};

#endif // USBLINECAMERA8MPLUGIN_H
