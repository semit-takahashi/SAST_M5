#include "SAST_Recv_ide.h"		// Additional Header

/* FILE NAME: SAST_Recv.c
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 SETM-IT.
 * All rights reserved.
*/

#define SUBGHZ_CH		25					// 送信チャネル（ 
#define SUBGHZ_PANID	0x2310				// PAN ID
#define	SUBGHZ_ADDR		0xFFFF				// 受信先（0xffff＝ブロードキャスト）

#define SR_BUFF_SIZE	64					// シリアル受信バッファサイズ
#define RF_BUFF_SIZE	256					// SUBGH受信バッファサイズ

uint8_t rx_data[RF_BUFF_SIZE];						// 受信バッファ
uint32_t last_recv_time = 0;
SUBGHZ_STATUS rx;							// structure for getting rx status

#define BL_LED		26
#define YL_LED		25
#define RD_LED		20
#define LED_ON		LOW
#define LED_OFF		HIGH


// Encrypt Key （= NULL is None）
//static const unsigned char key[] = {0x07,0xa4,0x5e,0x45,0x43,0x18,0xb6,0xd4,0xa9,0xd5,0x44,0x9f,0x3b,0x34,0x2b,0xda};
static const unsigned char key = 0x00;

uint8_t baud = 100;
uint8_t ch = SUBGHZ_CH;

// Print HEX data
void print_hex_func(uint8_t data)
{
  if (data == 0) {
    Serial.print("00");
  }
  else {
    if (data < 16) {
      Serial.print("0");
      Serial.print_long(data, HEX);
    } else {
      Serial.print_long(data, HEX);
    }
  }
}

// Serialに特定の文字列が来るまで待機
void wait_string( char * rData )
{
	int16_t  	recv = NULL;
	uint8_t   	rBuff[SR_BUFF_SIZE];
	uint16_t	rBuffIndex = 0;
	uint16_t  	len = 0;

	len = strlen(rData,SR_BUFF_SIZE);
	//Serial.println_long(len,DEC);

	while(1) {
		//Serial.println("Read buff");
		recv = Serial.read();
		if (recv >= 0) {
			//Serial.println_long(recv, HEX);
			rBuff[rBuffIndex] = recv;
			rBuffIndex++;
		}

		if( rBuffIndex == SR_BUFF_SIZE ) {
			//Serial.println("Buffer Overflow!!");
			rBuffIndex = 0;
			Serial.flush();
			continue;
		}
		
		if( recv == 10 || recv == 13 ) {
			//Serial.println("recv CR/LF");
			rBuff[rBuffIndex] = NULL; //terminated
			if( strncmp( rData, rBuff, len ) == 0 ){
				//Serial.println("Match");
				Serial.flush();
				return;
			}
			else {
				//Serial.println("Unmatch");
				rBuffIndex = 0;
				recv = 0;
				continue;
			}
		}
		delay(100);
	}
}


// 無線アドレス表示 
void print_MyAddress() {
	uint8_t myAddr[8];				// 自分自身のADDR
	SubGHz.getMyAddr64(myAddr);
	Serial.print("myAddress = ");
	print_hex_func(myAddr[0]);
	print_hex_func(myAddr[1]);
	Serial.print(" ");
	print_hex_func(myAddr[2]);
	print_hex_func(myAddr[3]);
	Serial.print(" ");
	Serial.print(" ");
	print_hex_func(myAddr[4]);
	print_hex_func(myAddr[5]);
	Serial.print(" ");
	print_hex_func(myAddr[6]);
	print_hex_func(myAddr[7]);
	Serial.println("");
}


// 初期化
void setup(void)
{
  SUBGHZ_MSG msg;					// 受信メッセージ構造体（
  char rbuff[64];					// シリアル受信バッファ
  char wKey[] = "SAST";				// ホスト受信待機文字列

  // シリアルポート初期化
  Serial.begin(115200);

  // LED初期化
  pinMode(BL_LED, OUTPUT);
  pinMode(YL_LED, OUTPUT);
  digitalWrite(BL_LED, LED_OFF);
  digitalWrite(YL_LED, LED_OFF);

  delay(10);
  digitalWrite(YL_LED, LED_ON);

  // SubGHz通信初期化
  msg = SubGHz.init();
  if (msg != SUBGHZ_OK)  {
    SubGHz.msgOut(msg);
    while (1) { }
  }

  // AESキーの設定
  SubGHz.setKey(key);

  // SubGHz通信の設定
  msg = SubGHz.begin(ch, SUBGHZ_PANID,  baud, SUBGHZ_PWR_20MW);
  if (msg != SUBGHZ_OK)  {
    SubGHz.msgOut(msg);
    while (1) { }
  }
  msg = SubGHz.rxEnable(NULL);
  if (msg != SUBGHZ_OK)  {
    SubGHz.msgOut(msg);
    while (1) { }
  }

  digitalWrite(YL_LED, LED_ON);

  wait_string( wKey );
  delay(1000);
  Serial.println("Lazurite Ready");

  digitalWrite(YL_LED, LED_OFF);

  //print_MyAddress();
 
  return;
}

// メインループ
void loop(void)
{
  SUBGHZ_MAC_PARAM mac;
  SUBGHZ_MSG msg;
  short rx_len;
  short index = 0;
  uint16_t data16;

  // 無線データの受信
  rx_len = SubGHz.readData(rx_data, sizeof(rx_data));
  rx_data[rx_len] = 0;
  if (rx_len > 0)
  {
    digitalWrite(BL_LED, LED_ON);
    SubGHz.getStatus(NULL, &rx);										// get status of rx
    SubGHz.decMac(&mac, rx_data, rx_len);
    // 本体（自分）の起動からの経過時間（ミリ秒）
    //Serial.print_long(millis(), DEC);
    //Serial.print("\t");

    // MAC ヘッダー
    //Serial.print_long(mac.mac_header.fc16, HEX);
    //Serial.print("\t");

    // 送信シーケンス NO（ソース側付与）
    Serial.print_long(mac.seq_num, HEX);
    Serial.print(",");

    // PAN-ID
    //Serial.print_long(mac.dst_panid, HEX);
    //Serial.print("\t");

    // 受け側ADDR（送信先付与＝0xffffはブロードキャスト）
    //print_hex_func(mac.dst_addr[7]);
    //print_hex_func(mac.dst_addr[6]);
    //print_hex_func(mac.dst_addr[5]);
    //print_hex_func(mac.dst_addr[4]);
    //print_hex_func(mac.dst_addr[3]);
    //print_hex_func(mac.dst_addr[2]);
    //print_hex_func(mac.dst_addr[1]);
    //print_hex_func(mac.dst_addr[0]);
    //Serial.print("\t");

    //送信元ADDR
    //print_hex_func(mac.src_addr[7]);
    //print_hex_func(mac.src_addr[6]);
    //print_hex_func(mac.src_addr[5]);
    //print_hex_func(mac.src_addr[4]);
    //print_hex_func(mac.src_addr[3]);
    //print_hex_func(mac.src_addr[2]);
    print_hex_func(mac.src_addr[1]);
    print_hex_func(mac.src_addr[0]);
    Serial.print(",");

    // RSSI
    Serial.print_long(rx.rssi, DEC);
    Serial.print(",");

    // ペイロード（送信データ）
    Serial.print(mac.payload);


    delay( 100 );
    digitalWrite(BL_LED, LED_OFF);
  }
  delay( 10 );
  return;
}
