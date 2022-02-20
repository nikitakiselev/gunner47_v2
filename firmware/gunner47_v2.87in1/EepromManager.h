#pragma once

/*
 * Схема использования EEPROM памяти:
 * Используются адреса:
 * Начало    Длина    Описание
 *  ***************  массив alarm (будильник рассвет)
 * 0         1        будильник пн вкл/выкл
 * 1-2       2        будильник пн время в минутах от начала суток (0 - 1440), 2 байта
 * 3         1        будильник вт вкл/выкл
 * 4-5       2        будильник вт время в минутах от начала суток (0 - 1440), 2 байта
 * 6         1        будильник ср вкл/выкл
 * 7-8       2        будильник ср время в минутах от начала суток (0 - 1440), 2 байта
 * 9         1        будильник чт вкл/выкл
 * 10-11     2        будильник чт время в минутах от начала суток (0 - 1440), 2 байта
 * 12        1        будильник пт вкл/выкл
 * 13-14     2        будильник пт время в минутах от начала суток (0 - 1440), 2 байта
 * 15        1        будильник сб вкл/выкл
 * 16-17     2        будильник сб время в минутах от начала суток (0 - 1440), 2 байта
 * 18        1        будильник вс вкл/выкл
 * 19-20     2        будильник вс время в минутах от начала суток (0 - 1440), 2 байта
 * 
 * ***************** прочие настройки
 * 21        1        признак "кнопка разблокирована"
 * 22        1        режим работы модуля ESP (точка доступа/WiFi клиент)
 * 23        1        состояние лампы (вкл/выкл)
 * 24        1        признак первого запуска (определяет необходимость первоначальной записи всех хранимых настроек)
 * 25        1        время до "рассвета" (dawnMode)
 * 26        1        номер текущего эффекта лампы (currentMode)
 * 27        1        ВКЛ/ВЫКЛ режима случайного выбора настроек эффектов в режиме Цикл
 
 * ***************** массив modes (эффекты)
 * 50-52     3        режим №1:  яркость, скорость, масштаб (по одному байту)
 * 53-55     3        режим №2:  яркость, скорость, масштаб (по одному байту)
 * 56-58     3        режим №3:  яркость, скорость, масштаб (по одному байту)
 * 59-61     3        режим №4:  яркость, скорость, масштаб (по одному байту)
 * 62-64     3        режим №5:  яркость, скорость, масштаб (по одному байту)
 * 65-67     3        режим №6:  яркость, скорость, масштаб (по одному байту)
 * 68-70     3        режим №7:  яркость, скорость, масштаб (по одному байту)
 * 71-73     3        режим №8:  яркость, скорость, масштаб (по одному байту)
 * 74-76     3        режим №9:  яркость, скорость, масштаб (по одному байту)
 * 77-79     3        режим №10: яркость, скорость, масштаб (по одному байту)
 * 80-82     3        режим №11: яркость, скорость, масштаб (по одному байту)
 * 83-85     3        режим №12: яркость, скорость, масштаб (по одному байту)
 * 86-88     3        режим №13: яркость, скорость, масштаб (по одному байту)
 * 89-91     3        режим №14: яркость, скорость, масштаб (по одному байту)
 * 92-94     3        режим №15: яркость, скорость, масштаб (по одному байту)
 * 95-97     3        режим №16: яркость, скорость, масштаб (по одному байту)
 * 98-100    3        режим №17: яркость, скорость, масштаб (по одному байту)
 * 101-103   3        режим №18: яркость, скорость, масштаб (по одному байту)
 * 104-106   3        режим №19: яркость, скорость, масштаб (по одному байту)
 * 107-109   3        режим №20: яркость, скорость, масштаб (по одному байту)
 * 110-112   3        режим №21: яркость, скорость, масштаб (по одному байту)
 * 113-115   3        режим №22: яркость, скорость, масштаб (по одному байту)
 * 116-118   3        режим №23: яркость, скорость, масштаб (по одному байту)
 * 119-121   3        режим №24: яркость, скорость, масштаб (по одному байту)
 * 122-124   3        режим №25: яркость, скорость, масштаб (по одному байту)
 * другие эффекты, если вы будете увиличивать их количество, будут сами автоматически занимать дальше по 3 байта, сдвигая вниз массив "избранных эффектов"
 * от посл.эффект+1   настройки режима избранных эффектов (вкл/выкл - 1 байт; интервал - 2 байта; разброс - 2 байта; инициализировать вкл/выкл - 1 байт; вкл/выкл каждого эффекта - 26 (MODE_AMOUNT) байт)
 * количество эффектов, которое можно добавить так никто и не посчитал...
*/

#include <EEPROM.h>
#include "Types.h"
#define EEPROM_ESP_BUTTON_ENABLED_ADDRESS    (21U)         // адрес в EEPROM памяти для записи признака разблокированной кнопки
#define EEPROM_ESP_MODE                      (22U)         // адрес в EEPROM памяти для записи режима работы модуля ESP (точка доступа/WiFi клиент)
#define EEPROM_LAMP_ON_ADDRESS               (23U)         // адрес в EEPROM памяти для записи состояния лампы (вкл/выкл)
#define EEPROM_FIRST_RUN_ADDRESS             (24U)         // адрес в EEPROM памяти для записи признака первого запуска (определяет необходимость первоначальной записи всех хранимых настроек)
#define EEPROM_DAWN_MODE_ADDRESS             (25U)         // адрес в EEPROM памяти для записи времени до "рассвета"
#define EEPROM_CURRENT_MODE_ADDRESS          (26U)         // адрес в EEPROM памяти для записи номера текущего эффекта лампы

#ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
#define EEPROM_RANDOM_ON_ADDRESS             (27U)         // адрес в EEPROM памяти для записи признака Вкл/Выкл случайного выбора настроек эффектов в режиме Цикл
#endif  //RANDOM_SETTINGS_IN_CYCLE_MODE

#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
#define EEPROM_BUTTON_SLEEP_TIMER            (28U)         // адрес в EEPROM памяти для записи значения таймера сна, активируемого по кнопке
#endif  //#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)

#define EEPROM_ALARM_STRUCT_SIZE             (3U)           // 1 байт - вкл/выкл; 2 байта - время от начала суток в минутах (0 - 1440)
#define EEPROM_ALARM_START_ADDRESS           (0U)           // начальный адрес в EEPROM памяти для записи настроек будильников

#define EEPROM_MODE_STRUCT_SIZE              (3U)           // 1 байт - яркость; 1 байт - скорость; 1 байт - масштаб

                                                            // начальный адрес в EEPROM памяти для записи настроек эффектов (яркость, скорость, масштаб)
#define EEPROM_MODES_START_ADDRESS           (50U)
                                                            // начальный адрес в EEPROM памяти для записи настроек режима избранных эффектов
#define EEPROM_FAVORITES_START_ADDRESS       (EEPROM_MODES_START_ADDRESS+MODE_AMOUNT*EEPROM_MODE_STRUCT_SIZE+1)       
                                                            // общий размер используемой EEPROM памяти (сумма всех хранимых настроек + 1 байт) 
#define EEPROM_TOTAL_BYTES_USED              (EEPROM_FAVORITES_START_ADDRESS+MODE_AMOUNT+7)       

#define EEPROM_FIRST_RUN_MARK                (24U)          // число-метка, если ещё не записно в EEPROM_FIRST_RUN_ADDRESS, значит нужно проинициализировать EEPROM и записать все первоначальные настройки
#define EEPROM_WRITE_DELAY                   (30000UL)      // отсрочка записи в EEPROM после последнего изменения хранимых настроек, позволяет уменьшить количество операций записи в EEPROM

class EepromManager
{
  public:
    static void InitEepromSettings(ModeType modes[], AlarmType alarms[], uint8_t* espMode, bool* onFlag, uint8_t* dawnMode, uint8_t* currentMode, bool* buttonEnabled
      #ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
      , uint8_t* random_on
      #endif //ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
      #if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
      , uint8_t* button_sleep_time
      #endif //#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
      , void (*readFavoritesSettings)(), void (*saveFavoritesSettings)(), void (*restoreDefaultSettings)())
    {
      EEPROM.begin(EEPROM_TOTAL_BYTES_USED);
      delay(50);

      // записываем в EEPROM начальное состояние настроек, если их там ещё нет
      if (EEPROM.read(EEPROM_FIRST_RUN_ADDRESS) != EEPROM_FIRST_RUN_MARK)
      {
        restoreDefaultSettings(); // а почему бы нам не восстановить настройки по умолчанию в этом месте?

        EEPROM.write(EEPROM_FIRST_RUN_ADDRESS, EEPROM_FIRST_RUN_MARK);
        //EEPROM.commit();

        for (uint8_t i = 0; i < MODE_AMOUNT; i++)
        {
          EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
          //EEPROM.commit();
        }

        for (uint8_t i = 0; i < 7; i++)
        {
          EEPROM.write(EEPROM_ALARM_START_ADDRESS + EEPROM_ALARM_STRUCT_SIZE * i, alarms[i].State);
          WriteUint16(EEPROM_ALARM_START_ADDRESS + EEPROM_ALARM_STRUCT_SIZE * i + 1, alarms[i].Time);
          //EEPROM.commit();
        }

        EEPROM.write(EEPROM_ESP_MODE, ESP_MODE);
        EEPROM.write(EEPROM_LAMP_ON_ADDRESS, 0);
        EEPROM.write(EEPROM_DAWN_MODE_ADDRESS, 0);
        EEPROM.write(EEPROM_CURRENT_MODE_ADDRESS, 0);
        EEPROM.write(EEPROM_ESP_BUTTON_ENABLED_ADDRESS, 1);
        #ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
          EEPROM.write(EEPROM_RANDOM_ON_ADDRESS, RANDOM_SETTINGS_IN_CYCLE_MODE);
        #endif  //RANDOM_SETTINGS_IN_CYCLE_MODE        
        #if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
          EEPROM.write(EEPROM_BUTTON_SLEEP_TIMER, 1U);
        #endif  //#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)

        saveFavoritesSettings(); // там уже есть EEPROM.commit();
    
        //EEPROM.commit();
      }

      // инициализируем настройки лампы значениями из EEPROM
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        EEPROM.get(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
      }

      for (uint8_t i = 0; i < 7; i++)
      {
        alarms[i].State = EEPROM.read(EEPROM_ALARM_START_ADDRESS + EEPROM_ALARM_STRUCT_SIZE * i);
        alarms[i].Time = ReadInt16(EEPROM_ALARM_START_ADDRESS + EEPROM_ALARM_STRUCT_SIZE * i + 1);
      }

      readFavoritesSettings();

      *espMode = (uint8_t)EEPROM.read(EEPROM_ESP_MODE);
#ifdef DONT_TURN_ON_AFTER_SHUTDOWN
      *onFlag = false;
#else
      *onFlag = (bool)EEPROM.read(EEPROM_LAMP_ON_ADDRESS);
#endif
      *dawnMode = EEPROM.read(EEPROM_DAWN_MODE_ADDRESS);
      *currentMode = EEPROM.read(EEPROM_CURRENT_MODE_ADDRESS);
      if (*buttonEnabled) *buttonEnabled = EEPROM.read(EEPROM_ESP_BUTTON_ENABLED_ADDRESS);
#ifdef RANDOM_SETTINGS_IN_CYCLE_MODE      
      *random_on=EEPROM.read(EEPROM_RANDOM_ON_ADDRESS);
#endif //#ifdef RANDOM_SETTINGS_IN_CYCLE_MODE      
#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
      *button_sleep_time=EEPROM.read(EEPROM_BUTTON_SLEEP_TIMER);
#endif //#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
    }

#ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
    static void Save_random_on( uint8_t* random_on)
    {
      EEPROM.write(EEPROM_RANDOM_ON_ADDRESS, *random_on);
      EEPROM.commit();
    } 
#endif  //RANDOM_SETTINGS_IN_CYCLE_MODE 

#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
    static void Save_button_sleep_time( uint8_t* button_sleep_time)
    {
      EEPROM.write(EEPROM_BUTTON_SLEEP_TIMER, *button_sleep_time);
      EEPROM.commit();
    } 
#endif  //#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)

    static void SaveModesSettings(uint8_t* currentMode, ModeType modes[])
    {
      EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * (*currentMode), modes[*currentMode]);
      EEPROM.commit();
    }
    
    static void HandleEepromTick(bool* settChanged, uint32_t* eepromTimeout, bool* onFlag, uint8_t* currentMode, ModeType modes[], void (*saveFavoritesSettings)())
    {
      if (*settChanged && millis() - *eepromTimeout > EEPROM_WRITE_DELAY)
      {
        *settChanged = false;
        *eepromTimeout = millis();
        //SaveOnFlag(onFlag);
        #ifndef DONT_TURN_ON_AFTER_SHUTDOWN
        EEPROM.write(EEPROM_LAMP_ON_ADDRESS, *onFlag);
        #endif        
        //SaveModesSettings(currentMode, modes);
        for (uint8_t i = 0; i < MODE_AMOUNT; i++)
          EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
        //if (EEPROM.read(EEPROM_CURRENT_MODE_ADDRESS) != *currentMode)
          EEPROM.write(EEPROM_CURRENT_MODE_ADDRESS, *currentMode);
        saveFavoritesSettings(); // там уже есть EEPROM.commit();
        //EEPROM.commit();
      }
    }
    
    static void SaveAlarmsSettings(uint8_t* alarmNumber, AlarmType alarms[])
    {
      EEPROM.write(EEPROM_ALARM_START_ADDRESS + EEPROM_ALARM_STRUCT_SIZE * (*alarmNumber), alarms[*alarmNumber].State);
      WriteUint16(EEPROM_ALARM_START_ADDRESS + EEPROM_ALARM_STRUCT_SIZE * (*alarmNumber) + 1, alarms[*alarmNumber].Time);
      EEPROM.commit();
    }

    static void SaveEspMode(uint8_t* espMode)
    {
      EEPROM.write(EEPROM_ESP_MODE, *espMode);
      EEPROM.commit();
    }

//    static void SaveOnFlag(bool* onFlag)
//    {
//#ifndef DONT_TURN_ON_AFTER_SHUTDOWN
//      EEPROM.write(EEPROM_LAMP_ON_ADDRESS, *onFlag);
//      EEPROM.commit();
//#endif
//    }

    static void SaveDawnMode(uint8_t* dawnMode)
    {
      EEPROM.write(EEPROM_DAWN_MODE_ADDRESS, *dawnMode);
      EEPROM.commit();
    }

    static void SaveButtonEnabled(bool* buttonEnabled)
    {
      EEPROM.write(EEPROM_ESP_BUTTON_ENABLED_ADDRESS, *buttonEnabled);
      EEPROM.commit();
    }

    static uint16_t ReadUint16(uint16_t address)
    {
      uint16_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      return val;
    }

    static void WriteUint16(uint16_t address, uint16_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.commit();
    }

    static int16_t ReadInt16(uint16_t address)
    {
      int16_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      return val;
    }

    static void WriteInt16(uint16_t address, int16_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.commit();      
    }

    static uint32_t ReadUint32(uint16_t address)
    {
      uint32_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      *(p + 2)  = EEPROM.read(address + 2);
      *(p + 3)  = EEPROM.read(address + 3);
      return val;
    }

    static void WriteUint32(uint16_t address, uint32_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.write(address + 2, *(p + 2));
      EEPROM.write(address + 3, *(p + 3));
      EEPROM.commit();
    }

    static int32_t ReadInt32(uint16_t address)
    {
      int32_t val;
      uint8_t* p = (uint8_t*)&val;
      *p        = EEPROM.read(address);
      *(p + 1)  = EEPROM.read(address + 1);
      *(p + 2)  = EEPROM.read(address + 2);
      *(p + 3)  = EEPROM.read(address + 3);
      return val;
    }

    static void WriteInt32(uint16_t address, int32_t val)
    {
      uint8_t* p = (uint8_t*)&val;
      EEPROM.write(address, *p);
      EEPROM.write(address + 1, *(p + 1));
      EEPROM.write(address + 2, *(p + 2));
      EEPROM.write(address + 3, *(p + 3));
      EEPROM.commit();      
    }

  private:
};
