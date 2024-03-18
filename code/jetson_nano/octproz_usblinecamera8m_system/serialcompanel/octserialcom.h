#ifndef OCTSERIALCOM_H
#define OCTSERIALCOM_H

#include <QObject>
#include <QSerialPort>

class OCTSerialCom : public QObject
{
	Q_OBJECT

public:
	explicit OCTSerialCom(QObject *parent = nullptr);
	~OCTSerialCom();

public slots:
	void connectToComPort(const QString &comPortName, qint32 baudRate);
	void disconnectFromComPort();
	void sendCommand(const QString &command);

private:
	QSerialPort *serialPort;

private slots:
	void readData();
	void handleError(QSerialPort::SerialPortError error);

signals:
	void responseReceived(const QString &response);
	void connectionEstablished(bool connected);
	void errorOccurred(const QString &errorString);
	void startRecordingRequested();
};

#endif // OCTSERIALCOM_H
