#include "SAST_Send_ide.h"		// Additional Header

/* Cusomized F.Takahashi 2021年8月14日
 * センサー出力を10秒単位（通信およびシリアル）
 *  温度（℃）、湿度（％）、気圧（hPa）、照度（lux）、近接距離（PS）、電池電圧（V）
 */

// LED Define
#define LED_BL 26
#define LED_YL 25
#define LED_RD 20
#define LED_ON LOW
#define LED_OFF HIGH

// SubGHz
#define SUBGHZ_CH		25			// ch 25
#define SUBGHZ_PANID	0x2310		// PAN-ID 0x2310
#define HOST_ADDRESS	0xffff		// HOST Broadcast(0xffff)

uint8_t i2c_addr = 0x76;        //I2C Address
uint8_t osrs_t = 1;             //Temperature oversampling x 1
uint8_t osrs_p = 1;             //Pressure oversampling x 1
uint8_t osrs_h = 1;             //Humidity oversampling x 1
uint8_t bme280mode = 1;         //Normal mode
uint8_t t_sb = 5;               //Tstandby 1000ms
uint8_t filter = 0;             //Filter off
uint8_t spi3w_en = 0;           //3-wire SPI Disable
uint8_t txdata[128];

bool timer_flag = false;

uint32_t interval = 60;

const char vls_val[][8] ={
	"0",		//	"invalid",
	"0",		//	"invalid",
	"1.8",		//	"< 1.898V"
	"1.95",		//	"1.898 ~ 2.000V"
	"2.05",		//	"2.000 ~ 2.093V"
	"2.15",		//	"2.093 ~ 2.196V",
	"2.25",		//	"2.196 ~ 2.309V",
	"2.35",		//	"2.309 ~ 2.409V",
	"2.5",		//	"2.409 ~ 2.605V",
	"2.7",		//	"2.605 ~ 2.800V",
	"2.9",		//	"2.800 ~ 3.068V",
	"3.2",		//	"3.068 ~ 3.394V",
	"3.6",		//	"3.394 ~ 3.797V",
	"4.0",		//	"3.797 ~ 4.226V",
	"4.4",		//	"4.226 ~ 4.667V",
	"4.7"		//	"> 4.667V"
};

void timer_isr(void)
{
  timer_flag = true;
}


void sendData() {
	// put your main code here, to run repeatedly:
	byte rc;
	double temp_act, press_act, hum_act; //最終的に表示される値を入れる変数
	unsigned short ps_val;
	float als_val;
	uint8_t level;
	SUBGHZ_MSG msg;

	digitalWrite(LED_BL, LED_ON);
	Serial.println("Send Data...");
	
	bme280.setMode(i2c_addr, osrs_t, osrs_p, osrs_h, bme280mode, t_sb, filter, spi3w_en);
	bme280.readData(&temp_act, &press_act, &hum_act);
	rc = rpr0521rs.get_oneShot(&ps_val, &als_val);
	level = voltage_check(VLS_4_667);
	
	Print.init(txdata,sizeof(txdata));
	Print.d(temp_act,1);
	Print.p(",");
	Print.d(hum_act,1);
	Print.p(",");
	Print.d(press_act,0);
	Print.p(",");
	Print.d(als_val,3);
	Print.p(",");
	Print.d(ps_val,0);
	Print.p(",");
	Print.p(vls_val[level]);
	Print.ln();

	Serial.print(txdata);

	// SubGHz init ( 出力 1mW )
	SubGHz.begin(SUBGHZ_CH, SUBGHZ_PANID, SUBGHZ_100KBPS, SUBGHZ_PWR_1MW);
	msg=SubGHz.send(SUBGHZ_PANID, HOST_ADDRESS, txdata, Print.len(),NULL);	// make messge
	SubGHz.msgOut(msg);		//send 
	SubGHz.close();			// close
	
	if( msg != SUBGHZ_OK ) {
		// ERROR Occord 
		digitalWrite(LED_YL, LED_ON);
		sleep(1000 );
		digitalWrite(LED_YL, LED_OFF);
	}

	digitalWrite(LED_BL, LED_OFF);

	return;
}

void setup() {
	// put your setup code here, to run once:
	byte rc;
	
	pinMode(LED_YL, OUTPUT);
	pinMode(LED_BL, OUTPUT);
	//pinMode(LED_RD, OUTPUT);
	//digitalWrite(LED_YL, LED_ON);
	
	// Setup Serial
	Serial.begin(115200);
	Serial.print(__DATE__); Serial.print(" "); Serial.print(__TIME__);Serial.println(""); 

	// Setup SubGHz
	SubGHz.init();			// SubGHz to enter standby mode after init
	SubGHz.begin(SUBGHZ_CH, SUBGHZ_PANID, SUBGHZ_100KBPS, SUBGHZ_PWR_1MW);		// setup params
	SubGHz.rxEnable(NULL);														// Recv None
	SubGHz.setBroadcastEnb(false);												// ブロードキャスト通知なし
	//Serial.println_long(SubGHz.getMyAddress,HEX);

	// Setup BME280
	Wire.begin();
	bme280.setMode(i2c_addr, osrs_t, osrs_p, osrs_h, bme280mode, t_sb, filter, spi3w_en);
	bme280.readTrim();

	// Setup LightSensor
	rc = rpr0521rs.init();							// not standby mode after this process;
	rc = rpr0521rs.get_oneShot(NULL, NULL);		// set standby after data is read

	// Send Timer Start 
	Serial.print("Interval : "); Serial.print_long(interval,DEC); Serial.println(" sec");
	timer2.set( ( interval * 1000L ),timer_isr);
	timer2.start();


	//digitalWrite(LED_YL, LED_OFF);

	// End Setup
	Serial.println("End Setup");
	
	// Send First DATA
	sendData();

	Serial.println("Enter Main Loop...");
	
}


void checkCMD()
{
	return;
}


void loop() {

	// Wait 
	wait_event(&timer_flag);

	// Send Data
	sendData();

	// Check Command
	checkCMD();

	return;
}
