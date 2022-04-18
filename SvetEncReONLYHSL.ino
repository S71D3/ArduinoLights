#include <GyverEncoder.h>

#include <Adafruit_NeoPixel.h>

#include <Adafruit_NeoPixel.h>
#include "GyverEncoder.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

#define ROUTNUM 3 //кол-во режимов
#define PIN            2
#define NUMPIXELS      46 //+1 от реального
#define pin_CLK 3
#define pin_DT  4
#define pin_Btn 5
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Encoder ENCO(pin_CLK, pin_DT, pin_Btn);

int delayval = 5; // delay for half a second

int globalsetup = 0;
int nowsetup = 0; //нынешний режим (0 - дефолт резет, 1-3 - рабочие)

double step = static_cast<double>(1)/255;
//режим полной настройки
uint8_t r = 255;
uint8_t g = 0;
uint8_t b = 0;
double hsl[3];

//режим hue
double hue = 0.5;
uint8_t hr = 0;
uint8_t hg = 255;
uint8_t hb = 0;

//режим kelvin
int kelvins[12] = {600, 800, 1000, 1300, 2500, 3900, 5000, 5600, 6000, 7500, 12000, 18000};
int kelvinnum = 6;
uint8_t kr = 0;
uint8_t kg = 0;
uint8_t kb = 255;

double threeway_max(double a, double b, double c)
{
  return max(a, max(b, c));
}

double threeway_min(double a, double b, double c)
{
  return min(a, min(b, c));
}

double hue2rgb(double p, double q, double t)
{
  if (t < 0) t += 1;
  if (t > 1) t -= 1;
  if (t < 1 / 6.0) return p + (q - p) * 6 * t;
  if (t < 1 / 2.0) return q;
  if (t < 2 / 3.0) return p + (q - p) * (2 / 3.0 - t) * 6;
  return p;
}

void TemperatureToRgb(int kelvin, uint8_t& red, uint8_t& green, uint8_t& blue)
{
  auto temp = kelvin / 100;

  if (temp <= 66)
  {
    red = 255;
    green = 99.4708025861 * log(temp) - 161.1195681661;

    if (temp <= 19)
    {
      blue = 0;
    }
    else
    {
      blue = 138.5177312231 * log(temp - 10) - 305.0447927307;
    }
  }
  else
  {
    red = 329.698727446 * pow(temp - 60, -0.1332047592);
    green = 288.1221695283 * pow(temp - 60, -0.0755148492);
    blue = 255;
  }
}

void RgbToHsl(uint8_t red, uint8_t green, uint8_t blue, double& hue, double& saturation, double& lighting)
{
  auto rd = static_cast<double>(red) / 255;
  auto gd = static_cast<double>(green) / 255;
  auto bd = static_cast<double>(blue) / 255;

  auto max = threeway_max(rd, gd, bd);
  auto min = threeway_min(rd, gd, bd);

  double h, s, l = (max + min) / 2;

  if (max == min)
  {
    h = s = 0; // achromatic
  }
  else
  {
    auto d = max - min;
    s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
    if (max == rd)
    {
      h = (gd - bd) / d + (gd < bd ? 6 : 0);
    }
    else if (max == gd)
    {
      h = (bd - rd) / d + 2;
    }
    else if (max == bd)
    {
      h = (rd - gd) / d + 4;
    }
    h /= 6;
  }

  hue = h;
  saturation = s;
  lighting = l;
}

void HslToRgb(double hue, double saturation, double lightness, uint8_t& red, uint8_t& green, uint8_t& blue)
{
  double r, g, b;

  if (saturation == 0)
  {
    r = g = b = lightness; // achromatic
  }
  else
  {
    auto q = lightness < 0.5 ? lightness * (1 + saturation) : lightness + saturation - lightness * saturation;
    auto p = 2 * lightness - q;
    r = hue2rgb(p, q, hue + 1 / 3.0);
    g = hue2rgb(p, q, hue);
    b = hue2rgb(p, q, hue - 1 / 3.0);
  }

  red = static_cast<uint8_t>(r * 255);
  green = static_cast<uint8_t>(g * 255);
  blue = static_cast<uint8_t>(b * 255);
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
  
  pixels.begin(); // This initializes the NeoPixel library.
  nowsetup = 1;
  globalsetup = 1;
  Beauty();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, r, g, b);
  }
  RgbToHsl(r, g, b, hsl[0], hsl[1], hsl[2]);
  pixels.show();
  ENCO.tick();
  Serial.begin(9600);
}

void loop() {

  switch (globalsetup)
  {
    case 1:
      HslToRgb(hsl[0], hsl[1], hsl[2], r, g, b);
      
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, r, g, b);
      }
      pixels.show();
    
      if (ENCO.isRight())//по часовой
      {
        if (nowsetup == 1)
        {
          hsl[nowsetup-1] += step;
            if (hsl[nowsetup-1] >= 1) hsl[nowsetup-1] = 0;
        }
        else
        {
          hsl[nowsetup-1] += step*2;
            if (hsl[nowsetup-1] >= 1)
            {
              hsl[nowsetup-1] = 1;
              Beauty();
            }
        }
      }
      else if (ENCO.isLeft())
      {
        if (nowsetup == 1)
        {
          hsl[nowsetup-1] -= step;
            if (hsl[nowsetup-1] < 0) hsl[nowsetup-1] = 1;
        }
        else
        {
          hsl[nowsetup-1] -= step*2;
            if (hsl[nowsetup-1] < 0)
            {
              hsl[nowsetup-1] = 0;
              Beauty();
            }
        }
      }
  
      else if (ENCO.isPress())
      {
        Beauty();
        nowsetup++;
        if (nowsetup > ROUTNUM)
          nowsetup = 1;
      }
     break;
    case 2:
      HslToRgb(hue, 1, 0.5, hr, hg, hb);
      
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, hr, hg, hb);
      }
      pixels.show();
    
      if (ENCO.isRight())//по часовой
      {
          hue += step;
          if (hue >= 1)
           hue = 0;
      }
      else if (ENCO.isLeft())
      {
          hue -= step;
          if (hue < 0)
           hue = 1;
      }
      Serial.println(hue);
      break;
    case 3:
      TemperatureToRgb(kelvins[kelvinnum], kr, kg, kb);
      
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, kr, kg, kb);
      }
      pixels.show();
    
      if (ENCO.isRight())//по часовой
      {
          kelvinnum ++;
          if (kelvinnum >= 11)
           kelvinnum = 0;
      }
      else if (ENCO.isLeft())
      {
          kelvinnum --;
          if (kelvinnum < 0)
           kelvinnum = 11;
      }
      break;
    default:
      break;
  }
  
  if (ENCO.isHolded())
  {
    globalsetup+=1;
    if (globalsetup > 3)
      globalsetup = 1;
    Beauty();
  }
  
  ENCO.tick();
}
