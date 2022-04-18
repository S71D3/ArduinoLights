#include <GyverEncoder.h>

#include <Adafruit_NeoPixel.h>
// #include <rp2040_pio.h>

#include <Kelvin2RGB.h>
#include <Adafruit_NeoPixel.h>
#include "GyverEncoder.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

#define ROUTNUM 5 //кол-во режимов
#define PIN            2
#define NUMPIXELS      46 //+1 от реального
#define pin_CLK 3
#define pin_DT  4
#define pin_Btn 5
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Encoder ENCO(pin_CLK, pin_DT, pin_Btn);

int delayval = 5; // delay for half a second

int nowsetup = 0; //нынешний режим (0 - дефолт резет, 1-4 - рабочие)
int hue, saturation, lightness;
//int nowcolor[] = {0, 0, 0};
uint32_t nowcolor = (0 | 0 | 0);
int nowwhite[] = {0, 0, 0};
int kel = 5600; //стандартная теплота для режима кельвинов, нейтральный белый свет
Kelvin2RGB nkel(5600, 75);

Kelvin2RGB getKelvin2rgb(int temp, int light)
{
  Kelvin2RGB tempor(temp, light);
  return tempor;
}

uint32_t getPixelColorHsv(
  uint16_t h, uint8_t s, uint8_t v) {

  uint8_t r, g, b;

  if (!s) {
    // Monochromatic, all components are V
    r = g = b = v;
  } else {
    uint8_t sextant = h >> 8;
    if (sextant > 5)
      sextant = 5;  // Limit hue sextants to defined space

    g = v;    // Top level

    // Perform actual calculations
    uint16_t ww;        // Intermediate result
    ww = v * (uint8_t)(~s);
    ww += 1;            // Error correction
    ww += ww >> 8;      // Error correction
    b = ww >> 8;

    uint8_t h_fraction = h & 0xff;  // Position within sextant
    uint32_t d;      // Intermediate result

    if (!(sextant & 1)) {
      // r = ...slope_up...
      // --> r = (v * ((255 << 8) - s * (256 - h)) + error_corr1 + error_corr2) / 65536
      d = v * (uint32_t)(0xff00 - (uint16_t)(s * (256 - h_fraction)));
      d += d >> 8;  // Error correction
      d += v;       // Error correction
      r = d >> 16;
    } else {
      // r = ...slope_down...
      // --> r = (v * ((255 << 8) - s * h) + error_corr1 + error_corr2) / 65536
      d = v * (uint32_t)(0xff00 - (uint16_t)(s * h_fraction));
      d += d >> 8;  // Error correction
      d += v;       // Error correction
      r = d >> 16;
    }

    // Swap RGB values according to sextant. This is done in reverse order with
    // respect to the original because the swaps are done after the
    // assignments.
    if (!(sextant & 6)) {
      if (!(sextant & 1)) {
        uint8_t tmp = r;
        r = g;
        g = tmp;
      }
    } else {
      if (sextant & 1) {
        uint8_t tmp = r;
        r = g;
        g = tmp;
      }
    }
    if (sextant & 4) {
      uint8_t tmp = g;
      g = b;
      b = tmp;
    }
    if (sextant & 2) {
      uint8_t tmp = r;
      r = b;
      b = tmp;
    }
  }
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void Beauty()
{
  pixels.setPixelColor(23, 0, 0, 0);
  pixels.setPixelColor(12, 120, 120, 120);
  pixels.setPixelColor(14, 120, 120, 120);
  pixels.setPixelColor(30, 120, 120, 120);
  pixels.setPixelColor(32, 120, 120, 120);
  pixels.show();
  delay(100);
  pixels.setPixelColor(12, 0, 0, 0);
  pixels.setPixelColor(14, 0, 0, 0);
  pixels.setPixelColor(30, 0, 0, 0);
  pixels.setPixelColor(32, 0, 0, 0);
  pixels.setPixelColor(1, 120, 120, 120);
  pixels.setPixelColor(7, 120, 120, 120);
  pixels.setPixelColor(37, 120, 120, 120);
  pixels.setPixelColor(43, 120, 120, 120);
  pixels.show();
  delay(100);
}

void setup() {
  pinMode(pin_DT,  INPUT);
  pinMode(pin_CLK, INPUT);
  pinMode(pin_Btn, INPUT_PULLUP); // Кнопка не подтянута к +5 поэтому задействуем внутренний pull-up резистор

  hue = 0; //цвет по HSV, от 0 до 360 (наверное, на самом деле поебать, стоит фикс внутри)
  saturation = 255; //насыщенность 0-255
  lightness = 100; //яркость, 0-100
  kel = 5600; //стандартная теплота для режима кельвинов, нейтральный белый свет
  
  nowcolor = getPixelColorHsv(hue, saturation, lightness); //дефолтный цвет
  
  pixels.begin(); // This initializes the NeoPixel library.
  nowsetup = 1;
  Beauty();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, getPixelColorHsv(0, 255, 100));
  }
  pixels.show();
  ENCO.tick();
  Serial.begin(9600);
}

void loop() {
  //присрать три режима: изменение цвета, изменение яркости //upd. частично присрано
  bool beeperFlag = false;
  
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, nowcolor);
  }
  pixels.show();

  if (ENCO.isRight())//по часовой
  {
    switch (nowsetup) {
      case 1:
        hue += (1536 / 256); //256 цветов
        if (hue >= 1536) hue = 1;
        nowcolor = getPixelColorHsv(hue, saturation, lightness);
        // изменение цвета
        break;
      case 2:
        lightness += 5;
        if (lightness > 100)
          lightness = 100;
        nowcolor = getPixelColorHsv(hue, saturation, lightness);
        // изменение яркости
        break;
      case 5: //режим кельвинов
        if (kel<20000)
          kel+=1000;
        Serial.println(kel);
        nkel = getKelvin2rgb(kel, 75);
        //nowcolor = ((uint32_t)nkel.Red << 16 | (uint32_t)nkel.Green << 8 | nkel.Blue);
        nowcolor = ((uint32_t)getKelvin2rgb(kel, 75).Red << 16 | (uint32_t)getKelvin2rgb(kel, 75).Green << 8 | getKelvin2rgb(kel, 75).Blue);
        break;
      case 3: //режим радуги
      case 4: //режим белого цвета
      default:
        nowcolor = getPixelColorHsv(hue, saturation, lightness);
        break;
    }

  }
  else if (ENCO.isLeft())
  {

    switch (nowsetup) {
      case 1:
        hue -= (1536 / 256); //256 цветов
        if (hue < 0) hue = 1536 - 1;
        nowcolor = getPixelColorHsv(hue, saturation, lightness);
        // изменение цвета
        break;
      case 2:
        lightness -= 5;
        if (lightness < 5)
          lightness = 5;
        nowcolor = getPixelColorHsv(hue, saturation, lightness);
        // изменение яркости
        break;
      case 5: //режим кельвинов
        if (kel>2000)
          kel-=1000;
        Serial.println(kel);
        nkel = getKelvin2rgb(kel, 75);
        nowcolor = ((uint32_t)nkel.Red << 16 | (uint32_t)nkel.Green << 8 | nkel.Blue);
        break;
      case 3: //режим радуги
      case 4: //режим белого цвета
      default:
        nowcolor = getPixelColorHsv(hue, saturation, lightness);
        break;
    }

  }


  else if (ENCO.isPress())
  {
    Beauty();
    nowsetup++;
    if (nowsetup > ROUTNUM)
      nowsetup = 1;
  }

  switch (nowsetup) {
    case 3:
      hue += (1536 / 750); //256 цветов
        if (hue >= 1536) hue = 1;
      nowcolor = getPixelColorHsv(hue, saturation, lightness);
      break;
    case 4:
      hue = 0;
      saturation = 0;
      lightness = 100;
      nowcolor = getPixelColorHsv(hue, saturation, lightness);
      break;
    case 5:
      nkel = getKelvin2rgb(5600, 75);
      nowcolor = ((uint32_t)nkel.Red << 16 | (uint32_t)nkel.Green << 8 | nkel.Blue);
      break;
    default:
      saturation = 255;
      nowcolor = getPixelColorHsv(hue, saturation, lightness);
      break;
  }
  
  ENCO.tick();
}
