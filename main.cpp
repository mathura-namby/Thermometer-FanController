#include "LCD_DISCO_F429ZI.h"
#include "TS_DISCO_F429ZI.h"
#include "mbed.h"


LCD_DISCO_F429ZI LCD;
TS_DISCO_F429ZI TS;
Ticker tick1;
Timeout delay;

AnalogIn lm35(PA_0);
PwmOut fan(PD_14);

float temp, thresh;
bool curr_temp = true;
float cycle = 0;
bool fan_tick = true;

void CurrTemp_tick() { curr_temp = true; }

void Plus_Minus() {
  LCD.SetTextColor(LCD_COLOR_BLUE);
  LCD.FillRect(45, 190, 10, 60);
  LCD.FillRect(20, 215, 60, 10);
  LCD.FillRect(160, 215, 60, 10);
}

void Fan_Tick() { fan_tick = true; }

int main() {
  lm35.set_reference_voltage(3);
  thresh = round((lm35.read_voltage() * 100 / 3) + 1);
  fan.period_us(255);
  fan.pulsewidth(0);
  TS_StateTypeDef tsState;
  uint16_t tsX, tsY;

  while (true) {
    Plus_Minus();
    TS.GetState(&tsState);
    if (tsState.TouchDetected) {
      tsX = tsState.X;
      tsY = 320 - tsState.Y;
      if ((tsX >= 20 && tsX <= 80) && (tsY >= 190 && tsY <= 270)) {
        LCD.SetTextColor(LCD_COLOR_RED);
        LCD.FillRect(45, 190, 10, 60);
        LCD.FillRect(20, 215, 60, 10);
      }
      if ((tsX >= 160 && tsX <= 220) && (tsY >= 190 && tsY <= 270)) {
        LCD.SetTextColor(LCD_COLOR_RED);
        LCD.FillRect(160, 215, 60, 10);
      }
    }

    if (!tsState.TouchDetected && (tsX >= 20 && tsX <= 80) &&
        (tsY >= 190 && tsY <= 270)) {
      thresh += 0.5;
      tsX = 0;
      tsY = 0;
    } else if (!tsState.TouchDetected && (tsX >= 160 && tsX <= 220) &&
               (tsY >= 190 && tsY <= 270)) {
      thresh -= 0.5;
      tsX = 0;
      tsY = 0;
    }

    if (curr_temp == true) {
      tick1.detach();
      temp = lm35.read_voltage() * 100 / 3;

      LCD.SetFont(&Font20);
      LCD.SetTextColor(LCD_COLOR_BLACK);
      uint8_t text[100];
      sprintf((char *)text, "Sensor: %.1f C", temp);
      LCD.DisplayStringAt(0, 40, (uint8_t *)&text, LEFT_MODE);
      sprintf((char *)text, "Thresh: %.1f C", thresh);
      LCD.DisplayStringAt(0, 80, (uint8_t *)&text, LEFT_MODE);
      curr_temp = false;
      tick1.attach(&CurrTemp_tick, 1000ms);
    }

    if (temp > thresh) {
        if (cycle < 255 && fan_tick == true) {
            delay.detach();
            cycle ++;
            fan.pulsewidth_us(cycle);
            fan_tick = false;
            delay.attach(&Fan_Tick,10ms);
        }
    }
    else if (temp <= thresh) {
        cycle = 0;
        fan.pulsewidth_us(cycle);
        fan_tick = true;
    }
  }
}