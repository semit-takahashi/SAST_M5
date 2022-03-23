/**
 * @file sensor.cpp
 * @author Fumihito Takahashi (fumihito.takahashi@sem-it.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "sensor.h"

/** ======================================================== SensThresh */

/**
 * @brief 引数のSensThreshを自分自身にコピーする
 * 
 * @param th 元のデータ
 */
void SensThresh::copy( const SensThresh *th ) {
    warn_templ = th->warn_templ;
    caut_templ = th->caut_templ;
    warn_humid = th->warn_humid;
    caut_humid = th->caut_humid;
}

/**
 * @brief 温度閾値の設定
 * 
 * @param warn 
 * @param caution 
 */
void SensThresh::setTempl( const float warn, const float caution) {
    warn_templ = warn;
    caut_templ = caution;
}

/**
 * @brief 湿度閾値の設定
 * 
 * @param warn 
 * @param caution 
 */
void SensThresh::setHumid( const float warn, const float caution) {
    warn_humid = warn;
    caut_humid = caution;
}

/**
 * @brief DEBUG dump()
 */
void SensThresh::dump( const SensThresh *s ) {
    Serial.printf("SensThresh-> warn_templ : %8.2f\r\n",s->warn_templ );
    Serial.printf("SensThresh-> caut_templ : %8.2f\r\n",s->caut_templ );
    Serial.printf("SensThresh-> warn_humid : %8.2f\r\n",s->warn_humid );
    Serial.printf("SensThresh-> caut_humid : %8.2f\r\n",s->caut_humid );
}

/**
 * @brief 温度を確認してステータスを返答
 * 
 * @param templ  確認対象の温度
 * @return SSTAT_t 温度のステータス
 */
SSTAT_t SensThresh::getStatusTempl( const float templ ) {
    if( templ > caut_templ ) {
        return SSTAT_t::caution;
    }
    else if( templ > warn_templ ) {
        return SSTAT_t::warn;
    }
    return SSTAT_t::normal;
}

/** ======================================================== sData */

/**
 * @brief sDataを別の構造体にクローンする
 * 
 * @param src コピー元（アドレス渡し）
 * @param dst コピー先（アドレス渡し）
 */
void sData::clone( const sData *src, sData *dst ) {
    dst->Type    = src->Type;
    dst->ID      = src->ID;
    dst->Templ   = src->Templ;
    dst->Humid   = src->Humid;
    dst->Press   = src->Press;
    dst->RSSI    = src->RSSI;
    dst->AVS     = src->AVS;
    dst->batt    = src->batt;
    dst->date    = src->date;
}

void sData::dump( const sData *s ){
    Serial.printf("Data-> Type  : %d\r\n",s->Type);
    Serial.printf("Data-> ID    : %s\r\n",s->ID.c_str());
    Serial.printf("Data-> Templ : %8.2f\r\n",s->Templ);
    Serial.printf("Data-> Humid : %8.2f\r\n",s->Humid);
    Serial.printf("Data-> Press : %d\r\n",s->Press);
    Serial.printf("Data-> RSSI  : %d\r\n",s->RSSI);
    Serial.printf("Data-> AVS   : %d\r\n",s->AVS);
    Serial.printf("Data-> BATT  : %8.2f\r\n",s->batt);
    Serial.printf("Data-> date  : %d\r\n",s->date);
}

/** ======================================================== Sensor */

/**
 * @brief センサー情報の初期化
 * 
 * @param type センサータイプ
 * @param name センサーの名称（LINE表示用）
 * @param ID センサーのID（MACアドレス）
 */
void Sensor::init( const SENS_t type, const String name, const String id, const SensThresh *th,
                    const uint8_t a_templ, const uint8_t a_humid, const uint8_t a_avs ) {
    Data.Type = type;
    Type = &Data.Type;
    Name = name;
    Data.ID = id;
    ID = &Data.ID;
    updated = false;
    thr.copy( th );
    use = true;

    amb_templ = a_templ;
    amb_humid = a_humid;
    amb_avs   = a_avs;    
}


/**
 * @brief 引数のデータを内部のデータにコピーして更新
 * 
 * @param dt コピー元センサーデータ
 */
void Sensor::update( const sData *dt) {
    // 一つ前のデータにコピー
    prevData.ID      = Data.ID;
    prevData.Templ   = Data.Templ;
    prevData.Humid   = Data.Humid;
    prevData.Press   = Data.Press;
    prevData.RSSI    = Data.RSSI;
    prevData.AVS     = Data.AVS;
    prevData.batt    = Data.batt;
    prevData.date    = Data.date;

    //新しいデータに更新
    Data.ID      = dt->ID;
    Data.Templ   = dt->Templ;
    Data.Humid   = dt->Humid;
    Data.Press   = dt->Press;
    Data.RSSI    = dt->RSSI;
    Data.AVS     = dt->AVS;
    Data.batt    = dt->batt;
    Data.date    = dt->date;

    //更新済フラグ
    updated = true;

    // センターデータの状態検知
    status = thr.getStatusTempl( dt->Templ );

}

/**
 * @brief 現在のセンサデータ
 * 
 * @return sData* クラスアドレス
 */
sData* Sensor::getData() {
    return &Data;
}

/**
 * @brief 一つ目のセンサデータ
 * 
 * @return sData* クラスアドレス
 */
sData* Sensor::getPrevData(){
    return &prevData;
}

/**
 * @brief TODO 前回更新時間からの間隔確認
 * 
 * @param interval 計測間隔時間（UNIXTime）
 * @return true  指定時間内
 * @return false 更新間隔を過ぎている
 */
bool Sensor::updateTimeSpan( const  uint32_t interval ){
    // TODO 現在のdataimeからDate.dateにintervalを足した値を引く
    // true  時間内
    // false 時間経過済
    return true;
}

/**
 * @brief 新しいデータがあったら返す
 * 
 * @return sData* データがあればアドレス、無ければNULL
 */
sData* Sensor::getNewData() {
    if ( updated ) {
        updated = false;
        return &Data;
    }
    return NULL;
}

/** ======================================================== SensList */

/**
 * @brief Construct a new Sens List:: Sens List object
 * 
 */
SensList::SensList(){
    Num = 0;
}

/**
 * @brief センサーを追加
 * 
 * @param type   センサータイプ
 * @param ID     センサーID（MACアドレス
 * @param Name   センサー名  
 * @param th     センサー閾値description
 * @param a_templ Ambient通知ID 温度
 * @param a_humid Ambient通知ID 湿度
 * @param a_avs   Ambient通知ID 光量
 * 
 * @return true  追加成功
 * @return false 最大数のため追加不可能
 */
bool SensList::add( const  SENS_t type, const  String name, const  String id , SensThresh th,
                    const uint8_t a_templ, const uint8_t a_humid, const uint8_t a_avs ){
    if( type == SENS_t::None ) return false;
    if( Num < MAX_SENS ) {
        Sens[Num].init( type, name, id, &th, a_templ, a_humid, a_avs );
        Num++;
        return true;
    }
    return false;
}

/**
 * @brief センサデータを更新
 * 
 * @param dt 更新するデータ（内部のIDにて検索）
 * @return true 更新成功
 * @return false センサーが無い
 */
bool SensList::update( const  sData *dt ) {
    for( uint8_t i; i < Num; i++  ) {
        if( Sens[i].Data.Type == dt->Type ) {    // 高速化のためTypeで一次判定
            if( Sens[i].Data.ID == dt->ID ) {    // 次に文字列比較(ID)
                Sens[i].update( dt );
                LCD->update( i, Sens[i].status, &Sens[i].Data );
                // TODO Ambientに通知
                // ex: amb->update( amb_num, data, keta );
                if( Sens[i].status != SSTAT_t::normal ) {
                    // TODO Notifyに通知
                    // ex: notify->set( sen->Data, sen->status )
                }
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 指定されたIDのセンサーを返す
 * 
 * @param id センサーのID（MACアドレス）
 * @param type センサーのタイプ
 * @return Sensor* 
 */
Sensor * SensList::getSensor( const String id, const SENS_t type ) {
    for( uint8_t i; i < Num; i++  ) {
        if( *Sens[i].Type == type ) {    // 高速化のためTypeで一次判定
            if( *Sens[i].ID == id ) {    // 次に文字列比較(ID)
                return &Sens[i];
            }
        }
    }
    return NULL;
}

/**
 * @brief DEBUG dump
 * 
 */
void SensList::dump() {
    for( int i=0; MAX_SENS; i++ ) {
    Serial.print("Type          :"); Serial.printf("%d\r\n",*Sens[i].Type);
    Serial.print("ID            :"); Serial.printf("%s\r\n",*Sens[i].ID);
    Serial.print("Name          :"); Serial.println(Sens[i].Name);
    Serial.println("Data.duump()");  sData::dump( &Sens[i].Data );
    Serial.print("prevData.dump()"); sData::dump( &Sens[i].prevData );
    Serial.print("updated       :"); Serial.println(Sens[i].updated);
    Serial.print("use           :"); Serial.println(Sens[i].use);
    Serial.print("status        :"); Serial.printf("%d\r\n",Sens[i].status);
    Serial.print("SensThresh.dump()"); SensThresh::dump( &Sens[i].thr );
    }
}
