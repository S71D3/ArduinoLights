#include <Adafruit_NeoPixel.h>
#include "GyverEncoder.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            2
#define NUMPIXELS      46 //+1 от реального
#define pin_CLK 3
#define pin_DT  4
#define pin_Btn 5
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 5; // delay for half a second
int MAXHUE = 256 * 6;
int CNT = 128; //кол-во позиций цветов, делитель 1536, иначе погрешность при прокрутке
int MODE = 1;
int Gkelvin[] = {180, 196, 213, 223, 231, 238, 244};
int Bkelvin[] = {107, 137, 161, 184, 204, 222, 237};
int CNTK = 6;
Encoder ENCO(pin_CLK, pin_DT, pin_Btn, 1);
int counter;

uint32_t getPixelColorHsv(
  uint16_t n, uint16_t h, uint8_t s, uint8_t v) {

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
  for (int i = 0; i < 23; i++) {
    pixels.setPixelColor(i, 120, 120, 120);
    pixels.setPixelColor(NUMPIXELS - i - 2, 120, 120, 120);
    delay(20);
    pixels.show();
    pixels.setPixelColor(i, 0, 0, 0);
    pixels.setPixelColor(NUMPIXELS - i - 2, 0, 0, 0);
  }
  delay(80);
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
  delay(120);
}

void setup() {
  pinMode(pin_DT,  INPUT);
  pinMode(pin_CLK, INPUT);
  pinMode(pin_Btn, INPUT_PULLUP); // Кнопка не подтянута к +5 поэтому задействуем внутренний pull-up резистор
  counter = 0;
  pixels.begin(); // This initializes the NeoPixel library.
  Beauty();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, getPixelColorHsv(i, 0, 255, 100));
  }
  pixels.show();
  MODE = 1;
  ENCO.setTickMode(AUTO);
}

void loop() {
  if (ENCO.isRight())//по часовой
  {
    counter ++;
    if (counter >= CNT) counter = 1;
  }
  else if (ENCO.isLeft())
  {
    counter --;
    if (counter < 0) counter = CNT - 1;
  }
  else if (ENCO.isPress())
  {
    Beauty();
    for (int i = 0; i < NUMPIXELS; i++)
      pixels.setPixelColor(i, getPixelColorHsv(i, 255, 225, 188));
    pixels.show();

    while (!ENCO.isPress())
    {
      for (int i = 0; i < NUMPIXELS; i++)
        pixels.setPixelColor(i, 255, 223, 184);
      pixels.show();
      delay(50);
    }
    
    Beauty();
  }

  for (int i = 0; i < NUMPIXELS; i++)
    pixels.setPixelColor(i, getPixelColorHsv(i, counter * (MAXHUE / CNT), 255, 100));
  //pixels.setPixelColor(i, getPixelColorHsv(i, counter*CNTKOF, 255, 100));
  //pixels.setPixelColor(i, getPixelColorHsv(i, i * (MAXHUE / NUMPIXELS), 255, 100));
  //  delay(20);
  pixels.show();
}

