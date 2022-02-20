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

// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
!!!!!!!!!!!!!!!!!!!!!!!!!!!   смотрите инструкцию: https://alexgyver.ru/wp-content/uploads/2018/11/scheme3.jpg
!!!!!!!!!!!!!!!!!!!!!!!!!!!   такого сочетания CONNECTION_ANGLE и STRIP_DIRECTION не бывает
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"

#endif

// получить номер пикселя в ленте по координатам
// библиотека FastLED тоже использует эту функцию
uint16_t XY(uint8_t x, uint8_t y)
{
  if (!(THIS_Y & 0x01) || MATRIX_TYPE)               // Even rows run forwards
    return (THIS_Y * _WIDTH + THIS_X);
  else                                                  
    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);  // Odd rows run backwards
}

// если у вас матрица необычной формы с зазорами/вырезами, либо просто маленькая, тогда вам придётся переписать функцию XY() под себя
// массив для переадресации можно сформировать на этом онлайн-сервисе: https://macetech.github.io/FastLED-XY-Map-Generator/
// или тут по-русски: https://firelamp.pp.ua/matrix_generator/

// ниже пример функции, когда у вас матрица 8х16, а вы хотите, чтобы эффекты рисовались, будто бы матрица 16х16 (рисуем по центру, а по бокам обрезано)
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  9  8  -  -  -  - 
//   -  -  -  -  0  1  2  3  4  5  6  7  -  -  -  -
/*
uint8_t XY (uint8_t x, uint8_t y) {
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= 16) || (y >= 16) ) {
    return (128); //(LAST_VISIBLE_LED + 1);
  }

  const uint8_t XYTable[] = {
   255, 254, 253, 252, 127, 126, 125, 124, 123, 122, 121, 120, 251, 250, 249, 248,
   240, 241, 242, 243, 112, 113, 114, 115, 116, 117, 118, 119, 244, 245, 246, 247,
   239, 238, 237, 236, 111, 110, 109, 108, 107, 106, 105, 104, 235, 234, 233, 232,
   224, 225, 226, 227,  96,  97,  98,  99, 100, 101, 102, 103, 228, 229, 230, 231,
   223, 222, 221, 220,  95,  94,  93,  92,  91,  90,  89,  88, 219, 218, 217, 216,
   208, 209, 210, 211,  80,  81,  82,  83,  84,  85,  86,  87, 212, 213, 214, 215,
   207, 206, 205, 204,  79,  78,  77,  76,  75,  74,  73,  72, 203, 202, 201, 200,
   192, 193, 194, 195,  64,  65,  66,  67,  68,  69,  70,  71, 196, 197, 198, 199,
   191, 190, 189, 188,  63,  62,  61,  60,  59,  58,  57,  56, 187, 186, 185, 184,
   176, 177, 178, 179,  48,  49,  50,  51,  52,  53,  54,  55, 180, 181, 182, 183,
   175, 174, 173, 172,  47,  46,  45,  44,  43,  42,  41,  40, 171, 170, 169, 168,
   160, 161, 162, 163,  32,  33,  34,  35,  36,  37,  38,  39, 164, 165, 166, 167,
   159, 158, 157, 156,  31,  30,  29,  28,  27,  26,  25,  24, 155, 154, 153, 152,
   144, 145, 146, 147,  16,  17,  18,  19,  20,  21,  22,  23, 148, 149, 150, 151,
   143, 142, 141, 140,  15,  14,  13,  12,  11,  10,   9,   8, 139, 138, 137, 136,
   128, 129, 130, 131,   0,   1,   2,   3,   4,   5,   6,   7, 132, 133, 134, 135
  };

  uint8_t i = (y * 16) + x;
  return XYTable[i];
}
*/

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
