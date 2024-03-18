//This file is from the Coptonix "Qt / C++ (NVIDIA Jetson Nano)" programming exaple
//see: https://coptonix.com/products/usb-line-camera-8m/ 

#include <QtCore>
extern "C"
{
    #include "usblc8mjtn.h"
}
#include "pipethread.h"

PipeThread::PipeThread(QObject *parent):
    QThread(parent)
{
    this->stop = false;
}

void PipeThread::run()
{
    qint32 BytesRead;
    qint32 BytesAvailable;
    qint32 fps;
    quint16* CamBuf;


    if (this->BufSize == 0) this->BufSize = 4096; //2 x 2048 pixel
    CamBuf = new quint16[this->BufSize];

    while(1)
    {
        if (this->shouldstop()) break;

        BytesAvailable = ls_waitforpipecount(this->BufSize, 100);
        if (BytesAvailable >= this->BufSize) {
            BytesRead = ls_getpipe(CamBuf,this->BufSize);
            if (BytesRead > 0) {
                fps = ls_getfps() / this->BufSize;
                emit DataChanged(CamBuf,BytesRead,BytesAvailable-BytesRead,fps);
            }
        }
    }
    delete[] CamBuf;
}

bool PipeThread::shouldstop()
{
    QMutexLocker locker(&this->stop_mx);
    return(this->stop);
}

void PipeThread::stopthread()
{
    QMutexLocker locker(&this->stop_mx);
    this->stop = true;
}
