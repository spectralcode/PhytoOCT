#ifndef COMPORTSELECTWIDGET_H
#define COMPORTSELECTWIDGET_H

#include <QWidget>
#include <QSerialPortInfo>
#include "ui_comportselectwidget.h"

class ComPortSelectWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ComPortSelectWidget(QWidget *parent = nullptr);

private:
	Ui::ComPortSelectWidget ui;

private slots:
	void scanComPorts();
	void updateComPortInfo(int id);
	void fillBaudRatesComboBox();
	void onConnectButtonPressed();
	void onDisconnectButtonPressed();

signals:
	void connectToComPortRequested(QString comPortName, qint32 baudRate);
	void disconnectFromComPortRequested();
};

#endif // COMPORTSELECTWIDGET_H








