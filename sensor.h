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
#ifndef __SAST_SENSOR_H__
#define __SAST_SENSOR_H__


#include <M5Stack.h>
#include "SAST_M5.h"
#include "display.h"

// Protype
class M5_LCD;


/**
 * @brief 閾値情報クラス
 * 
 */
class SensThresh {
    public:
        float warn_templ;
        float caut_templ;
        float warn_humid;
        float caut_humid;
    public:
        void copy( const SensThresh *th );
        void setTempl( const float warn, const float catuion);
        void setHumid( const float warn, const float caution);
        SSTAT_t getStatusTempl( const float templ );

        static void dump( const SensThresh *s);
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
        void clone( const sData *src);

        static void dump( const sData *dt );

};


/**
 * @brief センサークラス
 * 個々のセンサーの情報　INFにて読み込まれた数だけ生成
 * データ自体はsDataクラスにて定義
 */
class Sensor {
    public:
        SENS_t  Type;
        String  Name;
        String  ID;

    //private:
        sData   Data;
        sData   prevData;
        bool    updated;
        bool    use = false;
        SSTAT_t status = SSTAT_t::disable;
        SensThresh thr;
        double  diffSec;
        time_t  notify_time = 0;

        // Ambient通知番号設定
        uint8_t amb_templ;
        uint8_t amb_humid;
        uint8_t amb_avs;

    public:
        void init( const SENS_t type, const String name, const String ID, const SensThresh *th , const uint8_t a_templ, const uint8_t a_humid, const uint8_t a_avs );
        void update( const sData *dt );
        sData *getData();
        sData *getPrevData();
        void done();
        bool updateTimeSpan( const uint32_t interval );
        sData *getNewData();

        static void dump( Sensor *s );
};


class SensList {
    public:
        Sensor  Sens[MAX_SENS] ;    // センサー情報配列
        Sensor  EnvS;               // 本体センサー
        bool    _notify[MAX_SENS];  // 通知が必要なセンサー
        uint8_t Num = 0;            // 登録センサー数
        M5_LCD  *LCD;               // LCD表示画面クラス
      //Ambient *amb;               // グラフ化クラス
      //Notify  *notify;            // 通知クラス 
    
    public:
        SensList();
        bool add( const SENS_t type,  const String id, const String name, const SensThresh th, const uint8_t a_templ, const uint8_t a_humid, const uint8_t a_avs );
        bool update( const sData *dt );
        void updateEnv( const sData *dt );
        Sensor *getSensor( const String id, const SENS_t type );
        void getAmbientData( st_AMB dt[] );

        void dump();
};

#endif __SAST_SENSOR_H__
