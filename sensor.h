/**
 * @file sensor.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <M5Stack.h>
#include "SAST_M5.h"

/**
 * @brief 閾値情報クラス
 * 
 */
class SensThresh {
    private:
        float warn_templ;
        float caut_templ;
        float warn_humid;
        float caut_humid;
    public:
        void copy( SensThresh *th );
        void setTempl( float warn, float catuion);
        void setHumid( float warn, float caution);
        SSTAT_t getStatusTempl( float templ );

        void dump();
};


/**
 * @brief センサー情報クラス　SData
 */
class sData {
    public:
        SENS_t      Type;
        String      ID = "";
        float       Templ = 0.0;
        float       Humid = 0.0;
        uint16_t    Press = 0;
        int16_t     RSSI = 0;
        uint32_t    AVS = 0;
        float       batt = 0.0;
        time_t      date;
    
    public:
        void clone( sData *src,  sData *dst);

        void dump();

};


/**
 * @brief センサークラス
 * 個々のセンサーの情報　INFにて読み込まれた数だけ生成
 * データ自体はsDataクラスにて定義
 */
class Sensor {
    public:
        SENS_t  *Type;
        String  Name;
        String  *ID;

    //private::
        sData   Data;
        sData   prevData;
        bool    updated;
        bool    use = false;
        SSTAT_t status;
        SensThresh thr;

    public:
        void init( SENS_t type, String name, String ID, SensThresh *th );
        void update( sData *dt );
        sData *getData();
        sData *getPrevData();
        void done();
        bool updateTimeSpan( uint32_t interval );
        sData *getNewData();

};


class SensList {
    public:
        Sensor  Sens[MAX_SENS] ;  // センサー情報配列
        uint8_t Num = 0;        // 登録センサー数
    
    public:
        SensList();
        bool add( SENS_t type,  String id, String name, SensThresh th ) ;
        bool update( sData *dt );
        Sensor *getSensor( String id, SENS_t type );


        void dump();

};
