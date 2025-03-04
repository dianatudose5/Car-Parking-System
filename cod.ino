#include <IRremote.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define IR_RECEIVE_PIN 8
#define PIN_IR_INTRARE 10
#define PIN_IR_IESIRE 9

LiquidCrystal_I2C lcd(0x27, 16, 2);
bool ir_1_trigger = false;
bool ir_2_trigger = false;
bool updateDisplay = true;
int masiniInParcare = 0;
unsigned long timp_buzzer = 0;
int timpIntrareServo = 0;
int timpIesireServo = 0;
int verificareStatus = 0;
unsigned long lastEcranUpdate = 0;
int timpReset = 0;
unsigned long lastTick_foc = 0;
Servo intrare_servo;
Servo iesire_servo;

int acceptedCommands[4] = {69, 70, 71, 68};

void setup() {
    Wire.begin();
    lcd.begin();
    lcd.backlight();
    Serial.begin(9600);
    IrReceiver.begin(IR_RECEIVE_PIN);
    intrare_servo.attach(17);
    iesire_servo.attach(16);
    pinMode(A2, INPUT);
    pinMode(3, OUTPUT);
}

void loop() {
    irLogica();
    logicaIRremote();
    if (millis() - lastEcranUpdate > 1000) {
        updateScreenContent();
        if (timpReset > 0) {
            timpReset--;
        }
        if (timpReset == 0 && verificareStatus != 1) {
            verificareStatus = 0;
        }
        if (timpIntrareServo > 0) {
            intrare_servo.write(180);
            timpIntrareServo--;
        }
        if (timpIesireServo > 0) {
            iesire_servo.write(180);
            timpIesireServo--;
        }
        if (timpIntrareServo == 0) {
            intrare_servo.write(90);
        }
        if (timpIesireServo == 0) {
            iesire_servo.write(90);
        }
        lastEcranUpdate = millis();
    }
    if (millis() > lastTick_foc) {
        logicaFoc();
        lastTick_foc = millis() + 3000;
    }
    if (millis() < timp_buzzer)
        digitalWrite(3, HIGH);
    else
        digitalWrite(3, LOW);
}

void logicaIRremote() {
    if (IrReceiver.decode()) {
        int command = IrReceiver.decodedIRData.command;
        Serial.print("Received IR command: ");
        Serial.println(command);
        if (verificareStatus == 1) {
            if (isIntInArray(command, acceptedCommands, 4)) {
                verificareStatus = 2;
                timpReset = 3;
                if (masiniInParcare < 4) {
                    masiniInParcare++;
                    timpIntrareServo = 5;
                }
            } else {
                verificareStatus = 3;
                timpReset = 3;
            }
        }
        IrReceiver.resume();
    }
}

void updateScreenContent() {
    lcd.clear();
    String line_1 = "Masini: " + String(masiniInParcare) + "/4";
    String urgenta = "";
    String line_2 = "";
    switch (verificareStatus) {
        case 1:
            line_2 = "Apasati tasta";
            break;
        case 2:
            line_2 = "Acces permis!";
            break;
        case 3:
            line_2 = "Acces respins!";
            break;
        default:
            break;
    }
    String line_3 = "";
    String line_4 = "";

    if (millis() < timp_buzzer)
       line_4 = "Parasiti park.";
   
   
    if (masiniInParcare == 4) {
        line_4 = "Parcare plina!";
    }
    lcd.setCursor(0, 0);
    lcd.print(line_1);
    lcd.setCursor(0, 1);
    lcd.print(line_2);
    lcd.setCursor(0, 2);
    lcd.print(line_3);
    lcd.setCursor(0, 3);
    lcd.print(line_4);
}

void irLogica() {
    if (!digitalRead(PIN_IR_INTRARE) && !ir_1_trigger) {
        ir_1_trigger = true;
        if (masiniInParcare < 4) {
            verificareStatus = 1;
        }
    }
    if (!digitalRead(PIN_IR_IESIRE) && !ir_2_trigger) {
        ir_2_trigger = true;
        if (masiniInParcare != 0) {
            masiniInParcare--;
            timpIesireServo = 5;
            Serial.print(masiniInParcare);
            Serial.println("O masina a iesit!");
        }
        delay(50);
    }
    if (digitalRead(PIN_IR_INTRARE) && ir_1_trigger) {
        ir_1_trigger = false;
    }
    if (digitalRead(PIN_IR_IESIRE) && ir_2_trigger) {
        ir_2_trigger = false;
    }
    delay(5);
}

bool isIntInArray(int arg1, int* arg2, int size) {
    for (int i = 0; i < size; i++) {
        if (arg1 == arg2[i]) return true;
    }
    return false;
}

void logicaFoc() {
    if (!digitalRead(28)) {
        timp_buzzer = millis() + 10000;
        timpIesireServo = 10;
        timpIntrareServo = 10;
        Serial.println("Urgenta!");
    }
}
