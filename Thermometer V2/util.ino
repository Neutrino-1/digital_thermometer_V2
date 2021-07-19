// DS18B20 utility

float getTemperature()
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  #if DEBUG
  Serial.println("Sending request to sensor");
  #endif
  float tempC = sensors.getTempCByIndex(0);

  // Check if reading was successful
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    #if CELSIUS
      return tempC;
      #if DEBUG
      Serial.println("Sending request to sensor");
      #endif
    #else
      return DallasTemperature::toFahrenheit(tempC);
    #endif
  } 
  else
  {
    #if DEBUG
    Serial.println("Error: Could not read temperature data");
    #endif
    return 1;
  }
}

// Dispaly utility

/*Different frames*/
void drawFrame(void (*drawableFrame)())
{ 
   u8g2.firstPage(); 
  do{
     drawableFrame(); 
  }while(u8g2.nextPage());
}

void firstBoot()
{
   u8g2.setFont(u8g2_font_courR08_tf);
   u8g2.drawStr(10,10,"Smart Thermometer"); 
}

void readyToMeasure()
{
  u8g2.setFont(u8g2_font_courR08_tf);
   u8g2.drawStr(10,10,"Start to measure"); 
}

void ReadingTemperature()
{
  u8g2.setFont(u8g2_font_courR08_tf);
   u8g2.drawStr(10,10,"Measuring...."); 
}

void showTemperature(float temperature)
{
   int numberOfBar; 
   u8g2.clear();
   u8g2.setFont(u8g2_font_courR08_tf);
   u8g2.setCursor(10,10);
   u8g2.print(temperature);
   u8g2.setCursor(10,20);
   u8g2.print("  C");
   u8g2.updateDisplay();
}

void deepSleepMCU()
{
    //clear the screen 
    u8g2.clear();
    //put to deep sleep 
    ESP.deepSleep(0);
}
