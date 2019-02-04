#ifndef QHPGDEVICELIB_H
#define QHPGDEVICELIB_H

#include <QObject>
#include <QDebug>
#include <QWidget>
#include <windows.h>
#include "tchar.h"      // to use _T("") for LoadLibrary
#define WM_Raduga4_MCA (WM_USER + 1015)

struct RadugaDevInfo{
    bool iRadugaDevice; // устройство типа RadugaMCA
    int8_t iRadugaPort; // номер COM порта
    int iRadugaBaud; // скорость обмена с устройством
    bool iRadugaDevInUse; // устройство уже используется (занято)
    bool iRadugaDevError; // обнаружены ошибки при связи с устройством
    char iRadugaDescript[128]; // дескриптор (название) устройства
    char iRadugaSerial[32]; // серийный номер устройства
    char iRadugaUniqID[12]; // уникальный 12 байтный код устройства
    void *iRadugaMCA; // указатель на программный объект анализатора.
    char buff[12];  //страховка
};
struct SpectrumType{
    SpectrumType();
    SpectrumType(const SpectrumType &other);
    uint32_t channelCount; // количество каналов спектра
    bool dataIsNew; // произошло ли обновление спектра с момента предыдущего чтения
    int64_t dataCount; // количество зарегистрированных событий
    DWORD workTime; // счетчик времени набора спектра (в миллисекундах)
    DWORD liveTime; // счетчик «живого» времени набора спектра (в миллисекундах)
    DWORD dataTime; // время начала набора
    DWORD chanArray[16384]; // буфер спектра (по 4 байта на канал)

    //~SpectrumType();
};
struct DeviceStatusInfo{
    uint8_t sSPK_ON; // 1 байт, младший бит – признак активного набора спектра      1
    uint8_t sSPK_SET; // 1 байт, младшие 3 бита – размер набираемого спектра (*)    2
    uint8_t sFLTR_SETA; // 1 байт, служебная информация                             3
    uint8_t sFLTR_SETB; // 1 байт, служебная информация                             4
    uint32_t sSPK_COUNT; // 4 байта, общее количество событий (интеграл) в спектре  8
    uint32_t sSPK_REALTIME; // 4 байта, общее время набора спектра в миллисекундах  12
    uint32_t sSPK_LIVETIME; // 4 байта, «живое» время набора спектра в миллисекундах    16
    float sSPK_DEADTIME; // 4 байта, «мертвое» время набора в процентах                 20
    uint32_t sSPK_SLOWRATE; // 4 байта, «загрузка» в медленном канале анализатора (**)  24
    uint32_t sSPK_FASTRATE; // 4 байта, «загрузка» в быстром канале анализатора (**)    28
    uint8_t sADC_Freq; // 1 байт, частота выборок АЦП анализатора (МГц)                 29
    uint8_t sSPK_type; // 1 байт, не используется в текущей версии                      30
    uint16_t sSLOW_TH; // 2 байта, порог дискриминатора в медленном канале анализатора  32
    uint16_t sFATS_TH; // 2 байта, порог дискриминатора в быстром канале анализатора    34
};

struct DeviceStatusExtInfo{
    uint16_t sEXT_IN; // 2 байта, не используется в текущей версии
    uint16_t sEXT_OUT; // 2 байта, не используется в текущей версии
    int16_t sINT_TEMP; // 2 байта, температура процессора анализатора (***)
    int16_t sEXT_TEMP1; // 2 байта, температура первого внешнего сенсора анализатора (***)
    int16_t sEXT_TEMP2; // 2 байта, температура второго внешнего сенсора анализатора (***)
    //uint16_t sEXT_FLAGS; // 2 байта, не используется в текущей версии
    uint16_t sSTAB_FILTER; // 2 байта, значение на выходе фильтра стабилизации по пику (****)
    float sDAC1_SETVAL; // 4 байта, заданное значение высокого напряжения в вольтах
    float sDAC1_CURVAL; // 4 байта, текущее значение высокого напряжения в вольтах
    float sDAC2_SETVAL; // 4 байта, заданное значение тока охладителя в миллиамперах
    float sDAC2_CURVAL; // 4 байта, текущее значение тока охладителя в миллиамперах
    float sADC1_VALUE; // 4 байта, не используется в текущей версии
    float sADC2_VALUE; // 4 байта, не используется в текущей версии
    //float sECALGET_C0; // 4 байта, коэффициент C0 калибровки по энергии (см «Калибровка»)
    //float sECALGET_C1; // 4 байта, коэффициент C1 калибровки по энергии (см «Калибровка»)
    //float sECALGET_C2; // 4 байта, коэффициент C2 калибровки по энергии (см «Калибровка»)
    uint8_t sRESERV[8]; // зарезервировано для возможного расширения
};

typedef int (__stdcall *t_g4show_ctrl_window)(int dev_handle);
typedef int (__stdcall *t_g4com_search_devices)();
typedef int (__stdcall *t_g4com_get_device_description)(int deviceNumber,RadugaDevInfo *deviceDescriptor);
typedef int (__stdcall *t_g4com_connect_by_index)(int deviceNumber,int &deviceHandle);
typedef int (__stdcall *t_g4com_disconnect)(int deviceHandle);
typedef int (__stdcall *t_g4com_set_owner_whandle)(int deviceHandle,WId handle,WId handle2);
typedef int (__stdcall *t_g4com_get_status)(int deviceHandle,DeviceStatusInfo *deviceStatus);
typedef int (__stdcall *t_g4com_get_spk_buf)(int deviceHandle,SpectrumType *spectrum);
typedef int (__stdcall *t_g4com_start_spk_ext)(int deviceHandle,int workMode,bool clearBuf,bool continueScan,DWORD workTime,WORD winLeft,WORD winRight);
typedef int (__stdcall *t_g4com_stop_spk)(int deviceHandle);
typedef int (__stdcall *t_g4com_get_mca_idle_mode)(int deviceHandle,int *modeInfo);
typedef int (__stdcall *t_g4com_set_mca_idle_mode)(int deviceHandle,int modeInfo);
typedef int (__stdcall *t_g4com_set_hv)(int deviceHandle,WORD hv,bool useCalibr);
typedef int (__stdcall *t_g4com_get_mca_calibr)(int deviceHandle,char*wtf);
typedef int (__stdcall *t_g4com_get_mca_param)(int deviceHandle,int mca_param_num,void*wtf);
typedef int (__stdcall *t_g4com_get_ext_status)(int deviceHandle,DeviceStatusExtInfo *info);


//typedef int (__stdcall *t_g4com_set_owner_thandle)(int deviceHandle,Qt::HANDLE threadId,Qt::HANDLE _);

class QHPGDeviceLib : public QObject
{
    Q_OBJECT
public:
    static const int cmd_nop               = 0;
    static const int cmd_reset             = 1;
    static const int cmd_fill_buf          = 2;
    static const int cmd_start             = 3;
    static const int cmd_stop              = 4;
    static const int cmd_get_stat          = 5;
    static const int cmd_get_spk   		   = 6;
    static const int cmd_fix_spk  	       = 7;
    static const int cmd_filter_set		   = 8;
    static const int cmd_filter_get		   = 9;
    static const int cmd_filter_config	   = 10;
    static const int cmd_filter_default    = 11;
    static const int cmd_calibr_get        = 12; //чтение калибровки (с проверкой пароля)
    static const int cmd_calibr_set        = 13; //запись калибровки (с проверкой пароля)
    static const int cmd_get_osc		   = 14;
    static const int cmd_WriteToCyclone    = 16; //  0x2E  стирание FLASH (с проверкой пароля)
    static const int cmd_ReadFromCyclone   = 17; //  0x2E  стирание FLASH (с проверкой пароля)
    static const int cmd_WriteReadCyclone  = 18; //  тестовая запись и тут же чтение
    static const int cmd_set_list_mode	   = 19;	// включение/выключения режима Амплитуда/Время
    static const int cmd_set_dac1		   = 20;
    static const int cmd_set_dac2		   = 21;
    static const int cmd_clear_eeprom      = 22;
    static const int cmd_set_uart12        = 23;
    static const int cmd_get_ext_stat      = 24;
    static const int cmd_get_spk_page      = 64; // команды с 64-й по 127 - запрос на передачу страницы спектра

    QHPGDeviceLib(QObject *parent = 0);
    ~QHPGDeviceLib();
    bool libIsValid;
    bool isScanning;
    RadugaDevInfo deviceInfo;
    DeviceStatusInfo info;
    DeviceStatusExtInfo extraInfo;
    SpectrumType spectrum;
    void setMainWindowId(WId idHandle);
    void parseMsg(void *message);
    int getDeviceHandle();
    int showCorntolWindow(int handle);
    int getAvailableDevicesCount();
    int getDeviceInfo(int deviceNumber,RadugaDevInfo *deviceDescriptor);

    int connectToDevice(int deviceNumber);
    int disconnectFromDevice();
    int startScanSpectrum();
    int stopScanSpectrum();

    void setVoltage(int voltage);
    bool isConnected();
private:
    QHPGDeviceLib(const QHPGDeviceLib & other);
    HINSTANCE hGetProcIDDLL;
    int deviceHandle = -1;
    bool loadDeviceLib();
    WId idHandle;
    t_g4show_ctrl_window            g4show_ctrl_window;
    t_g4com_search_devices          g4com_search_devices;
    t_g4com_get_device_description  g4com_get_device_description;
    t_g4com_connect_by_index        g4com_connect_by_index;
    t_g4com_disconnect              g4com_disconnect;
    t_g4com_set_owner_whandle       g4com_set_owner_whandle;
    t_g4com_get_status              g4com_get_status;
    t_g4com_get_spk_buf             g4com_get_spk_buf;
    t_g4com_start_spk_ext           g4com_start_spk_ext;
    t_g4com_stop_spk                g4com_stop_spk;
    t_g4com_get_mca_idle_mode       g4com_get_mca_idle_mode;
    t_g4com_set_mca_idle_mode       g4com_set_mca_idle_mode;
    t_g4com_set_hv                  g4com_set_hv;
    t_g4com_get_mca_calibr          g4com_get_mca_calibr;
    t_g4com_get_mca_param           g4com_get_mca_param;    
    t_g4com_get_ext_status          g4com_get_ext_status;
signals:
    void connectionLost();
    void connected();
    void newSpectrum();
    void newStatus();
public slots:

};


#endif // QHPGDEVICELIB_H
