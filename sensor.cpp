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
void SensThresh::copy( SensThresh *th ) {
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
void SensThresh::setTempl( float warn, float caution) {
    warn_templ = warn;
    caut_templ = caution;
}

/**
 * @brief 湿度閾値の設定
 * 
 * @param warn 
 * @param caution 
 */
void SensThresh::setHumid( float warn, float caution) {
    warn_humid = warn;
    caut_humid = caution;
}

/**
 * @brief DEBUG dump()
 */
void SensThresh::dump() {
    Serial.printf("SensThresh-> warn_templ : %8.2f\r\n",warn_templ );
    Serial.printf("SensThresh-> caut_templ : %8.2f\r\n",caut_templ );
    Serial.printf("SensThresh-> warn_humid : %8.2f\r\n",warn_humid );
    Serial.printf("SensThresh-> caut_humid : %8.2f\r\n",caut_humid );
}

/**
 * @brief 温度を確認してステータスを返答
 * 
 * @param templ  確認対象の温度
 * @return SSTAT_t 温度のステータス
 */
SSTAT_t SensThresh::getStatusTempl( float templ ) {
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
void sData::clone( sData *src, sData *dst ) {
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

void sData::dump(){
    Serial.printf("Data-> Type  : %d\r\n",Type);
    Serial.printf("Data-> ID    : %s\r\n",ID.c_str());
    Serial.printf("Data-> Templ : %8.2f\r\n",Templ);
    Serial.printf("Data-> Humid : %8.2f\r\n",Humid);
    Serial.printf("Data-> Press : %d\r\n",Press);
    Serial.printf("Data-> RSSI  : %d\r\n",RSSI);
    Serial.printf("Data-> AVS   : %d\r\n",AVS);
    Serial.printf("Data-> BATT  : %8.2f\r\n",batt);
    Serial.printf("Data-> date  : %d\r\n",date);
}

/** ======================================================== Sensor */

/**
 * @brief センサー情報の初期化
 * 
 * @param type センサータイプ
 * @param name センサーの名称（LINE表示用）
 * @param ID センサーのID（MACアドレス）
 */
void Sensor::init( SENS_t type, String name, String id, SensThresh *th  ) {
    Data.Type = type;
    Type = &Data.Type;
    Name = name;
    Data.ID = id;
    ID = &Data.ID;
    updated = false;
    thr.copy( th );
    use = true;
}


/**
 * @brief 引数のデータを内部のデータにコピーして更新
 * 
 * @param dt コピー元センサーデータ
 */
void Sensor::update( sData *dt) {
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
bool Sensor::updateTimeSpan( uint32_t interval ){
    // 現在のdataimeからDate.dateにintervalを足した値を引く
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
 * @return true  追加成功
 * @return false 最大数のため追加不可能
 */
bool SensList::add( SENS_t type, String name, String id , SensThresh th ){
    if( Num < MAX_SENS ) {
        Sens[Num].init( type, name, id, &th);
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
bool SensList::update( sData *dt ) {
    Sensor *sen = getSensor( dt->ID, dt->Type );
    if( sen != NULL ) {
        sen->update( dt );
        return true;
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
Sensor * SensList::getSensor( String id, SENS_t type ) {
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
    Serial.println("Data.duump()"); Sens[i].Data.dump();
    Serial.print("prevData.dump()"); Sens[i].prevData.dump();
    Serial.print("updated       :"); Serial.println(Sens[i].updated);
    Serial.print("use           :"); Serial.println(Sens[i].use);
    Serial.print("status        :"); Serial.printf("%d\r\n",Sens[i].status);
    Serial.print("SensThresh.dump()"); Sens[i].thr.dump();
    }
}
