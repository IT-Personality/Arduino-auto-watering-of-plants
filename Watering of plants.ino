#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>

const int relayPin = 9;
const int lcdColumns = 16;
const int lcdRows = 2;
int wateringTimeSeconds = 0;
int wateringIntervalHours = 0;
unsigned long nextWateringTime = 0;
unsigned long timeLeftUntilNextWatering = 0;

const int encoderPin1 = 3;
const int encoderPin2 = 4;
const int encoderButtonPin = 5;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
Encoder encoder(encoderPin1, encoderPin2);

int currentStep = 1;
unsigned long lastEncoderActivity = 0;
unsigned long lastBacklightUpdate = 0;
unsigned long lastButtonClick = 0;
bool backlightEnabled = true;
bool encoderButtonPressed = false;
bool isChangingSettings = false;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  pinMode(encoderButtonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lastBacklightUpdate = millis();
}

void loop() {
  int encoderValue = encoder.read() / 4;

  if (encoderValue != 0) {
    lastEncoderActivity = millis();
    backlightEnabled = true;
  }
  if (millis() - lastEncoderActivity >= 5000) {
    backlightEnabled = false;
  }

  if (digitalRead(encoderButtonPin) == LOW) {
    if (!encoderButtonPressed && (millis() - lastButtonClick) < 300) {
      encoderButtonPressed = true;
      lastButtonClick = millis();
    } else if ((millis() - lastButtonClick) >= 1000) {
      encoderButtonPressed = false;
      lastButtonClick = millis();
      // Одиночный клик на энкодер
      lastBacklightUpdate = millis();
    }
  } else {
    if (encoderButtonPressed) {
      encoderButtonPressed = false;
      if (!isChangingSettings) {
        isChangingSettings = true;
        currentStep = 1;
      } else {
        isChangingSettings = false;
      }
    }
  }

  if (!isChangingSettings) {
    if (currentStep == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Watering time:");
      lcd.setCursor(0, 1);
      lcd.print(encoderValue);
      lcd.print("s");

      if (digitalRead(encoderButtonPin) == LOW && !encoderButtonPressed) {
        wateringTimeSeconds = encoderValue;
        currentStep = 2;
        delay(500);
      }
    } else if (currentStep == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Set interval:");
      lcd.setCursor(0, 1);
      lcd.print(encoderValue);
      lcd.print("h");

      if (digitalRead(encoderButtonPin) == LOW && !encoderButtonPressed) {
        wateringIntervalHours = encoderValue;
        nextWateringTime = millis() + (wateringIntervalHours * 3600000);
        currentStep = 3;
        delay(500);
      }
    } else if (currentStep == 3) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Interval: ");
      lcd.print(wateringIntervalHours);
      lcd.print("h");
      lcd.setCursor(0, 1);
      lcd.print("Time: ");
      lcd.print(wateringTimeSeconds);
      lcd.print("s");

      digitalWrite(relayPin, HIGH);
      delay(wateringTimeSeconds * 1000);
      digitalWrite(relayPin, LOW);

      nextWateringTime = millis() + (wateringIntervalHours * 3600000);
      timeLeftUntilNextWatering = (nextWateringTime - millis()) / 1000;

      currentStep = 4;
      lastBacklightUpdate = millis();
    } else if (currentStep == 4) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Next:");
      int hoursLeft = timeLeftUntilNextWatering / 3600;
      int minutesLeft = (timeLeftUntilNextWatering % 3600) / 60;
      int secondsLeft = timeLeftUntilNextWatering % 60;
      lcd.print(hoursLeft);
      lcd.print("h ");
      lcd.print(minutesLeft);
      lcd.print("m ");
      lcd.print(secondsLeft);
      lcd.print("s");
      lcd.setCursor(0, 1);
      lcd.print("Time: ");
      lcd.print(wateringTimeSeconds);
      lcd.print("s");

      timeLeftUntilNextWatering = (nextWateringTime - millis()) / 1000;
      if (timeLeftUntilNextWatering <= 0) {
        currentStep = 3;
      }
      if (millis() - lastBacklightUpdate >= 10000) {
        backlightEnabled = false;
      }
    }
  } else {
    // Handle changing settings
    if (currentStep == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Change time:");
      lcd.setCursor(0, 1);
      lcd.print(encoderValue);
      lcd.print("s");

      if (digitalRead(encoderButtonPin) == LOW && !encoderButtonPressed) {
        wateringTimeSeconds = encoderValue;
        currentStep = 2;
        delay(500);
      }
    } else if (currentStep == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Change interval:");
      lcd.setCursor(0, 1);
      lcd.print(encoderValue);
      lcd.print("h");

      if (digitalRead(encoderButtonPin) == LOW && !encoderButtonPressed) {
        wateringIntervalHours = encoderValue;
        nextWateringTime = millis() + (wateringIntervalHours * 3600000);
        currentStep = 3;
        delay(500);
      }
    } else if (currentStep == 3) {
      // Return to main loop
      isChangingSettings = false;
      currentStep = 4;
      lastBacklightUpdate = millis();
    }
  }

  if (backlightEnabled || (millis() - lastBacklightUpdate) <= 10000) {
    lcd.backlight(); 
  } else {
    lcd.noBacklight(); 
  }
  delay(100);
}
