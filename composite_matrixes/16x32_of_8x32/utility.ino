// служебные функции

// залить все
void fillAll(CRGB color)
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
    leds[i] = color;
}

// функция отрисовки точки по координатам X Y
#if (WIDTH > 127) || (HEIGHT > 127)
void drawPixelXY(int16_t x, int16_t y, CRGB color)
#else
void drawPixelXY(int8_t x, int8_t y, CRGB color)
#endif
{
  if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1)) return;
  //uint32_t thisPixel = XY((uint8_t)x, (uint8_t)y) * SEGMENTS;
  //for (uint8_t i = 0; i < SEGMENTS; i++)
  //{
  //  leds[thisPixel + i] = color;
  //}
  leds[XY(x, y)] = color;
}

// функция получения цвета пикселя по его номеру
//uint32_t getPixColor(uint32_t thisSegm)
//{
//  uint32_t thisPixel = thisSegm * SEGMENTS;
//  if (thisPixel > NUM_LEDS - 1) return 0;
//  return (((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8 ) | (uint32_t)leds[thisPixel].b); // а почему не просто return (leds[thisPixel])?
//}
uint32_t getPixColor(uint16_t thisPixel)
{
  if (thisPixel >= NUM_LEDS) return 0;
  return (((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8 ) | (uint32_t)leds[thisPixel].b);
}

// функция получения цвета пикселя в матрице по его координатам
uint32_t getPixColorXY(uint8_t x, uint8_t y)
{
  return getPixColor(XY(x, y));
}


// получить номер пикселя в ленте по координатам
// библиотека FastLED тоже использует эту функцию
// для матрицы размером 16х32, собранной из двух размером 8х32
// https://editor.soulmatelights.com/gallery/1187-xy-16x32

uint16_t XY(uint8_t x, uint8_t y)
{
 if (x>=8) // 1st matrix 8x32
   { 
     x = x - 8; // shift of 0:0 coordinate
     if (y % 2 == 0) // even rows
     {
       return  y * 8 + x;
     }
     else // odd rows
     {
       return  y * 8 + 8 - x - 1;
     }
   }
  else // 2nd matrix 8x32
   {
    if (y % 2 == 0) // even rows
     {
       return  (8*32) + (32 - y - 1) * 8 + x;
     }
     else // odd rows
     {
       return  (8*32) + (32 - y - 1) * 8 + 8 - x - 1;
     }
   }
}

// было оставлено для совместимости с эффектами из старых прошивок
//uint16_t getPixelNumber(uint8_t x, uint8_t y) {return XY(x, y);}


// восстановление настроек эффектов на настройки по умолчанию
void restoreSettings()
{
//  if (defaultSettingsCOUNT == MODE_AMOUNT)          // если пользователь не накосячил с количеством строк в массиве настроек в Constants.h, используем их
    for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
      modes[i].Brightness = pgm_read_byte(&defaultSettings[i][0]);
      modes[i].Speed      = pgm_read_byte(&defaultSettings[i][1]);
      modes[i].Scale      = pgm_read_byte(&defaultSettings[i][2]);
    }
//  else                                              // иначе берём какие-то абстрактные
//    for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
//      modes[i].Brightness = 50U;
//      modes[i].Speed      = 225U;
//      modes[i].Scale      = 40U;
//    }  
}

// неточный, зато более быстрый квадратный корень
float sqrt3(const float x)
{
  union
  {
    int i;
    float x;
  } u;

  u.x = x;
  u.i = (1<<29) + (u.i >> 1) - (1<<22);
  return u.x;
}
