#include "comportselectwidget.h"

ComPortSelectWidget::ComPortSelectWidget(QWidget *parent) : QWidget(parent)
{
	this->ui.setupUi(this);
	this->scanComPorts();
	this->fillBaudRatesComboBox();
	connect(this->ui.pushButton_connect, &QPushButton::clicked, this, &ComPortSelectWidget::onConnectButtonPressed);
	connect(this->ui.pushButton_disconnect, &QPushButton::clicked, this, &ComPortSelectWidget::onDisconnectButtonPressed);
	connect(this->ui.comboBox_port, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComPortSelectWidget::updateComPortInfo);
	connect(this->ui.toolButton_rescan, &QToolButton::clicked, this, &ComPortSelectWidget::scanComPorts);
}

void ComPortSelectWidget::scanComPorts() {
	//disconnect signal slot during scan
	disconnect(this->ui.comboBox_port, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComPortSelectWidget::updateComPortInfo);

	//remove previously stored com port data
	this->ui.comboBox_port->clear();

	//scan com ports and store data in comboBox as "userData" (see Qt QComboBox documentation)
	QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
	for (auto info : infos) {
		QStringList comPortInfoList;
		comPortInfoList << info.portName()
						<< info.description()
						<< info.manufacturer()
						<< info.serialNumber()
						<< info.systemLocation()
						<< QString::number(info.vendorIdentifier())
						<< QString::number(info.productIdentifier());

		this->ui.comboBox_port->addItem(comPortInfoList.first(), comPortInfoList);
	}

	//reconnect signal slot during scan
	connect(this->ui.comboBox_port, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComPortSelectWidget::updateComPortInfo);

	//update com port infos in gui
	if(this->ui.comboBox_port->count() > 0){
		this->updateComPortInfo(this->ui.comboBox_port->currentIndex());
	}
}

void ComPortSelectWidget::updateComPortInfo(int id) {
	//each comboBox element contains a QStringList as userData (see scanComPorts())
	QStringList comPorts = ui.comboBox_port->itemData(id).toStringList();
	ui.label_description->setText(comPorts.at(1));
	ui.label_manufacturer->setText(comPorts.at(2));
	ui.label_serialNumber->setText(comPorts.at(3));
	ui.label_location->setText(comPorts.at(4));
	ui.label_vendor->setText(comPorts.at(5));
	ui.label_productID->setText(comPorts.at(6));
}

void ComPortSelectWidget::fillBaudRatesComboBox() 
{
	int defaultIndex = -1;
	foreach(auto baudRate, QSerialPortInfo::standardBaudRates()) {
		this->ui.comboBox_baudrate->addItem(QString::number(baudRate));
		if (baudRate == 9600) {
			defaultIndex = this->ui.comboBox_baudrate->count() - 1;
		}
	}
	
	if (defaultIndex != -1) {
		this->ui.comboBox_baudrate->setCurrentIndex(defaultIndex);
	}
}

void ComPortSelectWidget::onConnectButtonPressed() {
	if(this->ui.comboBox_port->count() > 0){
		emit connectToComPortRequested(this->ui.comboBox_port->currentText(), this->ui.comboBox_baudrate->currentText().toInt());
	}
}

void ComPortSelectWidget::onDisconnectButtonPressed() {
	emit disconnectFromComPortRequested();
}
