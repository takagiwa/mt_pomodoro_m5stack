//
// Pomodoro Technique Timer for M5 series
//
// (C) Masayuki Takagiwa
//
// GPLv2
//

// [NOT IMPREMENTED] Choose your model first.

//#define CORE_BASIC
//#define CORE_GRAY
#define CORE_FIRE_GO_LITE_GO
//#define STICK_WHITE
//#define STICK_GRAY
//#define STICK_C
//#define STICK_V
//#define ATOM_LITE
//#define ATOM_MATRIX

#define BATTERY_CHECK_INTERVAL_IN_SEC 60


// define must ahead #include <M5Stack.h>
#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
// #define M5STACK_MPU6050
// #define M5STACK_200Q

#include <M5Stack.h>

unsigned long current_timestamp;
unsigned long battery_last_update;

enum STATTYPE {
  WAITINTG, WORKING, RESTING1, RESTING2
} run_status;


void setup() {
  M5.begin();
  Wire.begin();
  M5.Power.begin();

  current_timestamp = millis();
  battery_last_update = 0;

  run_status = WAITING;
}

// https://github.com/m5stack/M5Stack/issues/74
int8_t getBatteryLevel()
{
  
  Wire.beginTransmission(0x75);
  Wire.write(0x78);
  if (Wire.endTransmission(false) == 0
   && Wire.requestFrom(0x75, 1)) {
    switch (Wire.read() & 0xF0) {
    case 0xE0: return 25;
    case 0xC0: return 50;
    case 0x80: return 75;
    case 0x00: return 100;
    default: return 0;
    }
  }
  return -1;
}

int8_t getControlRes()
{
  if (M5.BtnA.wasReleased()) {
    return 1;
  }
  if (M5.BtnB.wasReleased()) {
    return 2;
  }
  if (M5.BtnC.wasReleased()) {
    return 3;
  }
  return 0;
}


void loop() {
  current_timestamp = millis();
  M5.update();

  if ((battery_last_update == 0)
      || ((current_timestamp - battery_last_update) > (BATTERY_CHECK_INTERVAL_IN_SEC*1000))) {
    M5.Lcd.clear();
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Battery Level: ");
    M5.Lcd.print(getBatteryLevel());
    M5.Lcd.print("%");
    battery_last_update = current_timestamp;
  }

  int8_t cnt;
  cnt = getControlRes();
  /*
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.setTextSize(4);
  M5.Lcd.print("Button: ");
  M5.Lcd.print(cnt);
  */

  switch(run_status) {
    case RESTING2:

      break;
     case RESTING1:

      break;
     case WORKING:
     


      break;
     default:
      if (cnt == 2) {
        run_status = WORKING;
        break;
      }
      run_status = WAITING;
      break;
  }

  
}
