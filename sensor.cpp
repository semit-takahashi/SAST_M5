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
 * @param th 元のデータ
 */
void SensThresh::copy( const SensThresh *th ) {
    //Serial.println("SensThresh::copy()");
    warn_templ = th->warn_templ;
    caut_templ = th->caut_templ;
    warn_humid = th->warn_humid;
    caut_humid = th->caut_humid;
    //SensThresh::dump( this );
}

/**
 * @brief 温度閾値の設定
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
 * @brief 温度を確認してステータスを返答
 * @param templ  確認対象の温度
 * @return SSTAT_t 温度のステータス
 */
SSTAT_t SensThresh::getStatusTempl( const float templ ) {
    //Serial.printf("SensThresh::getStatusTempl(%f)\n",templ);
    //SensThresh::dump( this );
    if( templ > caut_templ ) {
        Serial.println("[!CAUTION!]");
        return SSTAT_t::caution;
    }
    else if( templ > warn_templ ) {
        Serial.println("[!WARN]");
        return SSTAT_t::warn;
    }
    return SSTAT_t::normal;
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



/** ======================================================== sData */

/**
 * @brief sDataを自分自身にクローンする
 * @param src コピー元（アドレス渡し）
 */
void sData::clone( const sData *src ) {
    Type    = src->Type;
    ID      = src->ID;
    Templ   = src->Templ;
    Humid   = src->Humid;
    Press   = src->Press;
    RSSI    = src->RSSI;
    AVS     = src->AVS;
    batt    = src->batt;
    date    = src->date;
}

/**
 * @bref DEBUG dump for SData
 */
void sData::dump( const sData *s ){
    Serial.printf(" Type  : %d\r\n",s->Type);
    Serial.printf(" ID    : %s\r\n",s->ID.c_str());
    Serial.printf(" Templ : %8.2f\r\n",s->Templ);
    Serial.printf(" Humid : %8.2f\r\n",s->Humid);
    Serial.printf(" Press : %d\r\n",s->Press);
    Serial.printf(" RSSI  : %d\r\n",s->RSSI);
    Serial.printf(" AVS   : %d\r\n",s->AVS);
    Serial.printf(" BATT  : %8.2f\r\n",s->batt);
    Serial.printf(" date  : %d\r\n",s->date);
}

/** ======================================================== Sensor */

/**
 * @brief センサー情報の初期化
 * @param type センサータイプ
 * @param name センサーの名称（LINE表示用）
 * @param ID センサーのID（MACアドレス）
 */
void Sensor::init( const SENS_t type, const String name, const String id, const SensThresh *th,
                    const uint8_t a_templ, const uint8_t a_humid, const uint8_t a_avs ) {
    Type = type;
    Name = name;
    ID = id;
    updated = false;
    thr.copy( th );
    use = true;

    amb_templ = a_templ;
    amb_humid = a_humid;
    amb_avs   = a_avs;  

#if 0 
    Serial.printf("Type: %d\n",Type);
    Serial.printf("Name: %s\n",Name);
    Serial.printf("ID  : %s\n",ID);
    Serial.printf("A_TMP: %d\n",amb_templ);
    Serial.printf("A_HUM: %d\n",amb_humid);
    Serial.printf("A_AVS: %d\n",amb_avs);
#endif
    
}

/**
 * @brief 引数のデータを内部のデータにコピーして更新
 * @param dt コピー元センサーデータ
 */
void Sensor::update( const sData *dt) {
    Serial.println("Sensor::update()");
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

    // 利用状況確認
    if( use )  {
        status = SSTAT_t::disable;
        updated = false;
        return;
    }

    //更新済フラグ
    updated = true;

    // センターデータの状態検知
    status = thr.getStatusTempl( dt->Templ );

    //Sensor::dump(this);
}

/**
 * @brief 現在のセンサデータ
 * @return sData* クラスアドレス
 */
sData* Sensor::getData() {
    return &Data;
}

/**
 * @brief 一つ目のセンサデータ
 * @return sData* クラスアドレス
 */
sData* Sensor::getPrevData(){
    return &prevData;
}

/**
 * @brief TODO 前回更新時間からの間隔確認
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

void Sensor::dump( Sensor *s ) {
    Serial.print("Type          :"); Serial.printf("%d\n",s->Type);
    Serial.print("ID            :"); Serial.printf("%s\n",s->ID);
    Serial.print("Name          :"); Serial.println(s->Name);
    Serial.print("updated       :"); Serial.println(s->updated);
    Serial.print("use           :"); Serial.println(s->use);
    Serial.print("status        :"); Serial.printf("%d\n",s->status);
    Serial.print("amb Temp      :"); Serial.printf("%d\n",s->amb_templ);
    Serial.print("amb Humi      :"); Serial.printf("%d\n",s->amb_humid);
    Serial.print("amb AVS       :"); Serial.printf("%d\n",s->amb_avs);
    Serial.println("--Data.duump()");  sData::dump( &s->Data );
    Serial.println("--prevData.dump()"); sData::dump( &s->prevData );
    Serial.println("--SensThresh.dump()"); SensThresh::dump( &s->thr );
}

/** ======================================================== SensList */

/**
 * @brief Construct a new Sens List:: Sens List object
 */
SensList::SensList(){
    Num = 0;
    EnvS.thr.setTempl( 30.0, 35.0 );  // 室内計への設定
    EnvS.thr.setHumid( 75.0, 80.0 );  // 室内計への設定
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
bool SensList::add( const SENS_t type, const  String id, const  String name , SensThresh th,
                    const uint8_t a_templ, const uint8_t a_humid, const uint8_t a_avs ){
    Serial.println("SensList::add()");
    Serial.printf("Type %d / Name %s / ID: %s \n",type, name.c_str(), id.c_str() );
    
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
    Serial.println("SensList::update()");
    for( int i=0; i < Num; i++  ) {
        if( Sens[i].Type == dt->Type ) {    // 高速化のためTypeで一次判定
            if( Sens[i].ID == dt->ID ) {    // 次に文字列比較(ID)
                Serial.printf("update Sens %d\r\n",i);

                // Update SensorData
                Sens[i].update( dt );
                //Serial.printf("status %d\n",Sens[i].status);

                // TODO Ambientに通知
                // ex: amb->update( amb_num, data, keta );
                
                // TODO Notifyに通知
                if( Sens[i].status != SSTAT_t::normal ) {
                    // ex: notify->set( sen->Data, sen->status )
                }
                return true;
            }
        }
    }
    Serial.println("Not found Sensor");
    return false;
}

/**
 * @brief 本体センサー情報の更新
 * @param dt 
 */
void SensList::updateEnv( const  sData *dt ) {
    EnvS.update( dt );
}


/**
 * @brief 指定されたIDのセンサーを返す
 * @param id センサーのID（MACアドレス）
 * @param type センサーのタイプ
 * @return Sensor* 
 */
Sensor * SensList::getSensor( const String id, const SENS_t type ) {
    for( uint8_t i; i < Num; i++  ) {
        if( Sens[i].Type == type ) {    // 高速化のためTypeで一次判定
            if( Sens[i].ID == id ) {    // 次に文字列比較(ID)
                return &Sens[i];
            }
        }
    }
    return NULL;
}

/**
 * @brief Ambientデータの生成
 * @param dt[] 戻りデータ（バッファは呼び出し元が作成）
 */
void SensList::getAmbientData( st_AMB *dt[] ) {
    for( uint8_t i; i < Num; i++  ) {
        if( Sens[i].amb_templ != 0 ) {
            dt[Sens[i].amb_templ]->dt  = Sens[i].Data.Templ;
            dt[Sens[i].amb_templ]->use = true;
        } 
        if( Sens[i].amb_humid != 0 ) {
            dt[Sens[i].amb_humid]->dt  = Sens[i].Data.Humid;
            dt[Sens[i].amb_humid]->use = true;
        }
        if( Sens[i].amb_avs   != 0 ) {
            dt[Sens[i].amb_avs]->dt    = Sens[i].Data.AVS;
            dt[Sens[i].amb_avs]->use   = true;
        }
    }
}


/**
 * @brief DEBUG dump
 */
void SensList::dump( ) {
    for( int i=0; i < MAX_SENS; i++ ) {
        Serial.print("Type          :"); Serial.printf("%d\r\n",Sens[i].Type);
        Serial.print("ID            :"); Serial.printf("%s\r\n",Sens[i].ID);
        Serial.print("Name          :"); Serial.println(Sens[i].Name);
        Serial.print("updated       :"); Serial.println(Sens[i].updated);
        Serial.print("use           :"); Serial.println(Sens[i].use);
        Serial.print("status        :"); Serial.printf("%d\r\n",Sens[i].status);
        Serial.println("--Data.duump()");  sData::dump( &Sens[i].Data );
        Serial.println("--prevData.dump()"); sData::dump( &Sens[i].prevData );
        Serial.println("--SensThresh.dump()"); SensThresh::dump( &Sens[i].thr );
    }
}
