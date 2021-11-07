
// 1. 初期定義
// ① BLE通信
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "00001818-0000-1000-8000-00805F9B34FB" //2201
#define CHARACTERISTIC_UUID "00002A63-0000-1000-8000-00805F9B34FB" //2256

// ① BLE通信
BLEServer* pServer                 = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected    = false;
bool oldDeviceConnected = false;
int a;
uint8_t buff[18];
uint32_t value = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

short power, power_add;
unsigned short revolutions = 0;
unsigned short timestamp = 0;
unsigned short flags = 0x20;
byte sensorlocation = 0x0D;
float interval, preInterval, interval_1, adj_time=0.0f, adj_time_p=0.0f;

// ２. 初期読み込み関数
void setup() {
  Serial.begin(115200);
  delay(10);

// ① BLE通信
// Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer()           ;
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
     pCharacteristic -> addDescriptor(new BLE2902());
  // Start the service
     pService->start();

  // Start advertising
     BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
     pAdvertising -> addServiceUUID(SERVICE_UUID);
     pAdvertising -> setScanResponse(false);
     pAdvertising -> setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
     BLEDevice::startAdvertising();
     Serial.println("Waiting a client connection to notify...");

}


// 3. ループ関数
void loop() {
        interval = millis() - preInterval; 
     preInterval = millis();

    // ① BLE通信  // notify changed value
    if (deviceConnected) {

        adj_time += interval;

        if(adj_time > 200){     
            power_add  = 0;
            adj_time   = 0;
        }else{
            power_add += 1;
        }

          power = 100 + power_add;
        
        buff[0] = flags & 0xff;
        buff[1] = (flags >> 8) & 0xff;
        buff[2] = power & 0xff;
        buff[3] = (power >> 8) & 0xff;
        buff[4] = revolutions & 0xff;
        buff[5] = (revolutions >> 8) & 0xff;
        buff[6] = timestamp & 0xff;
        buff[7] = (timestamp >> 8) & 0xff;
        
        //bleBuffer[0] = flags & 0xff;
        //bleBuffer[1] = (flags >> 8) & 0xff;
        //bleBuffer[2] = power & 0xff;
        //bleBuffer[3] = (power >> 8) & 0xff;
        //bleBuffer[4] = revolutions & 0xff;
        //bleBuffer[5] = (revolutions >> 8) & 0xff;
        //bleBuffer[6] = timestamp & 0xff;
        //bleBuffer[7] = (timestamp >> 8) & 0xff;


        pCharacteristic->setValue( buff, sizeof(buff) );
        pCharacteristic->notify();
        value++;
        delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    
    }
    
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

    Serial.println(interval);// interval
}
