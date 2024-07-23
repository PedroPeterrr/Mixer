#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// Define the SS_PIN and RST_PIN variables with appropriate pin numbers
const int SS_PIN = 10; 
const int RST_PIN = 9; 

// Create an instance of the MFRC522 class
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Define the relay pin
const int relayPin = A0;
const int onButtonPin = 7; // Assuming push button is connected to pin 7
const int offButtonPin = 6; // Assuming other button is connected to pin 6
const int emergencyStopPin = 8; // Emergency stop button connected to pin 8

// LED and Buzzer pins
const int relayLedPin = 3;
const int emergencyLedPin = 4;
const int buzzerPin = 5;

// Authorized RFID UID
const String authorizedCardUID = "YOUR_AUTHORIZED_CARD_UID"; // Replace with your actual authorized card UID

// Create an instance of the LiquidCrystal_I2C class
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
    // Initialize the serial communication
    Serial.begin(9600);

    // Initialize the RFID reader
    SPI.begin();
    mfrc522.PCD_Init();
    
    // Initialize the relay pin as an output
    pinMode(relayPin, OUTPUT);
    
    // Initialize the push button pins as inputs with internal pull-up resistors
    pinMode(onButtonPin, INPUT_PULLUP);
    pinMode(offButtonPin, INPUT_PULLUP);
    pinMode(emergencyStopPin, INPUT_PULLUP);

    // Initialize the LED and Buzzer pins as outputs
    pinMode(relayLedPin, OUTPUT);
    pinMode(emergencyLedPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    // Initialize the LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Tap RFID or");
    lcd.setCursor(0, 1);
    lcd.print("Push Button");
}

void loop() {
    // Check if the emergency stop button is pressed
    if (digitalRead(emergencyStopPin) == LOW) {
        // Turn off the relay
        digitalWrite(relayPin, LOW);
        // Turn off the relay LED
        digitalWrite(relayLedPin, LOW);
        // Turn on the emergency LED
        digitalWrite(emergencyLedPin, HIGH);
        // Display emergency stop message on the LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Emergency Stop");
        while (digitalRead(emergencyStopPin) == LOW) {
            // Wait here until the emergency stop button is released
        }
        // Turn off the emergency LED
        digitalWrite(emergencyLedPin, LOW);
        // Reset the LCD message after the emergency stop button is released
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tap RFID or");
        lcd.setCursor(0, 1);
        lcd.print("Push Button");
    }
    
    // Check if a new RFID card is present
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        // Read the card UID
        String cardUID = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
            cardUID += String(mfrc522.uid.uidByte[i], HEX);
        }
        
        // Print the card UID to the serial monitor
        Serial.print("Card UID: ");
        Serial.println(cardUID);
        
        // Check if the card is authorized
        if (cardUID == authorizedCardUID) {
            // Check if the on button is pressed
            if (digitalRead(onButtonPin) == LOW) {
                // Turn on the relay
                digitalWrite(relayPin, HIGH);
                // Turn on the relay LED
                digitalWrite(relayLedPin, HIGH);

                // Display message on the LCD
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Relay ON");

                // Countdown from 10 to 0
                for (int i = 10; i >= 0; i--) {
                    lcd.setCursor(0, 1);
                    lcd.print("Countdown: ");
                    lcd.print(i);

                    // Make a ticking sound when the timer reaches 3 seconds
                    if (i <= 3) {
                        digitalWrite(buzzerPin, HIGH);
                        delay(100); // Buzz for 100ms
                        digitalWrite(buzzerPin, LOW);
                    }
                    delay(1000); // Wait for 1 second
                }

                // Turn off the relay after the countdown
                digitalWrite(relayPin, LOW);
                // Turn off the relay LED
                digitalWrite(relayLedPin, LOW);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Relay OFF");
            }
        } else {
            // Display unauthorized message on the LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Unauthorized");
            lcd.setCursor(0, 1);
            lcd.print("Card");
            delay(2000); // Display the message for 2 seconds
            // Reset the LCD message after displaying the unauthorized message
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Tap RFID or");
            lcd.setCursor(0, 1);
            lcd.print("Push Button");
        }

        // Halt the PICC (RFID card) until a new card is present
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    }
    
    // Check if the button to turn off the relay is pressed
    if (digitalRead(offButtonPin) == LOW) {
        // Turn off the relay
        digitalWrite(relayPin, LOW);
        // Turn off the relay LED
        digitalWrite(relayLedPin, LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Relay OFF");
    }

    // Other loop code...
}