//This file is from the Coptonix "Qt / C++ (NVIDIA Jetson Nano)" programming exaple
//see: https://coptonix.com/products/usb-line-camera-8m/ 

#ifndef PIPETHREAD_H
#define PIPETHREAD_H
#include <QtCore>

class PipeThread : public QThread
{
    Q_OBJECT
public:
    explicit PipeThread(QObject *parent = 0);
    void run();
    bool shouldstop();
    void stopthread();
    qint32 BufSize;
    qint8 is16bit;

private:
    QMutex stop_mx;
    bool stop;

signals:
    void DataChanged(quint16*,qint32,qint32,quint32);

public slots:
};

#endif // PIPETHREAD_H
