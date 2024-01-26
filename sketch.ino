#include <Wire.h>
#include <LiquidCrystal_I2C.h>
const byte rtcDS1307 = 0x68; // địa chỉ mặc địch của DS1307 trong I2C
LiquidCrystal_I2C lcd(0x27,16,2);
const int modeButtonPin = 13; // Nút nhấn thay đổi chế độ nối với chân 13 của arduino
const int day_hourButtonPin = 12; // Nút nhấn tăng ngày/giờ nối với chân 12 của arduino
const int month_minuteButtonPin = 11; // Nút nhấn tăng tháng/phút nối với chân 11 của arduino
const int yearup_secondButtonPin = 10; // Nút nhấn tăng năm/giây nối với chân 10 của arduino
const int yeardown_alarmButtonPin = 9; // Nút nhấn giảm năm/bật tắt báo thức nối với chân 9 của arduino
const int confirmButtonPin = 8; // Nút nhấn hoàn tất điều chỉnh nối với chân 8 của arduino
const int buzzerPin = 7; // Buzzer nối với chân 7 của arduino
int second, minute, hour, day, month, year; 
int alarm_hour = 0, alarm_minute = 0, alarm_state = 0;
int temp_hour,temp_minute,temp_state;
int mode = 0;
// trạng thái gần nhất của các chân nối với các nút bấm
int prev_modeButtonValue = HIGH;
int prev_day_hourButtonValue = HIGH;
int prev_month_minuteButtonValue = HIGH;
int prev_yearup_secondButtonValue = HIGH;
int prev_yeardown_alarmButtonValue = HIGH;
int prev_confirmButtonValue = HIGH;
int finishAlarm = 0;

int bcd2dec(byte num);
void readDS1307();
void setTimeRTC(int _second,int _minute,int _hour);
void setDateRTC(int _day,int _month,int _year);
void displayLCDCurrentTime();
void displayLCDDateAdjusted();
void displayLCDTimeAdjusted();
void displayLCDAlarmSetting();
void setAlarm();

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(modeButtonPin, INPUT_PULLUP);
  pinMode(day_hourButtonPin, INPUT_PULLUP);
  pinMode(month_minuteButtonPin, INPUT_PULLUP);
  pinMode(yearup_secondButtonPin, INPUT_PULLUP);
  pinMode(yeardown_alarmButtonPin, INPUT_PULLUP);
  pinMode(confirmButtonPin, INPUT_PULLUP);
  pinMode(buzzerPin,OUTPUT);
}
void loop() {
  int modeButtonValue = digitalRead(modeButtonPin);
  if(modeButtonValue != prev_modeButtonValue) {
    // Nếu nhấn nút chuyển chế độ
    if(modeButtonValue == LOW) {
      mode++;
      if(mode == 3) {
        temp_hour = alarm_hour;
        temp_minute = alarm_minute;
        temp_state = alarm_state;
      }
      lcd.clear();
      finishAlarm = 1;
    }
    prev_modeButtonValue = modeButtonValue;
  }
  
  // Chế độ hiển thị ngày giờ
  if(mode == 0) {
    readDS1307();
    displayLCDCurrentTime();
    if(hour == alarm_hour && minute == alarm_minute && second == 0) finishAlarm = 0;
    if(alarm_state == 1 && !finishAlarm) {
      if(alarm_minute <= 50) {
        if(hour == alarm_hour && minute >= alarm_minute && minute < alarm_minute + 10) tone(buzzerPin,200);
        else {
          finishAlarm = 1;
          noTone(buzzerPin);
        }
      }
      else {
        if((hour == alarm_hour && minute >= alarm_minute) || (hour == alarm_hour + 1 && minute < alarm_minute - 50)) 
          tone(buzzerPin,200);
        else {
          finishAlarm = 1;
          noTone(buzzerPin);
        } 
      }
    }
    else noTone(buzzerPin);

    int confirmButtonValue = digitalRead(confirmButtonPin);
    if(confirmButtonValue != prev_confirmButtonValue) {
      if(confirmButtonValue == LOW) finishAlarm = 1;
      prev_confirmButtonValue = confirmButtonValue;
    }
    
  }
  // Chế độ điều chỉnh giờ
  else if(mode == 1) {
    displayLCDTimeAdjusted();
    int day_hourButtonValue = digitalRead(day_hourButtonPin);
    if(day_hourButtonValue != prev_day_hourButtonValue) {
      if(day_hourButtonValue == LOW) {
        hour++;
        if(hour == 24) hour = 0;
      }
      prev_day_hourButtonValue = day_hourButtonValue;
    }

    int month_minuteButtonValue = digitalRead(month_minuteButtonPin);
    if(month_minuteButtonValue != prev_month_minuteButtonValue) {
      if(month_minuteButtonValue == LOW) {
        minute++;
        if(minute == 60) minute = 0;
      }
      prev_month_minuteButtonValue = month_minuteButtonValue;
    }

    int yearup_secondButtonValue = digitalRead(yearup_secondButtonPin);
    if(yearup_secondButtonValue != prev_yearup_secondButtonValue) {
      if(yearup_secondButtonValue == LOW) {
        second++;
        if(second == 60) second = 0;
      }
      prev_yearup_secondButtonValue = yearup_secondButtonValue;
    }

    int confirmButtonValue = digitalRead(confirmButtonPin);
    if(confirmButtonValue != prev_confirmButtonValue) {
      if(confirmButtonValue == LOW) {
        setTimeRTC(second,minute,hour);
        mode = 0;
        lcd.clear();
      }
      prev_confirmButtonValue = confirmButtonValue;
    }
  }
  // Chế đó điều chỉnh ngày
  else if(mode == 2) {
    displayLCDDateAdjusted();
    int day_hourButtonValue = digitalRead(day_hourButtonPin);
    if(day_hourButtonValue != prev_day_hourButtonValue) {
      if(day_hourButtonValue == LOW) {
        day++;
        int daylimit[] = {32,29,32,31,32,31,32,32,31,32,31,32};
        if(year % 4 == 0) daylimit[1] = 30;
        if(day == daylimit[month-1]) day = 1;
      }
      prev_day_hourButtonValue = day_hourButtonValue;
    }

    int month_minuteButtonValue = digitalRead(month_minuteButtonPin);
    if(month_minuteButtonValue != prev_month_minuteButtonValue) {
      if(month_minuteButtonValue == LOW) {
        month++;
        if(month == 13) minute = 1;
      }
      prev_month_minuteButtonValue = month_minuteButtonValue;
    }

    int yearup_secondButtonValue = digitalRead(yearup_secondButtonPin);
    if(yearup_secondButtonValue != prev_yearup_secondButtonValue) {
      if(yearup_secondButtonValue == LOW) {
        year++;
        if(year == 2100) second = 2000;
      }
      prev_yearup_secondButtonValue = yearup_secondButtonValue;
    }

    int yeardown_alarmButtonValue = digitalRead(yeardown_alarmButtonPin);
    if(yeardown_alarmButtonValue != prev_yeardown_alarmButtonValue) {
      if(yeardown_alarmButtonValue == LOW) {
        year--;
        if(year == 1999) year = 2000;
      }
      prev_yeardown_alarmButtonValue = yeardown_alarmButtonValue;
    }

    int confirmButtonValue = digitalRead(confirmButtonPin);
    if(confirmButtonValue != prev_confirmButtonValue) {
      if(confirmButtonValue == LOW) {
        setDateRTC(day,month,year);
        mode = 0;
        lcd.clear();
      }
      prev_confirmButtonValue = confirmButtonValue;
    }
  }
  // Chế đố cài báo thức
  else if(mode == 3) {
    displayLCDAlarmSetting();
    int day_hourButtonValue = digitalRead(day_hourButtonPin);
    if(day_hourButtonValue != prev_day_hourButtonValue) {
      if(day_hourButtonValue == LOW) {
        temp_hour++;
        if(temp_hour == 24) hour = 0;
      }
      prev_day_hourButtonValue = day_hourButtonValue;
    }

    int month_minuteButtonValue = digitalRead(month_minuteButtonPin);
    if(month_minuteButtonValue != prev_month_minuteButtonValue) {
      if(month_minuteButtonValue == LOW) {
        temp_minute++;
        if(temp_minute == 60) temp_minute = 0;
      }
      prev_month_minuteButtonValue = month_minuteButtonValue;
    }

    int yeardown_alarmButtonValue = digitalRead(yeardown_alarmButtonPin);
    if(yeardown_alarmButtonValue != prev_yeardown_alarmButtonValue) {
      if(yeardown_alarmButtonValue == LOW) temp_state = 1 - temp_state;
      prev_yeardown_alarmButtonValue = yeardown_alarmButtonValue;
    }

    int confirmButtonValue = digitalRead(confirmButtonPin);
    if(confirmButtonValue != prev_confirmButtonValue) {
      if(confirmButtonValue == LOW) {
        setAlarm();
        mode = 0;
        lcd.clear();
      }
      prev_confirmButtonValue = confirmButtonValue;
    }
  }
  else mode = 0;
}
// Chuyển đổi mã bcd sang nhị phân và ngược lại để truyền nhân dữ liệu
int bcd2dec(byte num) {
  return ((num/16 * 10) + (num % 16));
}
int dec2bcd(byte num) {
  return ((num/10 * 16) + (num % 10));
}
void printDigits(int digits){
  if(digits < 10) lcd.print('0');
  lcd.print(digits);
}
// Đọc giá trị ngày giờ từ DS1307 ra các biến
void readDS1307() {
  Wire.beginTransmission(rtcDS1307);
  Wire.write((byte)0x00); // Đưa con trỏ thanh ghi về thanh ghi giây (byte đầu trong chuỗi bit ghi dữ liệu kiểu I2C đến DS1307 là địa chỉ thanh ghi đầu tiên được đọc)
  Wire.endTransmission();
  Wire.requestFrom(rtcDS1307, 7); // Đọc 7 byte từ rtc

  second = bcd2dec(Wire.read() & 0x7f); // Bỏ bit 7 của thanh ghi second (bit trạng thái hoạt động của DS1307)
  minute = bcd2dec(Wire.read());
  hour   = bcd2dec(Wire.read() & 0x3f); // chế độ 24h.
  Wire.read(); // Bỏ qua byte của thanh ghi cho thứ
  day    = bcd2dec(Wire.read());
  month  = bcd2dec(Wire.read());
  year   = bcd2dec(Wire.read());
  year += 2000; 
}
// Cài lại ngày giờ bằng cách sửa các giá trị của các thanh ghi ngày giờ của DS1307
void setTimeRTC(int _second,int _minute,int _hour) {
  Wire.beginTransmission(rtcDS1307);
  Wire.write(byte(0x00)); // đặt con trỏ thanh ghi về thanh ghi giây (0x00) của DS1307
  Wire.write(dec2bcd(_second));
  Wire.write(dec2bcd(_minute)); 
  Wire.write(dec2bcd(_hour));
  Wire.endTransmission();
}
void setDateRTC(int _day,int _month,int _year) {
  Wire.beginTransmission(rtcDS1307);
  Wire.write(byte(0x04)); // // đặt con trỏ thanh ghi về thanh ghi ngày của DS1307
  Wire.write(dec2bcd(_day)); 
  Wire.write(dec2bcd(_month));
  Wire.write(dec2bcd(_year - 2000));
  Wire.endTransmission();
}
// Hiển thị lên LCD
void displayLCDCurrentTime() {
  lcd.setCursor(0,0);
  lcd.print("Date: ");
  printDigits(day);
  lcd.print('/');
  printDigits(month);
  lcd.print('/');
  lcd.print(year);
  lcd.setCursor(0,1);
  lcd.print("Time: ");
  printDigits(hour);
  lcd.print(':');
  printDigits(minute);
  lcd.print(':');
  printDigits(second);
}
void displayLCDDateAdjusted() {
  lcd.setCursor(2,0);
  lcd.print("Adjust date:");
  lcd.setCursor(3,1);
  printDigits(day);
  lcd.print('/');
  printDigits(month);
  lcd.print('/');
  lcd.print(year);
}
void displayLCDTimeAdjusted() {
  lcd.setCursor(2,0);
  lcd.print("Adjust time:");
  lcd.setCursor(4,1);
  printDigits(hour);
  lcd.print(':');
  printDigits(minute);
  lcd.print(':');
  printDigits(second);
}
void displayLCDAlarmSetting() {
  lcd.setCursor(1,0);
  lcd.print("Setting alarm:");
  lcd.setCursor(1,1);
  if(temp_state == 0) lcd.print("OFF");
  else lcd.print("ON ");
  lcd.setCursor(10,1);
  printDigits(temp_hour);
  lcd.print(':');
  printDigits(temp_minute);
}
void setAlarm() {
  alarm_hour = temp_hour;
  alarm_minute = temp_minute;
  alarm_state = temp_state;
}

