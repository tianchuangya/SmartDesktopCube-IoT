// #include <Wire.h>
// #include <Arduino.h>
// void setup() {
//     Serial.begin(115200);
//     Wire.begin(2, 1);  // 你的SDA、SCL
    
//     Serial.println("I2C 扫描中...");
//     byte error, address;
//     for (address = 1; address < 127; address++) {
//         Wire.beginTransmission(address);
//         error = Wire.endTransmission();
//         if (error == 0) {
//             Serial.print("找到设备：0x");
//             Serial.println(address, HEX);
//         }
//     }
//     Serial.println("扫描完成");
// }

// void loop() {}