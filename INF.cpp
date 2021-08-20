/**
 * @file INF.cpp
 * @author F.Takahashi (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "INF.h"

// 初期値ロード
bool INF::load(){
    const size_t  buff_len = 809;
    char          buff[buff_len];

    Serial.println("Load INI file.");
    if (!SD.begin()) {
    Serial.println(" Please Insert SD-CARD and RESET! ");
    M5.Lcd.println(" Please Insert SD-CARD and RESET! ");
    while (1) ;
    }

    IniFile in(iniFile);

    // File Open
    if (!in.open()) {
    Serial.printf("ERROR Ini file [%s] does not exist. \r\n STOP!", iniFile);
    while (1);
    }
    Serial.printf("INI file [ %s ] exists\r\n", iniFile);

    // Check File is Valid.
    if (!in.validate(buff, buff_len)) {
    Serial.printf("ini file %s not valid\r\n", in.getFilename());
    printErrorMessage(in.getError());
    // Cannot do anything else
    while (1);
    }

    // ======== Wi-Fi
    wifi_ssid = getValueSTR( in, "Wi-Fi", "SSID", buff, buff_len );
    wifi_key  = getValueSTR( in, "Wi-Fi", "KEY", buff, buff_len );

    // ====== Inkbird
    String sect_th1 = String("TH1-1");
    for ( int i = 0; i < 3; i++ ) {
    snse_th1[i].MAC       = getValueSTR( in, sect_th1.c_str(), "MAC", buff, buff_len );
    snse_th1[i].name      = getValueSTR( in, sect_th1.c_str(), "name", buff, buff_len );
    snse_th1[i].amb_templ = getValueSTR( in, sect_th1.c_str(), "templ", buff, buff_len );
    snse_th1[i].amb_humid = getValueSTR( in, sect_th1.c_str(), "humid", buff, buff_len );
    snse_th1[i].th_templ.use = getValueBOOL( in, sect_th1.c_str(), "thr1_use", buff, buff_len );
    if(snse_th1[i].th_templ.use) {
        snse_th1[i].th_templ.min = getValueFLOAT( in, sect_th1.c_str(), "thr1_min", buff, buff_len );
        snse_th1[i].th_templ.max = getValueFLOAT( in, sect_th1.c_str(), "thr1_max", buff, buff_len );
    }
    snse_th1[i].th_humid.use = getValueBOOL( in, sect_th1.c_str(), "thr2_use", buff, buff_len );
    if(snse_th1[i].th_humid.use) {
        snse_th1[i].th_humid.min = getValueFLOAT( in, sect_th1.c_str(), "thr2_min", buff, buff_len );
        snse_th1[i].th_humid.max = getValueFLOAT( in, sect_th1.c_str(), "thr2_max", buff, buff_len );
    }
    sect_th1.setCharAt( 4, (char)(50+i) );
    Serial.printf(" STR:%s (%c)\r\n",sect_th1, (char)(49+i)); 
    }

    // ===== 920J

    // ===== Ambient

    // ===== LINE

    // ===== GAS

}

// INI Value String 取得
const char *INF::getValueSTR( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len ) {
    if ( in.getValue( sect, name, buff, buff_len ) ) {
    Serial.printf("[%s] %s: %s\r\n", sect, name, buff);
    return buff;
    } else {
    Serial.printf("Not found [%s] [%s] : ", sect, name );
    printErrorMessage(in.getError());
    return NULL;
    }
}

// INI Bool 取得
bool INF::getValueBOOL( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len ) {
    bool val = false;
    if ( in.getValue( sect, name, buff, buff_len , val ) ) {
    Serial.printf("[%s] [%s]: %s\r\n", sect, name, val ? "TRUE":"FALSE");
    return val;
    } else {
    Serial.printf("Not found [%s] [%s] : ", sect, name );
    printErrorMessage(in.getError());
    return false;
    }
}

// INI float 取得
float INF::getValueFLOAT( IniFile in, const char *sect, const char *name, char *buff, size_t buff_len ) {
    float val = 0.0f;
    if ( in.getValue( sect, name, buff, buff_len , val) ) {
    Serial.printf("[%s] [%s]: %s\r\n", sect, name, buff);
    return val;
    } else {
    Serial.printf("Not found [%s] [%s] : ", sect, name );
    printErrorMessage(in.getError());
    return 0.0f;
    }
}

// エラーメッセージ変換
void INF::printErrorMessage(uint8_t e, bool eol ) {
    switch (e) {
    case IniFile::errorNoError:
        Serial.print("no error");
        break;
    case IniFile::errorFileNotFound:
        Serial.print("file not found");
        break;
    case IniFile::errorFileNotOpen:
        Serial.print("file not open");
        break;
    case IniFile::errorBufferTooSmall:
        Serial.print("buffer too small");
        break;
    case IniFile::errorSeekError:
        Serial.print("seek error");
        break;
    case IniFile::errorSectionNotFound:
        Serial.print("section not found");
        break;
    case IniFile::errorKeyNotFound:
        Serial.print("key not found");
        break;
    case IniFile::errorEndOfFile:
        Serial.print("end of file");
        break;
    case IniFile::errorUnknownError:
        Serial.print("unknown error");
        break;
    default:
        Serial.print("unknown error value");
        break;
    }
    if (eol)
    Serial.println();
}
