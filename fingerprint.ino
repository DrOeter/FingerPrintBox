#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Servo.h>
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
Servo servo;
uint8_t id;
bool servoMove = false;
    
class Finger : public Adafruit_Fingerprint{                                     // Eigene Fingerprint-Klasse abgeleitet von Adafruit_Fingerprint
public:
    Finger(): Adafruit_Fingerprint(&::mySerial){}                               // Konstruktor mit Serial initialisieren

    static void print(const String & str, bool clear = true){                   // Macht Bildschirmausgaben
        if (clear) display.clearDisplay();
        display.println(str);
        display.display();
    }
    
    uint8_t getImageSwitch(uint8_t p){                                          // Gibt Signale aus dem Sensor als Text aus
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Image taken");
                Finger::print("Image taken");
                return p;
            case FINGERPRINT_NOFINGER:
                Serial.println("No finger detected");
                Finger::print("No finger detected");
                return p;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Communication error");
                Finger::print("Communication error");
                return p;
            case FINGERPRINT_IMAGEFAIL:
                Serial.println("Imaging error");
                Finger::print("Imaging error");
                return p;
            default:
                Serial.println("Unknown error");
                Finger::print("Unknown error");
                return p;
        }
    }

    uint8_t getImage2TzSwitch(uint8_t p){                                       // Gibt Signale aus dem Sensor als Text aus
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Converted");
                Finger::print("Converted", false);
                return p;
            case FINGERPRINT_IMAGEMESS:
                Serial.println("Image too messy");
                Finger::print("Image too messy", false);
                return p;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Communication error");
                Finger::print("Communication error", false);
                return p;
            case FINGERPRINT_FEATUREFAIL:
                Serial.println("Could not find fingerprint features");
                Finger::print("Could not find fingerprint features", false);
                return p;
            case FINGERPRINT_INVALIDIMAGE:
                Serial.println("Could not find fingerprint features");
                Finger::print("Could not find fingerprint features", false);
                return p;
            default:
                Serial.println("Unknown error");
                Finger::print("Unknown error", false);
                return p;
        }
    }


    uint8_t getFingerprintID() {                                                // Funktion scannt aufliegenden Finger und guckt ob dieses Bild im Fingerprintsensorspeicher vorhanden ist
        uint8_t p = getImage();
        p = getImageSwitch(p);
        if (p != FINGERPRINT_OK) return p;
        
        p = image2Tz();
        p = getImage2TzSwitch(p);
        if (p != FINGERPRINT_OK) return p;                                      // Dann guckt er ob dieser Scan ein akzeptables Bild geliefert hat

        p = fingerSearch();
        if (p == FINGERPRINT_OK) {                                              // Gibt Signale aus dem Sensor als Text aus
            Serial.println("Match Found!");
            Finger::print("Match Found!", false);
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("Communication error");
            Finger::print("Communication error", false);
            return p;
        } else if (p == FINGERPRINT_NOTFOUND) {
            Serial.println("Did not find a match");
            Finger::print("Did not find a match", false);
            return p;
        } else {
            Serial.println("Unknown error");
            Finger::print("Unknown error", false);
            return p;
        }
        return fingerID;
    }

    int getFingerprintEnroll() {                                                // Funktion um einen neuen Fingerabdruck zu speichern
        int p = -1;
        Serial.print("Place your Finger");                                      // Er erwartet, dass man am anfang einmal den Finger auflegt
        Finger::print("Place your Finger");
        while (p != FINGERPRINT_OK) {                                           // Dann wartet er bis der Finger wieder weggenommen wurde
            p = getImage();
            p = getImageSwitch(p);        
        }
        
        p = image2Tz(1);
        p = getImage2TzSwitch(p);
        if (p != FINGERPRINT_OK) return p;                                      // Dann guckt er ob dieser Scan ein akzeptables Bild geliefert hat
    
        Serial.println("Remove finger");
        Finger::print("Remove finger");                                         // Wenn ja fordert er den Finger wegzunehmen 
        delay(2000);
        p = 0;
        while (p != FINGERPRINT_NOFINGER) {                                     // Warten bis der Finger weggenommen ist
            p = getImage();
        }
        Serial.print("ID "); Serial.println(id);
        p = -1;
        Serial.println("Place same finger again");
        Finger::print("Place same finger again");                               // Dann erwartet er den selben Finger nochmal
        while (p != FINGERPRINT_OK) {                                               
            p = getImage();
            p = getImageSwitch(p);
        }
    
        p = image2Tz(2);
        p = getImage2TzSwitch(p);
        if (p != FINGERPRINT_OK) return p;                                      // Dann guckt er ob dieser Scan ein akzeptables Bild geliefert hat
    
        Serial.print("Creating model for #");  Serial.println(id);
    
        p = createModel();                                                      // Dann formt er das Scanergebnis in ein speicherbares Modell um
        if (p == FINGERPRINT_OK) {                                              // Gibt Signale aus dem Sensor als Text aus
            Serial.println("Prints matched!");
            Finger::print("Prints matched!", false);
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("Communication error");
            Finger::print("Communication error", false);
            return p;
        } else if (p == FINGERPRINT_ENROLLMISMATCH) {
            Serial.println("Fingerprints did not match");
            Finger::print("Fingerprints did not match", false);
            return p;
        } else {
            Serial.println("Unknown error");
            Finger::print("Unknown error", false);
            return p;
        }
    
        Serial.print("ID "); Serial.println(id);
        p = storeModel(id);                                                     // Speichert Modell in Fingerprintspeicher unter im loop gesetzter ID
        if (p == FINGERPRINT_OK) {                                              // Gibt Signale aus dem Sensor als Text aus
            Serial.println("Stored!");
            Finger::print("Stored!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
            Serial.println("Communication error");
            Finger::print("Communication error");
            return p;
        } else if (p == FINGERPRINT_BADLOCATION) {
            Serial.println("Could not store in that location");
            Finger::print("Could not store in that location");
            return p;
        } else if (p == FINGERPRINT_FLASHERR) {
            Serial.println("Error writing to flash");
            Finger::print("Error writing to flash");
            return p;
        } else {
            Serial.println("Unknown error");
            Finger::print("Unknown error");
            return p;
        }
        return ENROLL_SUCCESS;
    }
};

Finger finger;

void setup()
{
    Serial.begin(9600);                                                         // Startet Serielle ausgabe
    while (!Serial);                                                            
    
    display.begin();                                                            // Startet Bildschirmausgabe
    display.clearDisplay();                                                     // Setzt Bildschirmeinstellungen
    display.setCursor(0, 0);
    display.setTextSize(0.5);

    servo.attach(9);                                                            // Setzt Servo-Signal-Pin auf PIN 9
    servo.write(70);                                                            // Verschließt standardmäßig die Box
  
    delay(100);
    finger.begin(57600);                                                        // Startet Sensorkommunikation
    delay(5);

    while(true){                                                                // Sucht den Sensor
        if (finger.verifyPassword()) {
            Serial.println("Found");
            Finger::print("Found");
            break;
        } else {
            Serial.println("Not Found");
            Finger::print("Not Found");
        }
        delay(100);
    }
    finger.getParameters();
    finger.getTemplateCount();
}

#define NORMAL_USAGE
//#define RESET_EEPROM                                                          // Hiermit kann man den Zähler der Fingerabdrücke zurücksetzen
    
void loop(){
#ifdef NORMAL_USAGE
    finger.fingerID = 0;                                                        // Reseted fingerID
    finger.getFingerprintID();                                                  // Wartet auf Fingerabdruck
                                                                                // Wenn fingerID != authorisierungsFingerID wird die Box geöffnet/geschlossen
    if(finger.fingerID != 0 && finger.fingerID != 126 && finger.fingerID != 127 && finger.confidence > 80){     
        String text = String("Found ID #") + String(finger.fingerID);           
        if(!servoMove){                                                         // Schließt Box auf
            // unlock
            Serial.println("unlocked");
            Finger::print(text + "\nunlocked", false);
            servo.write(140);                                                   // Setzt Servo auf 140 Grad
            servoMove = true; 
        }
        else if(servoMove){                                                     // Schließt Box zu
            // lock
            Serial.println("locked");
            Finger::print(text + "\nlocked", false);
            servo.write(70);                                                    // Setzt Servo auf 70 Grad
            servoMove = false;
        }
    }
                                                                                // Wenn fingerID != authorisierungsFingerID wird ein neuer Fingerabdruck abgespeichert
    else if((finger.fingerID == 126 || finger.fingerID == 127 ) && finger.confidence > 80){
        byte IDCount = EEPROM.read(0);                                          // Liest Zählerwert aus EEPROM-Speicher
        IDCount++;                                              
        if(IDCount == 126) IDCount+=2;                                          // Authorisierungsfingerabdrücke dürfen nicht überschrieben werden
        else if(IDCount == 127) IDCount++;
        id = IDCount;                                                           // gibt ID an globale variable
        String text = String("Enrolling ID #") + id + "\nSetting up. Please Wait....";
        
        Serial.println(text);
        Finger::print(text);
        
        delay(3000);

        while (finger.getFingerprintEnroll() != ENROLL_SUCCESS){                // Wiederholt Einscanprozess solange bis er erolgreich war
            Serial.println(text);
            Finger::print(text);
            delay(3000);
        }
        EEPROM.write(0, IDCount);                                               // Schreibt erhöhten IDZähler

        Serial.println("Success!\nRemove Your Finger");
        Finger::print("Success!\nRemove Your Finger");
    }
    int p = 0;
    while (p != FINGERPRINT_NOFINGER) {                                         // Wartet bis kein Finger aufliegt
        p = finger.getImage();
    }
    delay(100);
#endif
#ifdef RESET_EEPROM
    EEPROM.write(0, 1);
#endif 
}
