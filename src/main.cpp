#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

const int buttonPin = 2;    // the number of the pushbutton pin
const int stopButtonPin = 3; // the number of the stop button pin
const int relayPin = A0;    // the analog pin number where the relay is connected

bool authorized = false;    // flag to check if RFID is authorized
bool relayActive = false;   // flag to track relay activation
unsigned long relayTimer;   // variable to store the start time of relay activation
const unsigned long relayDuration = 10000; // relay activation duration in milliseconds (10 seconds)

#define RST_PIN 9  // Configurable, see typical pin layout above
#define SS_PIN 10  // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// Initialize the LCD, adjust the address (0x27) if necessary
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust the address according to your actual I2C address

void setup() {
  // Initialize serial communications with the PC
  Serial.begin(9600);
  
  // Initialize the relay pin as an output and make sure the relay is OFF initially
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Set relay to HIGH (off)
  
  // Initialize the pushbutton pin and stop button pin as inputs with pullup resistors
  pinMode(buttonPin, INPUT_PULLUP); // Use INPUT_PULLUP to avoid floating state
  pinMode(stopButtonPin, INPUT_PULLUP); // Use INPUT_PULLUP to avoid floating state
  
  // Initialize the SPI bus
  SPI.begin();
  
  // Initialize the MFRC522 RFID module
  mfrc522.PCD_Init();
  Serial.println("Scan an RFID tag or card...");
  
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Scan RFID card");
}

void loop() {
  // Check for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Show UID on serial monitor
    Serial.print("UID tag :");
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    content.toUpperCase();
    
    // Example: Check if the card UID matches a known UID
    if (content.substring(1) == "BB 27 AF 13") {
      authorized = true; // Set authorized flag to true
      Serial.println("Authorized access");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Authorized RFID");

      // Add a 1-second delay before showing the next message
      delay(1000); // Wait for 1 second
      
      // Display "Push the" on the first line and "button to start" on the second line
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Push the");
      lcd.setCursor(0, 1);
      lcd.print("button to start");

    } else {
      Serial.println("Unauthorized access");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unauthorized");
      authorized = false; // Set authorized flag to false
      delay(2000); // Display the message for 2 seconds
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scan RFID card");
    }
    delay(2000); // Display the message for 2 seconds
  }

  // Read the state of the pushbutton and stop button values
  int buttonState = digitalRead(buttonPin);
  int stopButtonState = digitalRead(stopButtonPin);

  // Check if the RFID is authorized and the pushbutton is pressed
  if (authorized && buttonState == HIGH) { // Button pressed is LOW due to INPUT_PULLUP
    if (!relayActive) {
      // Activate the relay
      digitalWrite(relayPin, LOW); // Turn relay on
      relayActive = true;
      relayTimer = millis(); // Record the start time of relay activation
      
      // Display relay activation message on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Relay ON");
      lcd.setCursor(0, 1);
      lcd.print("Time left: ");
    }
  }

  // Check if stop button is pressed
  if (stopButtonState == HIGH) { // Stop button pressed is LOW due to INPUT_PULLUP
    if (relayActive) {
      relayActive = false;
      digitalWrite(relayPin, HIGH); // Turn relay off
      
      // Reset the authorized flag so the user needs to tap the RFID again
      

      // Display relay deactivation message on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Relay why");
      delay(2000); // Display "Relay OFF" for 2 seconds
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scan RFID card");

      authorized = false;
    }
  }

  // Check if relay timer has expired
  if (relayActive && (millis() - relayTimer >= relayDuration)) {
    relayActive = false;
    digitalWrite(relayPin, HIGH); // Turn relay off
    
    // Reset the authorized flag so the user needs to tap the RFID again
    authorized = false;

    // Display relay deactivation message on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Relay OFF");
    delay(2000); // Display "Relay OFF" for 2 seconds
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID card");
  }

  // Update LCD with remaining time if relay is active
  if (relayActive) {
    unsigned long elapsedTime = millis() - relayTimer;
    unsigned long remainingTime = relayDuration - elapsedTime;
    
    lcd.setCursor(11, 1);
    lcd.print(remainingTime / 1000); // Print remaining seconds
  }
}
