#include "octserialcom.h"
#include <QElapsedTimer>

OCTSerialCom::OCTSerialCom(QObject *parent)
	: QObject(parent), serialPort(new QSerialPort(this))
{
	connect(serialPort, &QSerialPort::readyRead, this, &OCTSerialCom::readData);
	connect(serialPort, &QSerialPort::errorOccurred, this, &OCTSerialCom::handleError);
}

OCTSerialCom::~OCTSerialCom()
{
	this->disconnectFromComPort();
}

void OCTSerialCom::connectToComPort(const QString &comPortName, qint32 baudRate)
{
	serialPort->setPortName(comPortName);
	serialPort->setBaudRate(baudRate);
	if (serialPort->open(QIODevice::ReadWrite)) {
		emit connectionEstablished(true);
	} else {
		emit errorOccurred(serialPort->errorString());
	}
}

void OCTSerialCom::disconnectFromComPort()
{
	if(serialPort->isOpen()){
		serialPort->close();
		emit connectionEstablished(false);
	}
}
	
void OCTSerialCom::sendCommand(const QString &command)
{
	if(serialPort->isOpen()) {
		QByteArray carriageReturn = "\n";
		QByteArray commandArray = command.toLocal8Bit() + carriageReturn;
		serialPort->write(commandArray);
	}
}

void OCTSerialCom::readData()
{
	QByteArray data = serialPort->readAll();

	//read serial port until end of serial data was received ("\n") or until timout occured
	QElapsedTimer timer;
	timer.start();
	while(!timer.hasExpired(1000) && (!data.contains("\n"))){
		this->serialPort->waitForReadyRead(20);
		data += this->serialPort->readAll();
	}

	//convert serial data to string and remove all carriage returns and new lines
	QString dataString(data);
	dataString.remove(QRegExp("[\\r\\n]"));

	//emit received command
	emit responseReceived(dataString);
	
	//handle special commands
    if(dataString.contains("startrecording", Qt::CaseInsensitive)){
		emit startRecordingRequested();
	}
}

void OCTSerialCom::handleError(QSerialPort::SerialPortError error)
{
	if(error == QSerialPort::NoError){
		return;
	}
	else if(error == QSerialPort::PermissionError || error == QSerialPort::OpenError){
		emit responseReceived("ERROR: " + this->serialPort->errorString());
		return;
	}
	else if(error == QSerialPort::ResourceError){
		this->disconnectFromComPort();
		emit errorOccurred(serialPort->errorString());
	}
	else{
		//this->disconnectFromComPort();
		emit responseReceived("ERROR: " + this->serialPort->errorString());
	}
}
