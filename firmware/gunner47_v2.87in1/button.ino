#ifdef ESP_USE_BUTTON

bool brightDirection;
static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости/скорости/масштаба лампы кнопкой

#ifdef BUTTON_PAUSE_AFTER_TURN_ON
static bool breakButtonHolding = true;                      // флаг: кнопка была отпущена после включения лампы в режим Белый свет (удерживанием кнопки)
#endif

void buttonTick()
{
  if (!buttonEnabled)                                       // события кнопки не обрабатываются, если она заблокирована
  {
    return;
  }

  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;


  // однократное нажатие
  if (clickCount == 1U)
  {
    if (dawnFlag)
    {
      manualOff = true;
      dawnFlag = false;
      FastLED.setBrightness(modes[currentMode].Brightness);
      changePower();
    }
    else
    {
      ONflag = !ONflag;
      changePower();
    }
    settChanged = true;
    eepromTimeout = millis();
    loadingFlag = true;

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
  }


  // двухкратное нажатие
  else if (ONflag && clickCount == 2U)
  {
    #ifdef BUTTON_CHANGE_FAVORITES_MODES_ONLY
      uint8_t lastMode = currentMode;
      do {
        if (++currentMode >= MODE_AMOUNT) currentMode = 0;
      } while (FavoritesManager::FavoriteModes[currentMode] == 0 && currentMode != lastMode);
      if (currentMode == lastMode) // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
        if (++currentMode >= MODE_AMOUNT) currentMode = 0;
    #else
      if (++currentMode >= MODE_AMOUNT) currentMode = 0;
    #endif
    
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    //FastLED.clear();
    //delay(1);

    #ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
      if (random_on && FavoritesManager::FavoritesRunning)
        selectedSettings = 1U;
    #endif //RANDOM_SETTINGS_IN_CYCLE_MODE

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
  }

  // двухкратное нажатие на выключенной лампе
  #ifdef BUTTON_CAN_SET_SLEEP_TIMER
  else if (clickCount == 2U)
  {
    // мигать об успехе операции лучше до вызова changePower(), иначе сперва мелькнут кадры текущего эффекта
    showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды

    ONflag = true;
    changePower();
    settChanged = true;
    eepromTimeout = millis();
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif

    TimerManager::TimeToFire = millis() + button_sleep_time * 60UL * 1000UL;
    TimerManager::TimerRunning = true;
  }
  #endif //BUTTON_CAN_SET_SLEEP_TIMER

  // трёхкратное нажатие
  else if (ONflag && clickCount == 3U)
  {
    #ifdef BUTTON_CHANGE_FAVORITES_MODES_ONLY
      uint8_t lastMode = currentMode;
      do {
        if (--currentMode >= MODE_AMOUNT) currentMode = MODE_AMOUNT - 1;
      } while (FavoritesManager::FavoriteModes[currentMode] == 0 && currentMode != lastMode);
      if (currentMode == lastMode) // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
        if (--currentMode >= MODE_AMOUNT) currentMode = MODE_AMOUNT - 1;
    #else
      if (--currentMode >= MODE_AMOUNT) currentMode = MODE_AMOUNT - 1;
    #endif
    
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    settChanged = true;
    eepromTimeout = millis();
    //FastLED.clear();
    //delay(1);

    #ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
      if (random_on && FavoritesManager::FavoritesRunning)
        selectedSettings = 1U;
    #endif //RANDOM_SETTINGS_IN_CYCLE_MODE

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
  }


  // четырёхкратное нажатие
  else if (clickCount == 4U)
  {
    #ifdef OTA
    if (otaManager.RequestOtaUpdate())
    {
      ONflag = true;
      currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
      //FastLED.clear();
      //delay(1);
      changePower();
    }
    #endif
  }


  // пятикратное нажатие
  else if (clickCount == 5U)                                     // вывод IP на лампу
  {
    if (espMode == 1U)
    {
      loadingFlag = true;
      
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
      digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
      #endif

      while(!fillString(WiFi.localIP().toString().c_str(), CRGB::White, false)) { delay(1); ESP.wdtFeed(); }

      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
      digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
      #endif

      loadingFlag = true;
    }
  }


  // шестикратное нажатие
  else if (clickCount == 6U)                                     // вывод текущего времени бегущей строкой
  {
    printTime(thisTime, true, ONflag);
  }


  // семикратное нажатие
  else if (clickCount == 7U)  // if (ONflag &&                   // смена рабочего режима лампы: с WiFi точки доступа на WiFi клиент или наоборот
  {
    #ifdef RESET_WIFI_ON_ESP_MODE_CHANGE
      if (espMode) wifiManager.resetSettings();                             // сброс сохранённых SSID и пароля (сброс настроек подключения к роутеру)
    #endif
    espMode = (espMode == 0U) ? 1U : 0U;
    EepromManager::SaveEspMode(&espMode);

    #ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\nНовый рабочий режим: ESP_MODE = %d, %s\nРестарт...\n"),
      espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
    delay(1000);
    #endif

    showWarning(CRGB::Red, 3000U, 500U);                    // мигание красным цветом 3 секунды - смена рабочего режима лампы, перезагрузка
    ESP.restart();
  }


  // кнопка только начала удерживаться
  //if (ONflag && touch.isHolded())
  if (touch.isHolded()) // пускай для выключенной лампы удержание кнопки включает белую лампу
  {
    brightDirection = !brightDirection;
    startButtonHolding = true;
  }


  // кнопка нажата и удерживается
//  if (ONflag && touch.isStep())
if (touch.isStep())
  if (ONflag
      #ifdef BUTTON_PAUSE_AFTER_TURN_ON
      && breakButtonHolding
      #endif
  )
  {
    switch (touch.getHoldClicks())
    {
      case 0U:                                              // просто удержание (до удержания кнопки кликов не было) - изменение яркости
      {
        uint8_t delta = modes[currentMode].Brightness < 10U // определение шага изменения яркости: при яркости [1..10] шаг = 1, при [11..16] шаг = 3, при [17..255] шаг = 15
          ? 1U
          : 5U;
        modes[currentMode].Brightness =
          constrain(brightDirection
            ? modes[currentMode].Brightness + delta
            : modes[currentMode].Brightness - delta,
          1, 255);
        FastLED.setBrightness(modes[currentMode].Brightness);

        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение яркости: %d\n"), modes[currentMode].Brightness);
        #endif

        break;
      }

      case 1U:                                              // удержание после одного клика - изменение скорости
      {
        modes[currentMode].Speed = constrain(brightDirection ? modes[currentMode].Speed + 1 : modes[currentMode].Speed - 1, 1, 255);
        loadingFlag = true; // без перезапуска эффекта ничего и не увидишь

        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение скорости: %d\n"), modes[currentMode].Speed);
        #endif

        break;
      }

      case 2U:                                              // удержание после двух кликов - изменение масштаба
      {
        modes[currentMode].Scale = constrain(brightDirection ? modes[currentMode].Scale + 1 : modes[currentMode].Scale - 1, 1, 100);
        loadingFlag = true; // без перезапуска эффекта ничего и не увидишь

        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение масштаба: %d\n"), modes[currentMode].Scale);
        #endif

        break;
      }

      default:
        break;
    }

    settChanged = true;
    eepromTimeout = millis();
  }
  else
  #ifdef BUTTON_PAUSE_AFTER_TURN_ON
  if (breakButtonHolding)
  #endif
  {
    #ifdef BUTTON_PAUSE_AFTER_TURN_ON
    breakButtonHolding = false;
    #endif
   
    currentMode = EFF_WHITE_COLOR;
    ONflag = true;
    changePower();
    settChanged = true;
    eepromTimeout = millis();
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
  }

  // кнопка отпущена после удерживания
  if (ONflag && !touch.isHold() && startButtonHolding)      // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
  {
    #ifdef BUTTON_PAUSE_AFTER_TURN_ON
    breakButtonHolding = true;
    #endif

    startButtonHolding = false;
    loadingFlag = true;

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    
  }
}
#endif
