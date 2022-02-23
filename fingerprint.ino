/***************************************************
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h> 
#define ENROLL_SUCCESS 8192


//#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 6);
uint8_t id;
bool servo = false;
    
class Finger : public Adafruit_Fingerprint{
public:
    Finger(): Adafruit_Fingerprint(&::mySerial){}

    static void print(const char *str){
        display.clearDisplay();
        display.print(str);
        display.display();
    }

    uint8_t getFingerprintID() {
        uint8_t p = getImage();
        switch (p) {
            case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
            case FINGERPRINT_NOFINGER:
            Serial.println("No finger detected");
            return p;
            case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
            case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            return p;
            default:
            Serial.println("Unknown error");
            return p;
        }
        
        p = image2Tz();
        switch (p) {
            case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
            case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
            case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
            case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
            case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
            default:
            Serial.println("Unknown error");
            return p;
        }

        p = fingerSearch();
        if (p == FINGERPRINT_OK) {
            Serial.println("Found a print match!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("Communication error");
            return p;
        } else if (p == FINGERPRINT_NOTFOUND) {
            Serial.println("Did not find a match");
            return p;
        } else {
            Serial.println("Unknown error");
            return p;
        }
        return fingerID;
    }

    int getFingerprintEnroll() {
        int p = -1;
        Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
        while (p != FINGERPRINT_OK) {
            p = getImage();
            switch (p) {
                case FINGERPRINT_OK:
                    Serial.println("Image taken");
                break;
                case FINGERPRINT_NOFINGER:
                    Serial.println(".");
                break;
                case FINGERPRINT_PACKETRECIEVEERR:
                    Serial.println("Communication error");
                break;
                case FINGERPRINT_IMAGEFAIL:
                    Serial.println("Imaging error");
                break;
                default:
                    Serial.println("Unknown error");
                break;
            }
        }
        
        p = image2Tz(1);
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Image converted");
            break;
            case FINGERPRINT_IMAGEMESS:
                Serial.println("Image too messy");
                return p;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Communication error");
                return p;
            case FINGERPRINT_FEATUREFAIL:
                Serial.println("Could not find fingerprint features");
                return p;
            case FINGERPRINT_INVALIDIMAGE:
                Serial.println("Could not find fingerprint features");
                return p;
            default:
                Serial.println("Unknown error");
                return p;
        }
    
        Serial.println("Remove finger");
        delay(2000);
        p = 0;
        while (p != FINGERPRINT_NOFINGER) {
            p = getImage();
        }
        Serial.print("ID "); Serial.println(id);
        p = -1;
        Serial.println("Place same finger again");
        while (p != FINGERPRINT_OK) {
            p = getImage();
            switch (p) {
                case FINGERPRINT_OK:
                    Serial.println("Image taken");
                break;
                case FINGERPRINT_NOFINGER:
                    Serial.print(".");
                break;
                case FINGERPRINT_PACKETRECIEVEERR:
                    Serial.println("Communication error");
                break;
                case FINGERPRINT_IMAGEFAIL:
                    Serial.println("Imaging error");
                break;
                default:
                    Serial.println("Unknown error");
                break;
            }
        }
    
        p = image2Tz(2);
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Image converted");
            break;
            case FINGERPRINT_IMAGEMESS:
                Serial.println("Image too messy");
            return p;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Communication error");
            return p;
            case FINGERPRINT_FEATUREFAIL:
                Serial.println("Could not find fingerprint features");
            return p;
            case FINGERPRINT_INVALIDIMAGE:
                Serial.println("Could not find fingerprint features");
            return p;
            default:
                Serial.println("Unknown error");
            return p;
        }
    
        Serial.print("Creating model for #");  Serial.println(id);
    
        p = createModel();
        if (p == FINGERPRINT_OK) {
            Serial.println("Prints matched!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("Communication error");
            return p;
        } else if (p == FINGERPRINT_ENROLLMISMATCH) {
            Serial.println("Fingerprints did not match");
            return p;
        } else {
            Serial.println("Unknown error");
            return p;
        }
    
        Serial.print("ID "); Serial.println(id);
        p = storeModel(id);
        if (p == FINGERPRINT_OK) {
            Serial.println("Stored!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("Communication error");
            return p;
        } else if (p == FINGERPRINT_BADLOCATION) {
            Serial.println("Could not store in that location");
            return p;
        } else if (p == FINGERPRINT_FLASHERR) {
            Serial.println("Error writing to flash");
            return p;
        } else {
            Serial.println("Unknown error");
            return p;
        }
        return ENROLL_SUCCESS;
    }
};

Finger finger;

void setup()
{
    Serial.begin(9600);
    while (!Serial);  
    
    display.begin();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("yee");
    display.display();
  
    delay(100);
    Serial.println("\n\nAdafruit finger detect test");
    finger.begin(57600);
    delay(5);

    while(true){
        if (finger.verifyPassword()) {
            Serial.println("Found fingerprint sensor!");
            break;
        } else {
            Serial.println("Did not find fingerprint sensor :(");
        }
        delay(100);
    }
    
    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
    Serial.print(F("Security level: ")); Serial.println(finger.security_level);
    Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

    finger.getTemplateCount();

    if (finger.templateCount == 0) {
        Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    }
    else {
        Serial.println("Waiting for valid finger...");
        Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
    }
}

#define NORMAL_USAGE
//#define RESET_EEPROM
    
void loop(){
#ifdef NORMAL_USAGE
    finger.fingerID = 0;
    finger.getFingerprintID();
    if(finger.fingerID != 0 && finger.fingerID != 126 && finger.fingerID != 127 && finger.confidence > 80){
        Serial.print("Found ID #"); Serial.print(finger.fingerID);
        Serial.print(" with confidence of "); Serial.println(finger.confidence);
        if(!servo){
            // unlock
            Serial.println("unlocked");
            Finger::print("unlocked");
            servo = true; 
        }
        else if(servo){
            // lock
            Serial.println("locked");
            Finger::print("locked");
            servo = false;
        }
    }
    else if((finger.fingerID == 126 || finger.fingerID == 127 ) && finger.confidence > 80){
        //Serial.println("Ready to enroll a fingerprint!");
        //Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");

        byte IDCount = EEPROM.read(0);
        IDCount++;
        id = IDCount;
        
        Serial.print("Enrolling ID #");
        Serial.println(id);
        Serial.println("Setting up. Please Wait....");
        delay(3000);

        while (finger.getFingerprintEnroll() != ENROLL_SUCCESS){
            Serial.println("Setting up. Please Wait again....");
            delay(3000);
        }
        EEPROM.write(0, IDCount);

        Serial.println("Please Remove Your Finger!");
        delay(3000);
    }
#endif
#ifdef RESET_EEPROM
    EEPROM.write(0, 1);
#endif 
    delay(100);
}
