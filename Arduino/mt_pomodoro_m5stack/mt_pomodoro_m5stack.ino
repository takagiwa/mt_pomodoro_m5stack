//
// Pomodoro Technique Timer for M5 series
//
// (C) Masayuki Takagiwa
//
// GPLv2
//

// in munites
#define PERIOD_WORK 25
#define PERIOD_SHORT_REST 5
#define PERIOD_LONG_REST 15

#define TIMER_COUNTUP false;



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

/*
#define BGCOLOR BLACK
#define FONTCOLOR WHITE
*/
#define BGCOLOR WHITE
#define FONTCOLOR BLACK

unsigned long current_timestamp;
unsigned long prev_timestamp;
unsigned long start_timestamp;
unsigned long battery_last_update;
uint8_t current_min;
uint8_t current_sec;
// set true in each second
boolean sec_inc;
// update texts when needed
boolean drawn_button;

/*
 * enum not work?
enum STATTYPE {
  WAITINTG, WORKING, RESTING1, RESTING2
};
enum STATTYPE run_status;
*/
uint8_t run_status;
uint8_t prev_status;

void setup() {
  M5.begin();
  Wire.begin();
  M5.Power.begin();

  current_timestamp = millis();
  start_timestamp = current_timestamp;
  battery_last_update = 0;
  sec_inc = false;

  run_status = 0;
  prev_status = 9;
  drawn_button = false;

  M5.Lcd.fillScreen(BGCOLOR);

  /*
  int16_t y = 0;

  M5.Lcd.setTextColor(FONTCOLOR);

  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print("Font 1");

  y += 8;

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print("Font 1 x2");

  M5.Lcd.setTextFont(2);

  y += 12;

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print("Font 2");

  y += 8;

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print("Font 2 x2");

  M5.Lcd.setTextFont(4);

  y += 25;

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print("Font 4");

  y += 18;

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print("Font 4 x2");

  M5.Lcd.setTextFont(6);

  y += 38;

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print(" 6");

  y += 40;

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, y);
  M5.Lcd.print(" 6 2");
  */

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

void soundNotes()
{
  M5.Speaker.beep();
}

// b: A=1, B=2, C=3
void printOnButton(uint8_t b, String s, uint8_t n)
{
  int16_t textsize = 2;
  int16_t charheight = 8;
  int16_t charwidth = 4;
  int16_t marginbottom = 2;
  
  int16_t x, y, w, h;

  // clear
  h = (charheight * textsize) + (marginbottom * 2);
  y = M5.Lcd.height() - h;
  w = M5.Lcd.width() / 3;
  switch (b) {
    case 3:
      x = M5.Lcd.width() - w;
      break;
    case 2:
      x = (M5.Lcd.width() / 2) - (w / 2);
      break;
    case 1:
      x = 0;
      break;
    others:
      return;
  }
  M5.Lcd.fillRect(x, y, w, h, BGCOLOR);

  // draw text
  y = M5.Lcd.height() - ((charheight * textsize) + marginbottom);
  switch(b) {
    case 3:
      x = (M5.Lcd.width() * 3 / 4) - (charwidth * n * textsize / 2);
      break;
    case 2:
      x = (M5.Lcd.width() * 2 / 4) - (charwidth * n * textsize / 2);
      break;
    case 1:
      x = (M5.Lcd.width() * 1 / 4) - (charwidth * n * textsize / 2);
      break;
  }
  M5.Lcd.setTextColor(FONTCOLOR);
  M5.Lcd.setTextSize(textsize);
  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(s);
}

void loop() {
  current_timestamp = millis();
  M5.update();

  if ((current_timestamp - prev_timestamp) > 1000) {
    sec_inc = true;
    prev_timestamp = current_timestamp;
  } else {
    sec_inc = false;
  }

  if ((battery_last_update == 0)
      || ((current_timestamp - battery_last_update) > (BATTERY_CHECK_INTERVAL_IN_SEC * 1000))) {

    /* battery indicator: user define */
    int16_t b_margin = 1;
    int16_t b_border = 1*2;
    int16_t b_padding = 1;
    int16_t b_cell_width = 2*2;
    int16_t b_cell_height = 4*2;

    int16_t x, y, w, h;
    w = b_margin + b_border + b_padding + b_cell_width + b_padding + b_cell_width + b_padding + b_cell_width + b_padding + b_cell_width + b_padding + b_border + b_margin;
    x = M5.Lcd.width() - w;
    y = 0;
    h = b_margin + b_border + b_padding + b_cell_height + b_padding + b_border + b_margin;
    // margin
    M5.Lcd.fillRect(x, y, w, h, BLACK); // background
    // border
    M5.Lcd.fillRect(x+b_margin, y+b_margin, w-(2*b_margin), h-(2*b_margin), WHITE);
    // padding
    M5.Lcd.fillRect(x+b_margin+b_border, y+b_margin+b_border, w-(2*b_margin)-(2*b_border), h-(2*b_margin)-(2*b_border), BLACK);
    uint8_t b = getBatteryLevel();
    // cells
    if (b < 1) {
      // error
      M5.Lcd.fillRect(x+b_margin+b_border+b_padding, y+b_margin+b_border+b_padding, w-(b_margin+b_border+b_padding+b_padding+b_border+b_margin), h-(b_margin+b_border+b_padding+b_padding+b_border+b_margin), YELLOW);
    } else {
      // 25%
      if (b == 25) {
        M5.Lcd.fillRect(x+b_margin+b_border+b_padding, y+b_margin+b_border+b_padding, b_cell_width, b_cell_height, RED);
      } else {
        M5.Lcd.fillRect(x+b_margin+b_border+b_padding, y+b_margin+b_border+b_padding, b_cell_width, b_cell_height, GREEN);
      }
      if (b >= 50) {
        // 50%
        M5.Lcd.fillRect(x+b_margin+b_border+b_padding+b_cell_width+b_padding, y+b_margin+b_border+b_padding, b_cell_width, b_cell_height, GREEN);
      }
      if (b >= 75) {
        // 75%
        M5.Lcd.fillRect(x+b_margin+b_border+b_padding+b_cell_width+1+b_cell_width+b_padding, y+b_margin+b_border+b_padding, b_cell_width, b_cell_height, GREEN);
      }
      if (b >= 100) {
        // 100% or charging
        M5.Lcd.fillRect(x+b_margin+b_border+b_padding+b_cell_width+1+b_cell_width+b_padding+b_cell_width+b_padding, y+b_margin+b_border+b_padding, b_cell_width, b_cell_height, GREEN);
      }
    }
    battery_last_update = current_timestamp;
  }

  int8_t cnt;
  cnt = getControlRes();

  // 画面表示は、右上にバッテリー状態
  // 真ん中に大きく時間 XX.yy (XXの分を大きく)
  // オーバーしたら + 付きで表示
  // 上に小さく、作業時間、左のボタン情報の上に長い休みの時間、右は短い休みの時間
  // あと、それぞれの休みの回数もどこかに。
  // 上には作業の回数を。

  if (sec_inc) {
    int16_t dy = 30;
    // clear
    M5.Lcd.fillRect(0, dy, 100, 18, BGCOLOR);
    // draw
    char f = ' ';
    char time_str[16] = "";
    if ((run_status == 2) || (run_status == 5)) {
      f = '+';
    }
    sprintf(time_str, " %c%2d.%02d", f, current_min, current_sec);
    M5.Lcd.setTextColor(FONTCOLOR);
    M5.Lcd.setCursor(0, dy);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print(time_str);
  }
  

  
  if (true) {
    if (run_status != prev_status) {
      int16_t dy = 10;
      // clear
      M5.Lcd.fillRect(0, dy, 200, 18, BGCOLOR);
      // draw
      M5.Lcd.setTextColor(FONTCOLOR);
      M5.Lcd.setCursor(5, dy);
      M5.Lcd.setTextSize(2);
      switch (run_status) {
        case 5:
          M5.Lcd.print("Rest (ended)");
          break;
        case 4:
          M5.Lcd.print("Rest (long)");
          break;
        case 3:
          M5.Lcd.print("Rest (short)");
          break;
        case 2:
          M5.Lcd.print("Work (ended)");
          break;
        case 1:
          M5.Lcd.print("Work");
          break;
        default:
          M5.Lcd.print("Standby");
          break;
      }
      prev_status = run_status;
    }
  }

  switch (run_status) {
    case 5: // rest time up
      if (drawn_button == false) {
        printOnButton(1, "", 1);
        printOnButton(2, "START", 3);
        printOnButton(3, "", 1);
        drawn_button = true;
      }
      if (sec_inc) {
        // TODO: Show "+"
        if (current_min < 60) {
          if (current_sec == 59) {
            current_min += 1;
            current_sec = 0;
          } else {
            current_sec += 1;
          }
        }
      }
      if (cnt == 2) {
        // start
        run_status = 1;
        drawn_button = false;
        start_timestamp = current_timestamp;
        current_min = PERIOD_WORK;
        current_sec = 0;
      }
      break;

    case 4: // long rest
      if (drawn_button == false) {
        printOnButton(1, "", 1);
        printOnButton(2, "STOP", 4);
        printOnButton(3, "", 1);
        drawn_button = true;
      }
      if (sec_inc) {
        if ((current_min == 0) && (current_sec == 0)) {
          run_status = 5;
          drawn_button = false;
          current_sec = 1;
          soundNotes();
        } else {
          // count
          if (current_sec == 0) {
            current_min -= 1;
            current_sec = 59;
          } else {
            current_sec -= 1;
          }
        }
      }
      break;

    case 3: // short rest
      if (drawn_button == false) {
        printOnButton(1, "", 1);
        printOnButton(2, "STOP", 4);
        printOnButton(3, "", 1);
        drawn_button = true;
      }
      if (sec_inc) {
        if ((current_min == 0) && (current_sec == 0)) {
          run_status = 5;
          drawn_button = false;
          current_sec = 1;
          soundNotes();
        } else {
          // count
          if (current_sec == 0) {
            current_min -= 1;
            current_sec = 59;
          } else {
            current_sec -= 1;
          }
        }
      }
      break;

    case 2: // time up
      if (drawn_button == false) {
        printOnButton(1, "LONG", 4);
        printOnButton(2, "END", 3);
        printOnButton(3, "SHORT", 5);
        drawn_button = true;
      }
      if (sec_inc) {
        // TODO: Show "+"
        if (current_min < 60) {
          if (current_sec == 59) {
            current_min += 1;
            current_sec = 0;
          } else {
            current_sec += 1;
          }
        }
      }
      if (cnt == 3) {
        // short rest
        run_status = 3;
        drawn_button = false;
        current_min = PERIOD_SHORT_REST;
        current_sec = 0;
      }
      if (cnt == 2) {
        // end
        run_status = 0;
        drawn_button = false;
      }
      if (cnt == 1) {
        // long lest
        run_status = 4;
        drawn_button = false;
        current_min = PERIOD_LONG_REST;
        current_sec = 0;
      }
      break;

    case 1: // count
      if (drawn_button == false) {
        printOnButton(1, "", 1);
        printOnButton(2, "STOP", 4);
        printOnButton(3, "", 1);
        drawn_button = true;
      }
      if (sec_inc) {
        if ((current_min == 0) && (current_sec == 0)) {
          run_status = 2;
          current_sec = 1;
          drawn_button = false;
          soundNotes();
        } else {
          // count
          if (current_sec == 0) {
            current_min -= 1;
            current_sec = 59;
          } else {
            current_sec -= 1;
          }
        }
      }
      break;

    default: // waiting
      if (drawn_button == false) {
        printOnButton(1, "", 1);
        printOnButton(2, "START", 5);
        printOnButton(3, "", 1);
        drawn_button = true;
      }
      if (cnt == 2) {
        run_status = 1;
        drawn_button = false;
        start_timestamp = current_timestamp;
        current_min = PERIOD_WORK;
        current_sec = 0;
        break;
      }

      run_status = 0;
      break;
  }
}
