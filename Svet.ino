
// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            3

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      46

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second
const int buttonPin = 2; //пин кнопки
int nowsum = 0; //переменная аккумулятора цвета, потом поменять на свитч
int MAXHUE = 256 * 6;
int CNT = 12; //кол-во позиций цветов, делитель 1536, иначе погрешность при прокрутке

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

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, getPixelColorHsv(i, 0, 255, 100));
  }
  pixels.show();
}

void setup() {
  pinMode(buttonPin, INPUT); // инициализируем пин, подключенный к кнопке, как вход
  pixels.begin(); // This initializes the NeoPixel library.
  Beauty();
//  for (int i = 0; i < NUMPIXELS; i++) {
//    pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Moderately bright green color.
//    pixels.show(); // This sends the updated pixel color to the hardware.
//    delay(delayval); // Delay for a period of time (in milliseconds).
//  }
}

void loop() {

  if (digitalRead(buttonPin) == HIGH) //если кнопка нажата ...
  {
    nowsum++;
    delay(500);//небольшая защита от "дребезга" контактов кнопки
  }
  if (nowsum > CNT) nowsum = 0;

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, getPixelColorHsv(i, nowsum * (MAXHUE / CNT), 255, 100));
  }
  delay(20);
  pixels.show();

}
