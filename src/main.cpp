
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

const int buttonPin = 7;    // the number of the pushbutton pin
const int relayPin = A0;    // the analog pin number where relay is connected

bool authorized = false;    // flag to check if RFID is authorized
bool relayActive = true;   // flag to track relay activation
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
  
  // Initialize the relay pin as an output
  pinMode(relayPin, OUTPUT);
  
  // Initialize the pushbutton pin as an input
  pinMode(buttonPin, INPUT);
  
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
      Serial.println("Authorized access");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Authorized");
      authorized = true;
    } else {
      Serial.println("Unauthorized access");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unauthorized");
      authorized = false;
    }
    delay(2000); // Display the message for 2 seconds
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID card");
  }

  // Read the state of the pushbutton value
  int buttonState = digitalRead(buttonPin);

  // Check if the RFID is authorized and the pushbutton is pressed
  if (authorized && buttonState == HIGH && !relayActive) {
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

  // Check if relay timer has expired
  if (relayActive && (millis() - relayTimer >= relayDuration)) {
    relayActive = false;
    digitalWrite(relayPin, HIGH); // Turn relay off
    
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
