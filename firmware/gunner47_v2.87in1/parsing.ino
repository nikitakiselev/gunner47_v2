void parseUDP()
{
  int32_t packetSize = Udp.parsePacket();

  if (packetSize)
  {
    int16_t n = Udp.read(packetBuffer, MAX_UDP_BUFFER_SIZE);
    packetBuffer[n] = '\0';
    strcpy(inputBuffer, packetBuffer);

    #ifdef GENERAL_DEBUG
    LOG.print(F("Inbound UDP packet: "));
    LOG.println(inputBuffer);
    #endif

    if (Udp.remoteIP() == WiFi.localIP())                   // не реагировать на свои же пакеты
    {
      return;
    }

    char reply[MAX_UDP_BUFFER_SIZE];
    processInputBuffer(inputBuffer, reply, true);

    #if (USE_MQTT)                                          // отправка ответа выполнения команд по MQTT, если разрешено
    if (espMode == 1U)
    {
      strcpy(MqttManager::mqttBuffer, reply);               // разрешение определяется при выполнении каждой команды отдельно, команды GET, DEB, DISCOVER и OTA, пришедшие по UDP, игнорируются (приходят раз в 2 секунды от приложения)
    }
    #endif
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(reply);
    Udp.endPacket();

    #ifdef GENERAL_DEBUG
    LOG.print(F("Outbound UDP packet: "));
    LOG.println(reply);
    LOG.println();
    #endif
  }
}

void updateSets()
{
      loadingFlag = true;
      settChanged = true;
      eepromTimeout = millis();

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
}

void processInputBuffer(char *inputBuffer, char *outputBuffer, bool generateOutput)
{
    char buff[MAX_UDP_BUFFER_SIZE], *endToken = NULL;
    String BUFF = String(inputBuffer);

    if (!strncmp_P(inputBuffer, PSTR("GET"), 3))
    {
      #ifdef GET_TIME_FROM_PHONE
      if (!timeSynched || !(ntpServerAddressResolved && espMode == 1U) && manualTimeShift + millis() / 1000UL > phoneTimeLastSync + GET_TIME_FROM_PHONE * 60U)
      {// если прошло более 5 минут (GET_TIME_FROM_PHONE 5U), значит, можно парсить время из строки GET
        if (BUFF.length() > 7U){ // пускай будет хотя бы 7
          memcpy(buff, &inputBuffer[4], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 5
          phoneTimeLastSync = (time_t)atoi(buff);
          manualTimeShift = phoneTimeLastSync - millis() / 1000UL;
          #ifdef WARNING_IF_NO_TIME
            noTimeClear();
          #endif // WARNING_IF_NO_TIME  
          timeSynched = true;
          #if defined(PHONE_N_MANUAL_TIME_PRIORITY) && defined(USE_NTP)
            stillUseNTP = false;
          #endif
        }
      }
      #endif // GET_TIME_FROM_PHONE
      sendCurrent(inputBuffer);
    }
#if defined(GENERAL_DEBUG) || defined(USE_OLD_IOS_APP)
    else if (!strncmp_P(inputBuffer, PSTR("DEB"), 3))
    {
        //#ifdef USE_NTP
        #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
        getFormattedTime(inputBuffer);
        sprintf_P(inputBuffer, PSTR("OK %s"), inputBuffer);
        #else
        strcpy_P(inputBuffer, PSTR("OK --:--"));
        #endif
    }
#endif    
    else if (!strncmp_P(inputBuffer, PSTR("EFF"), 3))
    {
      EepromManager::SaveModesSettings(&currentMode, modes);
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      currentMode = (uint8_t)atoi(buff);
      updateSets();
      sendCurrent(inputBuffer);
      //FastLED.clear();
      //delay(1);

      #ifdef USE_BLYNK_PLUS
      updateRemoteBlynkParams();
      #endif

      #ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
        if (random_on && FavoritesManager::FavoritesRunning)
          selectedSettings = 1U;
      #endif //RANDOM_SETTINGS_IN_CYCLE_MODE
      
      FastLED.setBrightness(modes[currentMode].Brightness);
    }

    else if (!strncmp_P(inputBuffer, PSTR("BRI"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Brightness = constrain(atoi(buff), 1, 255);
      FastLED.setBrightness(modes[currentMode].Brightness);
      //loadingFlag = true; //не хорошо делать перезапуск эффекта после изменения яркости, но в некоторых эффектах от чётности яркости мог бы зависеть внешний вид
      settChanged = true;
      eepromTimeout = millis();
      sendCurrent(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif

      #ifdef USE_BLYNK_PLUS
      updateRemoteBlynkParams();
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("SPD"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Speed = atoi(buff);
      updateSets();
      sendCurrent(inputBuffer);
      #ifdef USE_BLYNK_PLUS
      updateRemoteBlynkParams();
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("SCA"), 3))
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      modes[currentMode].Scale = atoi(buff);
      updateSets();
      sendCurrent(inputBuffer);
      #ifdef USE_BLYNK_PLUS
      updateRemoteBlynkParams();
      #endif
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_ON"), 4))
    {
      if (dawnFlag) {
        manualOff = true; 
        dawnFlag = false;
        sendCurrent(inputBuffer);
      }
      else {
        ONflag = true;
        updateSets();
        changePower();
        sendCurrent(inputBuffer);
        #ifdef USE_BLYNK_PLUS
        updateRemoteBlynkParams();
        #endif
      }  
    }

    else if (!strncmp_P(inputBuffer, PSTR("P_OFF"), 5))
    {
      if (dawnFlag) {
        manualOff = true; 
        dawnFlag = false;
        sendCurrent(inputBuffer);
      }
      else {
        ONflag = false;
        settChanged = true;
        eepromTimeout = millis();
        changePower();
        sendCurrent(inputBuffer);

        #if (USE_MQTT)
        if (espMode == 1U)
        {
          MqttManager::needToPublish = true;
        }
        #endif
        #ifdef USE_BLYNK_PLUS
        updateRemoteBlynkParams();
        #endif
      }
    }

    else if (!strncmp_P(inputBuffer, PSTR("ALM_"), 4)) { // сокращаем GET и SET для ускорения регулярного цикла
      if (!strncmp_P(inputBuffer, PSTR("ALM_SET"), 7))
      {
        uint8_t alarmNum = (char)inputBuffer[7] - '0';
        alarmNum -= 1;
        if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 9)
        {
          alarms[alarmNum].State = true;
          sendAlarms(inputBuffer);
        }
        else if (strstr_P(inputBuffer, PSTR("OFF")) - inputBuffer == 9)
        {
          alarms[alarmNum].State = false;
          sendAlarms(inputBuffer);
        }
        else
        {
          memcpy(buff, &inputBuffer[8], strlen(inputBuffer)); // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 9
          alarms[alarmNum].Time = atoi(buff);
          sendAlarms(inputBuffer);
        }
        EepromManager::SaveAlarmsSettings(&alarmNum, alarms);

        #if (USE_MQTT)
        if (espMode == 1U)
        {
          strcpy(MqttManager::mqttBuffer, inputBuffer);
          MqttManager::needToPublish = true;
        }
        #endif
      }
      else
        sendAlarms(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("DAWN"), 4))
    {
      memcpy(buff, &inputBuffer[4], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 5
      dawnMode = atoi(buff) - 1;
      EepromManager::SaveDawnMode(&dawnMode);
      sendAlarms(inputBuffer);

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        MqttManager::needToPublish = true;
      }
      #endif
    }
    
    else if (!strncmp_P(inputBuffer, PSTR("DISCOVER"), 8))  // обнаружение приложением модуля esp в локальной сети
    {
      if (espMode == 1U)                                    // работает только в режиме WiFi клиента. интересно, зачем было запрещать обнаружение точки доступа?
      {
        #ifdef SEND_LAMP_NAME_TO_APP
          sprintf_P(inputBuffer, PSTR("IP %u.%u.%u.%u:%u:%s"),
          WiFi.localIP()[0],
          WiFi.localIP()[1],
          WiFi.localIP()[2],
          WiFi.localIP()[3],
          ESP_UDP_PORT,
          AP_NAME);
        #else
          sprintf_P(inputBuffer, PSTR("IP %u.%u.%u.%u:%u"),
          WiFi.localIP()[0],
          WiFi.localIP()[1],
          WiFi.localIP()[2],
          WiFi.localIP()[3],
          ESP_UDP_PORT);
        #endif
      }
      else
      {
        #ifdef SEND_LAMP_NAME_TO_APP
          sprintf_P(inputBuffer, PSTR("IP %u.%u.%u.%u:%u:%s"),
          AP_STATIC_IP[0],
          AP_STATIC_IP[1],
          AP_STATIC_IP[2],
          AP_STATIC_IP[3],
          ESP_UDP_PORT,
          AP_NAME);
        #else
          sprintf_P(inputBuffer, PSTR("IP %u.%u.%u.%u:%u"),
          AP_STATIC_IP[0],
          AP_STATIC_IP[1],
          AP_STATIC_IP[2],
          AP_STATIC_IP[3],
          ESP_UDP_PORT);
        #endif
      }
    }

    else if (!strncmp_P(inputBuffer, PSTR("TMR_"), 4)) { // сокращаем GET и SET для ускорения регулярного цикла
      if (!strncmp_P(inputBuffer, PSTR("TMR_SET"), 7))
      {
        memcpy(buff, &inputBuffer[8], 2);                     // взять подстроку, состоящую из 9 и 10 символов, из строки inputBuffer
        TimerManager::TimerRunning = (bool)atoi(buff);

        memcpy(buff, &inputBuffer[10], 2);                    // взять подстроку, состоящую из 11 и 12 символов, из строки inputBuffer
        TimerManager::TimerOption = (uint8_t)atoi(buff);

        memcpy(buff, &inputBuffer[12], strlen(inputBuffer));  // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 13
        TimerManager::TimeToFire = millis() + strtoull(buff, &endToken, 10) * 1000;

        #if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
          if (TimerManager::TimerRunning){
            button_sleep_time = constrain(strtoull(buff, &endToken, 10) / 60, 1 , 255);
            EepromManager::Save_button_sleep_time ( &button_sleep_time );
          }
        #endif // #if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)

        TimerManager::TimerHasFired = false;
        sendTimer(inputBuffer);

        #if (USE_MQTT)
        if (espMode == 1U)
        {
          MqttManager::needToPublish = true;
        }
        #endif
      }
      else
        sendTimer(inputBuffer);
    }

    else if (!strncmp_P(inputBuffer, PSTR("FAV_"), 4)) { // сокращаем GET и SET для ускорения регулярного цикла
      if (!strncmp_P(inputBuffer, PSTR("FAV_SET"), 7))
      {
        FavoritesManager::ConfigureFavorites(inputBuffer);
        FavoritesManager::SetStatus(inputBuffer);
        settChanged = true;
        eepromTimeout = millis();

        #if (USE_MQTT)
        if (espMode == 1U)
        {
          MqttManager::needToPublish = true;
        }
        #endif
      }
      else
        FavoritesManager::SetStatus(inputBuffer);
    }

    #ifdef OTA
    else if (!strncmp_P(inputBuffer, PSTR("OTA"), 3))
    {
      //if (espMode == 1U) пускай обновление работает даже в режиме точки доступа
      // && otaManager.RequestOtaUpdate()){ по идее, нужен положительный ответ от менеджера
        otaManager.RequestOtaUpdate(); // но из-за двойного запроса нихрена не работает
        delay(70);
        //if (otaManager.RequestOtaUpdate()) //по идее, нужен положительный ответ от менеджера
        otaManager.RequestOtaUpdate(); // но если уже был один ответ из двух в прошлый раз, то сейчас второй лучше не проверять
        if (OtaManager::OtaFlag == OtaPhase::InProgress) {
          currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
          FastLED.clear();
          delay(1);
          ONflag = true;
          changePower();
        }
        else
          showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)
      //}
      //else
      //  showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)
    }
    #endif // OTA

    else if (!strncmp_P(inputBuffer, PSTR("BTN"), 3))
    {
      if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 4)
      {
        buttonEnabled = true;
        EepromManager::SaveButtonEnabled(&buttonEnabled);
        sendCurrent(inputBuffer);
      }
      else// if (strstr_P(inputBuffer, PSTR("OFF")) - inputBuffer == 4)
      {
        buttonEnabled = false;
        EepromManager::SaveButtonEnabled(&buttonEnabled);
        sendCurrent(inputBuffer);
      }

      #if (USE_MQTT)
      if (espMode == 1U)
      {
        strcpy(MqttManager::mqttBuffer, inputBuffer);
        MqttManager::needToPublish = true;
      }
      #endif
    }
    else if (!strncmp_P(inputBuffer, PSTR("GBR"), 3)) // выставляем общую яркость для всех эффектов без сохранения в EEPROM, если приложение присылает такую строку
    {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
      uint8_t ALLbri = constrain(atoi(buff), 1, 255);
      for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
        modes[i].Brightness = ALLbri;
      }
      FastLED.setBrightness(ALLbri);
      loadingFlag = true;
    }
    #ifdef USE_RANDOM_SETS_IN_APP
    else if (!strncmp_P(inputBuffer, PSTR("RND_"), 4)) // управление включением случайных настроек
    {
       if (!strncmp_P(inputBuffer, PSTR("RND_0"), 5)) // вернуть настройки по умолчанию текущему эффекту
       {
         setModeSettings();
         updateSets();
         sendCurrent(inputBuffer);
       }
       else if (!strncmp_P(inputBuffer, PSTR("RND_1"), 5)) // выбрать случайные настройки текущему эффекту
       { // раньше была идея, что будут числа RND_1, RND_2, RND_3 - выбор из предустановленных настроек, но потом всё свелось к единственному варианту случайных настроек
         selectedSettings = 1U;
         updateSets();
       }
       else if (!strncmp_P(inputBuffer, PSTR("RND_Z"), 5)) // вернуть настройки по умолчанию всем эффектам
       {
         restoreSettings();
         selectedSettings = 0U;
         updateSets();
         sendCurrent(inputBuffer);
         #ifdef USE_BLYNK
         updateRemoteBlynkParams();
         #endif
       }
       #ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
       else if (!strncmp_P(inputBuffer, PSTR("RND_ON"), 6)) // включить выбор случайных настроек в режиме Цикл
       {
         random_on = 1U;
         EepromManager::Save_random_on ( &random_on );
         showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
       }
       else if (!strncmp_P(inputBuffer, PSTR("RND_OFF"), 7)) // отключить выбор случайных настроек в режиме Цикл
       {
         random_on = 0U;
         EepromManager::Save_random_on ( &random_on );
         showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
       }
       #endif //#ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
    }
    #endif //#ifdef USE_RANDOM_SETS_IN_APP
    else if (!strncmp_P(inputBuffer, PSTR("LIST"), 4)) // передача списка эффектов по запросу от приложения (если поддерживается приложением)
    {
       memcpy(buff, &inputBuffer[4], strlen(inputBuffer));  // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 5
       switch (atoi(buff))
         {
           case 1U:
           {
             Udp.write(efList_1.c_str());
             Udp.write("\0");
             break;
           }
           case 2U:
           {
             Udp.write(efList_2.c_str());
             Udp.write("\0");
             break;
           }
           case 3U:
           {
             Udp.write(efList_3.c_str());
             Udp.write("\0");

             #ifdef USE_DEFAULT_SETTINGS_RESET
             // и здесь же после успешной отправки списка эффектов делаем сброс настроек эффектов на значения по умолчанию
             restoreSettings();
             updateSets();
             #ifdef USE_BLYNK_PLUS
             updateRemoteBlynkParams();
             #endif
             #endif

             break;
           }
         }
    }
    else if (!strncmp_P(inputBuffer, PSTR("TXT"), 3)){     // Принимаем текст для бегущей строки
      #if defined(USE_SECRET_COMMANDS) || defined(USE_MANUAL_TIME_SETTING) // вкорячиваем ручную синхранизацию времени пока что сюда. пока нет другой функции в приложении...
        if (!strncmp_P(inputBuffer, PSTR("TXT-time="), 9) && (BUFF.length() > 15)){ 
          #ifdef USE_MANUAL_TIME_SETTING // всё-таки если данная директива не объявлена, то нет смысла высчитывать ручное время. использовать его всё равно не будет никто
            // 0000000000111111
            // 0123456789012345
            // TXT-time=07:25 7
            uint8_t mtH = BUFF.substring(9, 11).toInt();
            uint8_t mtM = BUFF.substring(12, 14).toInt();
            uint8_t mtD = BUFF.substring(15, 16).toInt();
            if (mtH < 24U && mtM < 60U && mtD < 8U && mtD > 0U){
              manualTimeShift = (((3650UL + mtD) * 24UL + mtH) * 60UL + mtM) * 60UL - millis() / 1000UL; // 3650 дней (521 полная неделя + 3 дня для сдвига на понедельник???)
              #ifdef GET_TIME_FROM_PHONE
                phoneTimeLastSync = manualTimeShift + millis() / 1000UL;
              #endif
              #ifdef WARNING_IF_NO_TIME
                noTimeClear();
              #endif
              timeSynched = true;
              #if defined(PHONE_N_MANUAL_TIME_PRIORITY) && defined(USE_NTP)
                stillUseNTP = false;
              #endif
              showWarning(CRGB::Blue, 2000U, 500U);     // мигание голубым цветом 2 секунды (2 раза) - время установлено
            }
            else
              showWarning(CRGB::Red, 2000U, 500U);      // мигание красным цветом 2 секунды (ошибка)
/*    Я БЕЗ ПОНЯТИЯ, ПОЧЕМУ ТЕКСТ В БЕГУЩЕЙ СТРОКЕ С КАЖДЫМ РАЗОМ СДВИГАЕТСЯ ВСЁ СИЛЬНЕЕ
            mtD = weekday(millis() / 1000UL + manualTimeShift);
            mtD = (mtD == 7) ? 1 : mtD + 1;
            String str = "УСТАНОВЛЕНО! ДЕНЬ НЕДЕЛИ = " + String(mtD) + String("\n");;
//            String str = String(mtH) + ":" + String(mtM) + " ДН:" + String(mtD) + " =" + String(weekday(millis() / 1000UL + manualTimeShift) + 1U) + String("\n");;
            
            str.toCharArray(TextTicker, str.length() + 1);
          }
          else
            TextTicker = "ОШИБКА ФОРМАТА ВРЕМЕНИ\n";
        currentMode = EFF_TEXT;                             // принудительное включение режима "Бегущая строка" для сообщения результате
        FastLED.clear();
        delay(1);
        ONflag = true;
        changePower();
*/        
          #else
            showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)          
          #endif // USE_MANUAL_TIME_SETTING
        }
        #ifdef USE_SECRET_COMMANDS
          else if (!strncmp_P(inputBuffer, PSTR("TXT-esp_mode=0"), 14)){
            if (espMode == 1U){
              espMode = 0U;
              EepromManager::SaveEspMode(&espMode);
              showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды - смена рабочего режима лампы, перезагрузка
              ESP.restart();
            }
            else
              showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)
          }
          else if (!strncmp_P(inputBuffer, PSTR("TXT-esp_mode=1"), 14)){
            if (espMode == 0U){
              espMode = 1U;
              EepromManager::SaveEspMode(&espMode);
              showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды - смена рабочего режима лампы, перезагрузка
              ESP.restart();
            }
            else
              showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)
          }
          else if (!strncmp_P(inputBuffer, PSTR("TXT-reset=wifi"), 14)){
            wifiManager.resetSettings();                             // сброс сохранённых SSID и пароля (сброс настроек подключения к роутеру)
            showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
          }
          else if (!strncmp_P(inputBuffer, PSTR("TXT-reset=effects"), 17)){
            restoreSettings();
            updateSets();
            showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
            #ifdef USE_BLYNK
            updateRemoteBlynkParams();
            #endif
          }
          else if (!strncmp_P(inputBuffer, PSTR("TXT-alarm"), 9)&& (BUFF.length() > 12) && (char)inputBuffer[10] == '='){
            // 0000000000111111
            // 0123456789012345
            // TXT-alarm4=07:25
            // TXT-alarm5=on
            // TXT-alarm2=off
            bool isError = false;
            uint8_t alarmNum = (char)inputBuffer[9] - '0' - 1U;
            if (strstr_P(inputBuffer, PSTR("on")) - inputBuffer == 11 && alarmNum < 7U)
            {
              alarms[alarmNum].State = true;
            }
            else if (strstr_P(inputBuffer, PSTR("off")) - inputBuffer == 11 && alarmNum < 7U)
            {
              alarms[alarmNum].State = false;
            }
            else if (BUFF.length() > 15)
            {
              uint8_t mtH = BUFF.substring(11, 13).toInt();
              uint8_t mtM = BUFF.substring(14, 16).toInt();
              if (mtH < 24U && mtM < 60U && alarmNum < 7U){
                alarms[alarmNum].Time = mtH * 60U + mtM;
                alarms[alarmNum].State = true;
              }
              else
                isError = true;
            }
            else 
              isError = true;
              
            if (isError){
              #ifdef USE_BLYNK
                Blynk.setProperty(V6, "label", String("Ошибка!"));
              #endif              
              showWarning(CRGB::Red, 2000U, 500U);      // мигание красным цветом 2 секунды (ошибка)
            }
            else {
              #ifdef USE_BLYNK
                Blynk.setProperty(V6, "label", String("Рассвет в ")+String(alarmNum+1U)+String("й д.н. ")+String(alarms[alarmNum].State ? String((uint8_t)(alarms[alarmNum].Time / 600U))+String((uint8_t)(alarms[alarmNum].Time / 60U % 10U))+':'+String(alarms[alarmNum].Time % 60U / 10U)+String(alarms[alarmNum].Time % 60U % 10U) : "ВЫКЛЮЧЕН"));
              #endif              
              showWarning(CRGB::Blue, 2000U, 500U);     // мигание голубым цветом 2 секунды (2 раза) - будильник установлен
              
              EepromManager::SaveAlarmsSettings(&alarmNum, alarms);
            }
          }
          else if (!strncmp_P(inputBuffer, PSTR("TXT-dawn="), 9)){
            memcpy(buff, &inputBuffer[9], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 10
            uint8_t temp = atoi(buff);
            if (temp) {
              //dawnOffsets[dawnMode] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60};
              dawnMode = 0U;
              for (uint8_t i = 1; i < 9; i++)
                if (temp >= pgm_read_byte(&dawnOffsets[i]))
                  dawnMode = i;
                else
                  break;
            EepromManager::SaveDawnMode(&dawnMode);
              showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
            }
            else{
              showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)
            }
            #ifdef USE_BLYNK
              Blynk.setProperty(V6, "label", String("Рассвет начнётся за ") + String(pgm_read_byte(&dawnOffsets[dawnMode])) + String(" мин."));
            #endif                
          }
          else if (!strncmp_P(inputBuffer, PSTR("TXT-timer=off"), 13)){
            TimerManager::TimerRunning = false;
            showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
            #ifdef USE_BLYNK
              Blynk.setProperty(V6, "label", String("Таймер отключен"));
            #endif                
          }
          else if (!strncmp_P(inputBuffer, PSTR("TXT-timer="), 10)){
            memcpy(buff, &inputBuffer[10], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 11
            uint16_t temp = atoi(buff);
            if (ONflag && temp) {
              #if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
                if (TimerManager::TimerRunning){
                  button_sleep_time = constrain(temp, 1U , 255U);
                  EepromManager::Save_button_sleep_time ( &button_sleep_time );
                }
              #endif // #if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
              
              TimerManager::TimeToFire = millis() + temp * 60UL * 1000UL;
              TimerManager::TimerRunning = true;
              showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
              #ifdef USE_BLYNK
                Blynk.setProperty(V6, "label", String("Выключение через ") + String(temp) + String(" мин."));
              #endif                
            }
            else{
              showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)
              #ifdef USE_BLYNK
                Blynk.setProperty(V6, "label", TimerManager::TimerRunning ? String("Выключение через ") + String((uint16_t)floor((TimerManager::TimeToFire - millis()) / 60000U)) + String(" мин.") : String("Таймер отключен"));
              #endif                
            }
          }
          #ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
          else if (!strncmp_P(inputBuffer, PSTR("TXT-random="), 11)){
            if (strstr_P(inputBuffer, PSTR("on")) - inputBuffer == 11)
            {
              random_on = 1U;
              EepromManager::Save_random_on ( &random_on );
              showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
            }
            else if (strstr_P(inputBuffer, PSTR("off")) - inputBuffer == 11)
            {
              random_on = 0U;
              EepromManager::Save_random_on ( &random_on );
              showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
            }
          }
          #endif //#ifdef RANDOM_SETTINGS_IN_CYCLE_MODE
          #if defined(off_BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
          else if (!strncmp_P(inputBuffer, PSTR("TXT-sleep="), 10)){
            memcpy(buff, &inputBuffer[10], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 11
            int16_t temp = atoi(buff);
            if (temp > 0 && temp <=255) {
              button_sleep_time = temp;
              EepromManager::Save_button_sleep_time ( &button_sleep_time );
              showWarning(CRGB::Blue, 2000U, 500U);                    // мигание синим цветом 2 секунды
            }
            else
              showWarning(CRGB::Red, 2000U, 500U);                     // мигание красным цветом 2 секунды (ошибка)
          }
          #endif //#if defined(BUTTON_CAN_SET_SLEEP_TIMER) && defined(ESP_USE_BUTTON)
        #endif // USE_SECRET_COMMANDS
        else
        {  
          //String str = getValue(BUFF, '-', 1); // этим способом дефисы нельзя в бегущую строку передать. почему вообще разделитель - дефис?!
          String str = (BUFF.length() > 4) ? BUFF.substring(4, BUFF.length()) : "";
          str.toCharArray(TextTicker, str.length() + 1);
        }
      #else
        //String str = getValue(BUFF, '-', 1); // этим способом дефисы нельзя в бегущую строку передать. почему вообще разделитель - дефис?!
        String str = (BUFF.length() > 4) ? BUFF.substring(4, BUFF.length()) : "";
        str.toCharArray(TextTicker, str.length() + 1);
      #endif // defined(USE_SECRET_COMMANDS) || defined(USE_MANUAL_TIME_SETTING)
    }
    else if (!strncmp_P(inputBuffer, PSTR("DRW"), 3)) {
      drawPixelXY((int8_t)getValue(BUFF, ';', 1).toInt(), (int8_t)getValue(BUFF, ';', 2).toInt(), DriwingColor);
      FastLED.show();
    }
    else if (!strncmp_P(inputBuffer, PSTR("CLR"), 3)) {
      FastLED.clear();
      FastLED.show();
    }
    else if (!strncmp_P(inputBuffer, PSTR("COL"), 3)) {
      #ifdef USE_OLD_APP_FROM_KOTEYKA // (в версии 2.3... цвета были только в формате RGB)
       DriwingColor = CRGB(getValue(BUFF, ';', 1).toInt(), getValue(BUFF, ';', 2).toInt(), getValue(BUFF, ';', 3).toInt());
      #else
       DriwingColor = CRGB(getValue(BUFF, ';', 1).toInt(), getValue(BUFF, ';', 3).toInt(), getValue(BUFF, ';', 2).toInt());
      #endif
    }
    else if (!strncmp_P(inputBuffer, PSTR("DRAWO"), 5)) { // сокращаем OFF и ON для ускорения регулярного цикла
      if (!strncmp_P(inputBuffer, PSTR("DRAWON"), 6))
        Painting = 1;
      else
        Painting = 0;
        //FastLED.clear();
        //FastLED.show();
    }
    else if (!strncmp_P(inputBuffer, PSTR("RESET"), 5)) { // сброс настроек WIFI по запросу от приложения
      wifiManager.resetSettings();
    }
//#ifdef USE_OLD_APP_FROM_KOTEYKA // (в версии 2.3... были кнопки, чтобы сохранить настройки эффектов из приложения в лампу)
//и в новых тоже появились
    else if (!strncmp_P(inputBuffer, PSTR("SETS"), 4)) // передача настроек эффектов по запросу от приложения (если поддерживается приложением)
    {
      memcpy(buff, &inputBuffer[4], 1U);  // взять первую циферку из строки inputBuffer, начиная с символа 5
      switch (atoi(buff))      
      {
        case 1U: // SET
          {
            memcpy(buff, &inputBuffer[5], strlen(inputBuffer));   // inputBuffer, начиная с символа 6
            uint8_t eff = getValue(buff, ';', 0).toInt();
            modes[eff].Brightness = getValue(buff, ';', 1).toInt();
            modes[eff].Speed = getValue(buff, ';', 2).toInt();
            modes[eff].Scale = getValue(buff, ';', 3).toInt();
            if (eff == currentMode) {
              updateSets();
              #ifdef USE_BLYNK_PLUS
              updateRemoteBlynkParams();
              #endif
            }
            break;
          }
        case 2U: // READ
          {
            String OutString;
            char replyPacket[MAX_UDP_BUFFER_SIZE];
            for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
              OutString = String(i) + ";" +  String(modes[i].Brightness) + ";" + String(modes[i].Speed) + ";" + String(modes[i].Scale) + "\n";
              OutString.toCharArray(replyPacket, MAX_UDP_BUFFER_SIZE);
              Udp.write(replyPacket);
            }
            break;
          }
      }
    }
//#endif // ifdef USE_OLD_APP_FROM_KOTEYKA
    else
    {
      inputBuffer[0] = '\0';
    }

    if (strlen(inputBuffer) <= 0)
    {
      return;
    }

    if (generateOutput)                                     // если запрошен вывод ответа выполнения команд, копируем его в исходящий буфер
    {
      strcpy(outputBuffer, inputBuffer);
    }
    inputBuffer[0] = '\0';                                  // очистка буфера, читобы не он не интерпретировался, как следующий входной пакет
}

void sendCurrent(char *outputBuffer)
{
  sprintf_P(outputBuffer, PSTR("CURR %u %u %u %u %u %u"),
    currentMode,
    modes[currentMode].Brightness,
    modes[currentMode].Speed,
    modes[currentMode].Scale,
    ONflag,
    espMode);
  
  #ifdef USE_NTP
  strcat_P(outputBuffer, PSTR(" 1"));
  #else
  strcat_P(outputBuffer, PSTR(" 0"));
  #endif

  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)TimerManager::TimerRunning);
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)buttonEnabled);

  //#ifdef USE_NTP
  #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
  char timeBuf[9];
  getFormattedTime(timeBuf);
  sprintf_P(outputBuffer, PSTR("%s %s"), outputBuffer, timeBuf);
  #else
  time_t currentTicks = millis() / 1000UL;
  sprintf_P(outputBuffer, PSTR("%s %02u:%02u:%02u"), outputBuffer, hour(currentTicks), minute(currentTicks), second(currentTicks));
  #endif
}

void sendAlarms(char *outputBuffer)
{
  strcpy_P(outputBuffer, PSTR("ALMS"));

  for (byte i = 0; i < 7; i++)
  {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)alarms[i].State);
  }

  for (byte i = 0; i < 7; i++)
  {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, alarms[i].Time);
  }

  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, dawnMode + 1);
}

void sendTimer(char *outputBuffer)
{
  sprintf_P(outputBuffer, PSTR("TMR %u %u %u"),
    TimerManager::TimerRunning,
    TimerManager::TimerOption,
   (TimerManager::TimerRunning ? (uint16_t)floor((TimerManager::TimeToFire - millis()) / 1000) : 0));
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
