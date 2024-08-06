#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>

/* 
 * Sample program for ESP32 acting as a Bluetooth keyboard
 * 
 * Copyright (c) 2019 Manuel Bl
 * 
 * Licensed under MIT License
 * https://opensource.org/licenses/MIT
 */



#define US_KEYBOARD 1
#include <Arduino.h>
#include "NimBLEDevice.h"
#include "NimBLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

// Change the below values if desired

#define DEVICE_NAME "KTM SDKGT Buttons"

// Buttons

#define BUTTON_PLAY_PREV D0
#define BUTTON_PLAY_NEXT D2
#define BUTTON_PLAY_PAUSE D3
#define BUTTON_VOL_UP D4
#define BUTTON_VOL_DOWN D1


// Media Key Values

#define MEDIA_KEY_PREV 0xB6
#define MEDIA_KEY_NEXT 0xB5
#define MEDIA_KEY_PLAY_PAUSE 0xCD
#define MEDIA_KEY_VOL_UP 0xE9
#define MEDIA_KEY_VOL_DOWN 0xEA



// Forward declarations
void bluetoothTask(void*);
void sendKey(int mediaButtton );
bool isBleConnected = false;

void setup() {
    Serial.begin(115200);

    // configure pin for buttons
    pinMode(BUTTON_PLAY_PREV, INPUT_PULLUP);
    pinMode(BUTTON_PLAY_NEXT, INPUT_PULLUP);
    pinMode(BUTTON_PLAY_PAUSE, INPUT_PULLUP);
    pinMode(BUTTON_VOL_UP, INPUT_PULLUP);
    pinMode(BUTTON_VOL_DOWN, INPUT_PULLUP);

    // start Bluetooth task
    xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);

    Serial.print("BleStart: ");
    Serial.println();

}

void loop() {
  
    if(isBleConnected) {

      if(digitalRead(BUTTON_PLAY_PREV) == LOW) {
        Serial.println("PREV Pressed");
        sendKey(MEDIA_KEY_PREV);
        while(digitalRead(BUTTON_PLAY_PREV) == LOW) {
          Serial.println("PREV Holding");
          delay(20);
        }
        Serial.println("PREV released");
      }

      if(digitalRead(BUTTON_PLAY_NEXT) == LOW) {
        Serial.println("NEXT Pressed");
        sendKey(MEDIA_KEY_NEXT);
        while(digitalRead(BUTTON_PLAY_NEXT) == LOW) {
          Serial.println("NEXT Holding");
          delay(20);
        }
        Serial.println("NEXT released");
      }

      if(digitalRead(BUTTON_VOL_UP) == LOW) {
        Serial.println("VOL UP Pressed");
        sendKey(MEDIA_KEY_VOL_UP);
        while(digitalRead(BUTTON_VOL_UP) == LOW) {
          Serial.println("VOL UP Holding");
          delay(20);
        }
        Serial.println("VOL UP released");
      }

      if(digitalRead(BUTTON_VOL_DOWN) == LOW) {
        Serial.println("VOL DOWN Pressed");
        sendKey(MEDIA_KEY_VOL_DOWN);
        while(digitalRead(BUTTON_VOL_DOWN) == LOW) {
          Serial.println("VOL DOWN Holding");
          delay(20);
        }
        Serial.println("VOL DOWN released");
      }


      if(digitalRead(BUTTON_PLAY_PAUSE) == LOW) {
        Serial.println("PLAY/PAUSE Pressed");
        sendKey(MEDIA_KEY_PLAY_PAUSE);
        while(digitalRead(BUTTON_PLAY_PAUSE) == LOW) {
          Serial.println("PLAY/PAUSE Holding");
          delay(20);
        }
        Serial.println("PLAY/PAUSE released");
      }


    }

    delay(100);
}

// Message (report) sent when a key is pressed or released
struct InputReport {
    uint8_t modifiers;       // bitmask: CTRL = 1, SHIFT = 2, ALT = 4
    uint8_t reserved;        // must be 0
    uint8_t pressedKeys[6];  // up to six concurrenlty pressed keys
};

struct ConsumerReport {
    uint16_t pressedKey;     // two byte keycode
};

// Message (report) received when an LED's state changed
struct OutputReport {
    uint8_t leds;            // bitmask: num lock = 1, caps lock = 2, scroll lock = 4, compose = 8, kana = 16
};

// The report map describes the HID device (a keyboard in this case) and
// the messages (reports in HID terms) sent and received.
static const uint8_t REPORT_MAP[] = {
//
//ReportId = 1 (Keyboard Conrol Input)
    USAGE_PAGE(1),      0x01,       // Generic Desktop Controls
    USAGE(1),           0x06,       // Keyboard
    COLLECTION(1),      0x01,       // Application
    REPORT_ID(1),       0x01,       //   Report ID (1)
    USAGE_PAGE(1),      0x07,       //   Keyboard/Keypad
    USAGE_MINIMUM(1),   0xE0,       //   Keyboard Left Control
    USAGE_MAXIMUM(1),   0xE7,       //   Keyboard Right Control
    LOGICAL_MINIMUM(1), 0x00,       //   Each bit is either 0 or 1
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_COUNT(1),    0x08,       //   8 bits for the modifier keys
    REPORT_SIZE(1),     0x01,       
    HIDINPUT(1),        0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   1 bytes (for up to 1 concurrently pressed keys)
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0xE7,       //   101 keys
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0xE7,
    HIDINPUT(1),        0x00,       //   Data, Array, Abs
    END_COLLECTION(0),              // End application collection
//
//Report Reference Id = 2 (Media Control Input)
    USAGE_PAGE(1), 0x0C,            // Usage Page (Consumer)
    USAGE(1), 0x01,                 // Usage (Consumer Control)
    COLLECTION(1), 0x01,            // Collection (Application)
    REPORT_ID(1), 0x02,             //   Report ID (2)
    REPORT_SIZE(1), 0x10,           //   Report Size (16)
    REPORT_COUNT(1), 0x01,          //   Report Count (1)
    LOGICAL_MINIMUM(1), 0x01,       //   Logical Minimum (1)
    LOGICAL_MAXIMUM(2), 0xff, 0x07, //   Logical Maximum (2047)
    USAGE_MINIMUM(1), 0x01,         //   Usage Minimum (Consumer Control)
    USAGE_MAXIMUM(2), 0xff, 0x07,   //   Usage Maximum (0x07FF)
    HIDINPUT(1), 0x00,              //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    END_COLLECTION(0),              // End Collection
//
//Report Reference Id = 3 (Mouse Control Input)
    USAGE_PAGE(1), 0x01,            // Usage Page (Generic Desktop Ctrls)
    USAGE(1), 0x02,                 // Usage (Mouse)
    COLLECTION(1), 0x01,            // Collection (Application)
    REPORT_ID(1), 0x03,             //   Report ID (3)
    USAGE(1), 0x01,                 //   Usage (Pointer)
    COLLECTION(1), 0x00,            //   Collection (Physical)
    USAGE_PAGE(1), 0x09,            //     Usage Page (Button)
    USAGE_MINIMUM(1), 0x01,         //     Usage Minimum (0x01)
    USAGE_MAXIMUM(1), 0x03,         //     Usage Maximum (0x03)
    LOGICAL_MINIMUM(1), 0x00,       //     Logical Minimum (0)
    LOGICAL_MAXIMUM(1), 0x01,       //     Logical Maximum (1)
    REPORT_COUNT(1), 0x03,          //     Report Count (3)
    REPORT_SIZE(1), 0x01,           //     Report Size (1)
    HIDINPUT(1), 0x02,              //     Input (Data,Var,Abs,No Wrap,Linear,...)
    REPORT_COUNT(1), 0x01,          //     Report Count (1)
    REPORT_SIZE(1), 0x05,           //     Report Size (5)
    HIDINPUT(1), 0x03,              //     Input (Const,Var,Abs,No Wrap,Linear,...)
    USAGE_PAGE(1), 0x01,            //     Usage Page (Generic Desktop Ctrls)
    USAGE(1), 0x30,                 //     Usage (X)
    USAGE(1), 0x31,                 //     Usage (Y)
    LOGICAL_MINIMUM(1), 0x81,       //     Logical Minimum (-127)
    LOGICAL_MAXIMUM(1), 0x7F,       //     Logical Maximum (127)
    REPORT_SIZE(1), 0x08,           //     Report Size (8)
    REPORT_COUNT(1), 0x02,          //     Report Count (2)
    HIDINPUT(1), 0x06,              //     Input (Data,Var,Rel,No Wrap,Linear,...)
    END_COLLECTION(0),              //   End Collection
    END_COLLECTION(0)               // End Collection
};

NimBLEHIDDevice* hid;
NimBLECharacteristic* input;
NimBLECharacteristic* output;

const InputReport NO_KEY_PRESSED = { };

/*
 * Callbacks related to BLE connection
 */
class BleKeyboardCallbacks : public BLEServerCallbacks {

    void onConnect(BLEServer* server) {
        isBleConnected = true;

        // Allow notifications for characteristics
        //BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(NimBLEUUID((uint16_t)0x2902));
        //cccDesc->setNotifications(true);

        Serial.println("Client has connected");
    }

    void onDisconnect(BLEServer* server) {
        isBleConnected = false;

        // Disallow notifications for characteristics
        //BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(NimBLEUUID((uint16_t)0x2902));
        //cccDesc->setNotifications(false);

        Serial.println("Client has disconnected");
    }
};

/*
 * Called when the client (computer, smart phone) wants to turn on or off
 * the LEDs in the keyboard.
 * 
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
 
 class OutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) {
        //OutputReport* report = (OutputReport*) characteristic->getValue();
        std::string value = characteristic->getValue();
        OutputReport* report = (OutputReport*)value.data();
//Alternatively use the getValue template:
//my_struct_t myStruct = pChr->getValue<my_struct_t>();

        
        Serial.print("LED state: ");
        Serial.print((int) report->leds);
        Serial.println();
    }
};

void bluetoothTask(void*) {

    // initialize the device
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEServer* server = NimBLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks());

    // create an HID device
    hid = new NimBLEHIDDevice(server);
    input = hid->inputReport(2); // report ID
    output = hid->outputReport(1); // report ID
    output->setCallbacks(new OutputCallbacks());

    // set manufacturer name
    hid->manufacturer()->setValue("Maker Community");
    // set USB vendor and product ID
    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    // information about HID device: device is not localized, device can be connected
    hid->hidInfo(0x00, 0x02);

    // Security: device requires bonding
    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);

    // set report map
    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    hid->startServices();

    // set battery level to 100%
    hid->setBatteryLevel(100);

    // advertise the services
    NimBLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->addServiceUUID(hid->deviceInfo()->getUUID());
    advertising->addServiceUUID(hid->batteryService()->getUUID());
    advertising->start();

    Serial.println("BLE ready");
    delay(portMAX_DELAY);
};

void sendKey(int mediaButton) {

    ConsumerReport report = {
        .pressedKey = mediaButton
    //    .pressedKey = 0x46 //FireTV Back
    };


    // send the input report
    input->setValue((uint8_t*)&report, sizeof(report));
    input->notify();

    delay(5);

    // release all keys between two characters; otherwise two identical
    // consecutive characters are treated as just one key press
    input->setValue((uint8_t*)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
    input->notify();

    printf("Sent\n");

    delay(5);
}
