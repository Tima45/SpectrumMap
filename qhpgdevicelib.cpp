#include "qhpgdevicelib.h"

QHPGDeviceLib::QHPGDeviceLib(QObject *parent) : QObject(parent)
{
    libIsValid = loadDeviceLib();
}

QHPGDeviceLib::~QHPGDeviceLib()
{
    FreeLibrary(hGetProcIDDLL);
}

void QHPGDeviceLib::setMainWindowId(WId idHandle)
{
    this->idHandle = idHandle;
}

void QHPGDeviceLib::parseMsg(void *message)
{
    MSG *msg = static_cast<MSG*>(message);
    if(msg->message == WM_Raduga4_MCA){
        if((int)msg->wParam == deviceHandle){
            if(msg->lParam == -1){
                disconnectFromDevice();
                return;
            }
            switch(msg->lParam){
                case cmd_get_ext_stat:{
                    //qDebug() << "get statusExtra"
                    g4com_get_ext_status(deviceHandle,&extraInfo);
                    break;
                }
                case cmd_get_stat:{
                    //qDebug() << "get status"
                    g4com_get_status(deviceHandle,&info);
                    emit newStatus();
                    break;
                }
                case cmd_get_spk:{
                    //qDebug() << "Getting spectrum" <<
                    g4com_get_spk_buf(deviceHandle,&spectrum);
                    emit newStatus();
                    if(spectrum.dataIsNew){
                        spectrum.chanArray[0] = 0;
                        spectrum.chanArray[1] = 0;
                        emit newSpectrum();
                    }

                    break;
                }
                default:{
                    qDebug() << "Unknown cmd type" << msg->lParam;
                    break;
                }
            }
        }
    }
}

int QHPGDeviceLib::getDeviceHandle()
{
    return deviceHandle;
}


bool QHPGDeviceLib::loadDeviceLib()
{
    hGetProcIDDLL = LoadLibrary(_T("RCOM4_CTRL.dll"));
    if(!hGetProcIDDLL){
        qDebug() << Q_FUNC_INFO << "Could not load the dynamic library RCOM4_CTRL.dll";
        return false;
    }

    g4show_ctrl_window = (t_g4show_ctrl_window)GetProcAddress(hGetProcIDDLL, "g4show_ctrl_window");
    g4com_search_devices = (t_g4com_search_devices)GetProcAddress(hGetProcIDDLL,"g4com_search_devices");
    g4com_get_device_description = (t_g4com_get_device_description)GetProcAddress(hGetProcIDDLL,"g4com_get_device_description");
    g4com_connect_by_index = (t_g4com_connect_by_index)GetProcAddress(hGetProcIDDLL,"g4com_connect_by_index");
    g4com_disconnect = (t_g4com_disconnect)GetProcAddress(hGetProcIDDLL,"g4com_disconnect");
    g4com_set_owner_whandle = (t_g4com_set_owner_whandle)GetProcAddress(hGetProcIDDLL,"g4com_set_owner_whandle");
    g4com_get_status = (t_g4com_get_status)GetProcAddress(hGetProcIDDLL,"g4com_get_status");
    g4com_get_spk_buf = (t_g4com_get_spk_buf)GetProcAddress(hGetProcIDDLL,"g4com_get_spk_buf");
    g4com_start_spk_ext = (t_g4com_start_spk_ext)GetProcAddress(hGetProcIDDLL,"g4com_start_spk_ext");
    g4com_stop_spk = (t_g4com_stop_spk)GetProcAddress(hGetProcIDDLL,"g4com_stop_spk");
    g4com_get_mca_idle_mode = (t_g4com_get_mca_idle_mode)GetProcAddress(hGetProcIDDLL,"g4com_get_mca_idle_mode");
    g4com_set_mca_idle_mode = (t_g4com_set_mca_idle_mode)GetProcAddress(hGetProcIDDLL,"g4com_set_mca_idle_mode");
    g4com_set_hv = (t_g4com_set_hv)GetProcAddress(hGetProcIDDLL,"g4com_set_hv");
    g4com_get_mca_calibr = (t_g4com_get_mca_calibr)GetProcAddress(hGetProcIDDLL,"g4com_get_mca_calibr");
    g4com_get_mca_param = (t_g4com_get_mca_param)GetProcAddress(hGetProcIDDLL,"g4com_get_mca_param");
    g4com_get_ext_status = (t_g4com_get_ext_status)GetProcAddress(hGetProcIDDLL,"g4com_get_ext_status");
    return true;
}

int QHPGDeviceLib::showCorntolWindow(int handle)
{
    if(libIsValid){
        return g4show_ctrl_window(handle);
    }else{
        qDebug() << Q_FUNC_INFO << "lib is not valid";
        return -1;
    }
}

int QHPGDeviceLib::getAvailableDevicesCount()
{
    if(libIsValid){
        return g4com_search_devices();
    }else{
        qDebug() << Q_FUNC_INFO << "lib is not valid";
        return -1;
    }
}

int QHPGDeviceLib::getDeviceInfo(int deviceNumber,RadugaDevInfo *deviceDescriptor)
{
    if(libIsValid){
        return g4com_get_device_description(deviceNumber,deviceDescriptor);
    }else{
        qDebug() << Q_FUNC_INFO << "lib is not valid";
        return -1;
    }
}

int QHPGDeviceLib::connectToDevice(int deviceNumber)
{
    if(deviceHandle == -1){
        if(g4com_connect_by_index(deviceNumber,deviceHandle) == 0){
            getDeviceInfo(deviceNumber,&deviceInfo);
            if(g4com_set_owner_whandle(deviceHandle,idHandle,0) == 0){
                emit connected();
                return 0;
            }else{
                qDebug() << "wait oh shi..";
                return -1;
            }
            return 0;
        }else{
            qDebug() << "could not connect to device";
            return -1;
        }
    }else{
        return 0;
    }
}

int QHPGDeviceLib::disconnectFromDevice()
{
    if(deviceHandle != -1){
        int res = g4com_disconnect(deviceHandle);
        deviceHandle = -1;
        isScanning = false;
        emit connectionLost();
        return res;
    }else{
        return 0;
    }
}

int QHPGDeviceLib::startScanSpectrum()
{
    if(isConnected()){
        isScanning = true;
        return g4com_start_spk_ext(deviceHandle,0,true,false,1000,0,1023) | g4com_set_mca_idle_mode(deviceHandle,3);
    }
    else{
        return -1;
    }
}

int QHPGDeviceLib::stopScanSpectrum()
{
    if(isConnected()){
        isScanning = false;
        return g4com_set_mca_idle_mode(deviceHandle,2) | g4com_stop_spk(deviceHandle);
    }else{
        return -1;
    }
}

void QHPGDeviceLib::setVoltage(int voltage)
{
    if(deviceHandle != -1){
        g4com_set_hv(deviceHandle,voltage,true);
    }
}

bool QHPGDeviceLib::isConnected()
{
    return deviceHandle != -1;
}



