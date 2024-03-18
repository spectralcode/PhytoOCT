/*
MIT License

Copyright (c) 2019-2022 Miroslav Zabic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#define SYSNAME "sys_name"
#define BITDEPTH "bit_depth"
#define WIDTH "width"
#define HEIGHT "height"
#define DEPTH "depth"
#define BUFFERS_PER_VOLUME "buffers_per_volume"


#include <qstandardpaths.h>
#include <qvariant.h>
#include <QDialog>
#include <QString>
#include <QFileDialog>
#include "usblinecamera8msettings.h"
#include "acquisitionparameter.h"
#include "ui_usblinecamera8msystemsettingsdialog.h"


class USBLineCamera8MSystemSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	USBLineCamera8MSystemSettingsDialog(QWidget *parent = nullptr);
	~USBLineCamera8MSystemSettingsDialog();

	void setSettings(QVariantMap settings);
	void getSettings(QVariantMap* settings);
	qint32 getDeviceIndex();


private:
	Ui::USBLineCamera8MSystemSettingsDialog* ui;
	usblinecamera8mSettings cameraParams;
	AcquisitionParams acqParams;

	void initGui();
	void connectAllGuiElementsToGetCameraSettingsFromGuiSlot();
	void disconnectAllGuiElementsToGetCameraSettingsFromGuiSlot();

public slots:
	void updateCameraSettingsInGui(const usblinecamera8mSettings cameraSettings);
	void updateDeviceComboBox(QStringList devices);
	void getAcquisitionSettingsFromGui();
	void getCameraSettingsFromGui();
	void enableGui(bool enable);
	void showDeviceInformation(QString infoString);
	void updateOffsetString(QString offset);
	void updateGainString(QString gain);

signals:
	void cameraSettingsChanged(usblinecamera8mSettings newCameraSettings);
	void acquisitionSettingsChanged(AcquisitionParams newAcquisitionParams);
	void enumerateClicked();
	void openClicked();
	void saveSettingsClicked();
};
