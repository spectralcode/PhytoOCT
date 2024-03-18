#ifndef USBLC8MJTN_H
#define USBLC8MJTN_H

#include <QtCore/qglobal.h>

quint16  ls_libversion();

void ls_initialize(qint32 pipesize, qint32 packetlen);
qint32 ls_enumdevices();
quint16 ls_getfwversion(qint32 index);
const char* ls_getvendorname(qint32 index);
const char* ls_getproductname(qint32 index);
const char* ls_getserialnumber(qint32 index);
quint8 ls_devicecount();
qint32 ls_currentdeviceindex();
qint32 ls_closedevice();
qint32 ls_opendevicebyindex(qint32 index);
qint32 ls_opendevicebyserial(char* serial_num);
qint32 ls_getpipe(void* lpbuffer, qint32 numberbytes);
void ls_waitforpipe(quint32 timeout);
qint32 ls_waitforpipecount(qint32 nNumberOfbytes, quint32 timeout);
qint32 ls_setpacketlength(qint32 packetlen);
quint32 ls_getfps();
qint32 ls_getsensortype(quint16 &wsensortype, quint16 &wpixelcount, quint32 timeout);
const char* ls_geterrorstring(int ierr);

const char* ls_getsensorname(quint16 wsensortype);
qint32 ls_getminmaxtime(quint16 wsensortype, quint32 &tmin, quint32 &tmax);

qint32 ls_savesettings(quint32 timeout);
qint32 ls_getadcconfig(quint16 &wconfig, quint32 timeout);
qint32 ls_getadcpga1(quint16 &wpga, quint32 timeout);
qint32 ls_getadcpga2(quint16 &wpga, quint32 timeout);
qint32 ls_getadcpga3(quint16 &wpga, quint32 timeout);
qint32 ls_getadcoffset1(quint16 &woffset, quint32 timeout);
qint32 ls_getadcoffset2(quint16 &woffset, quint32 timeout);
qint32 ls_getadcoffset3(quint16 &woffset, quint32 timeout);

qint32 ls_setadcconfig(quint16 wconfig, quint32 timeout);
qint32 ls_setadcpga1(quint16 wpga, quint32 timeout);
qint32 ls_setadcpga2(quint16 wpga, quint32 timeout);
qint32 ls_setadcpga3(quint16 wpga, quint32 timeout);
qint32 ls_setadcoffset1(quint16 woffset, quint32 timeout);
qint32 ls_setadcoffset2(quint16 woffset, quint32 timeout);
qint32 ls_setadcoffset3(quint16 woffset, quint32 timeout);

qint32 ls_setmode(quint8 ucmode, quint32 timeout);
qint32 ls_setstate(quint8 ucstate, quint32 timeout);
qint32 ls_setinttime(quint32 dwinttime, quint32 timeout);
qint32 ls_setextdelay(quint32 dwextdelay, quint32 timeout);
qint32 ls_setcfg1(quint8 uccfg1, quint32 timeout);
qint32 ls_setsofttrigtime(quint32 dwsofttrigtime, quint32 timeout);

qint32 ls_getmcusensortype(quint16 &wversion, quint32 timeout);
qint32 ls_getmode(quint8 &ucmode, quint32 timeout);
qint32 ls_getstate(quint8 &ucstate, quint32 timeout);
qint32 ls_getinttime(quint32 &dwinttime, quint32 timeout);
qint32 ls_getextdelay(quint32 &dwextdelay, quint32 timeout);
qint32 ls_getcfg1(quint8 &uccfg1, quint32 timeout);
qint32 ls_getsofttrigtime(quint32 &dwsofttrigtime, quint32 timeout);
qint32 ls_getminmaxsfttrigtime(quint32 &dwmintime, quint32 &dwmaxtime, quint32 timeout);
qint32 ls_getpacketlength(qint32 &ipacketlength, quint32 timeout);
qint32 ls_customfirmware(quint32 &dwcustom, quint32 timeout);


#endif // USBLC8MJTN_H
