/* ******************************************************************************************************
 *
 *       ScioSense ENS190
 *      ===================
 *      https://www.sciosense.com/ens190/
 *
 *       This sample code complements the more advanced libraries available on our GitHub pages 
 *          https://github.com/sciosense/ens190-arduino 
 *          https://github.com/sciosense/ens190-embedded
 *     Originally designed for use in the Arduino environment, this code can be easily 
 *     adapted to other platforms due to its simplicity and lack of external library dependencies.
 *     You just need to ensure the code runs on a system with a second serial port—either software- 
 *     or hardware-based—and adjust it accordingly
 *
 *      © ScioSense B.V.  tr 2025     V1.0
 * ******************************************************************************************************/

#include <SoftwareSerial.h>  // make sure to use proper library in case you haven't got a second UART available

// =======================================================
// Command definitions for CO2 sensor
// =======================================================
uint8_t instrCO2[] = { 0x1A, 0x01, 0x01, 0xE4 };                                     // Start CO2 measurement
uint8_t instrSetABC[] = { 0x1A, 0x03, 0x10, 0x00, 0x07, 0xC7 };                      // Set automatic baseline correction to 7 days
uint8_t instrAlarmThreshold[] = { 0x1A, 0x05, 0x13, 0xFF, 0xFF, 0xFF, 0xFF, 0xD2 };  // Set/read alarm threshold
uint8_t instrSetBL[] = { 0x1A, 0x03, 0x18, 0x01, 0x90, 0x3A };                       // Set baseline to 400 ppm
uint8_t instrSetBaud[] = { 0x1A, 0x02, 0x19, 0x00, 0xCB };                           // Set baud rate
uint8_t instrSN[] = { 0x1A, 0x01, 0x0E, 0xD7 };                                      // Production date and serial number
uint8_t instrFW[] = { 0x1A, 0x01, 0x0F, 0xD6 };                                      // Read firmware

// =======================================================
// Configure serial interface
// =======================================================
SoftwareSerial mySerial(11, 12);  // RX, TX



// =======================================================
// other defintions
// =======================================================
float lastMeasurement = 0;
int measurementDelay = 5;  // deleay between measurements in second

bool enableMeasurement = 1;


// =======================================================
//        S E T U P
// =======================================================
void setup() {


    Serial.begin(9600);    // set main UART for communication to PC
    mySerial.begin(9600);  // set second UART, here software UART

    Serial.println(__FILE__);
    Serial.print(__DATE__);
    Serial.print("\t");
    Serial.println(__TIME__);

    
    // show Menue for Serial control
    printMenue();
    
    
    // get serial number
    //getSN();

    //get firmware number
    //getFW();

    // set new baseline
    //Serial.print(setBaseline(400));

    // program threshold limits to upper and lower limits (hysteresis)
    //setAlarm(2000,600);

    // read programmed threshold limits
    // uint16_t alarmValue[2];
    // getAlarm(alarmValue);
    // Serial.print("\nAlarm threshold upper: ");
    // Serial.print(alarmValue[0]);
    // Serial.print("\nAlarm treshold lower: ");
    // Serial.print(alarmValue[1]);

    // set ABC to xx hours
    //setABC(24);

    // set new baudrate within the sensor.
    // !! make sure the second UART is set accordingly!
    //Serial.println(setBaudrate(00));

    Serial.print("\n");
}

// =======================================================
//      L O O P
// =======================================================
void loop() {

    runCommunication();
    if (((millis() - lastMeasurement) > (measurementDelay * 1000)) & enableMeasurement) {
        Serial.print("CO2: ");
        Serial.println(getCO2());
        lastMeasurement = millis();
    }
}

// =======================================================
// Retrieve CO2 value
// =======================================================
int getCO2() {
    int expected_bytes = 8;
    byte* response = send_and_receive(instrCO2, expected_bytes, 4);

    if (response != nullptr) {
        if (response[7] == calculateChecksum(response, expected_bytes - 1)) {
            return response[4] + 256 * response[3];
        }
    }
    return -1;
}

// =======================================================
// Set new baseline
// =======================================================
uint8_t setBaseline(uint16_t newBaseline) {
    int expected_bytes = 4;
    instrSetBL[3] = newBaseline / 256;
    instrSetBL[4] = newBaseline % 256;
    instrSetBL[5] = calculateChecksum(instrSetBL, 5);

    byte* response = send_and_receive(instrSetBL, expected_bytes, 6);
    if (response != nullptr) {
        if (response[2] == 0x18 && response[3] == 0xCD) {
            Serial.print("New baseline set to ");
            Serial.print(newBaseline);
            Serial.println(" ppm");
            return 1;
        }
    } else {
        Serial.println("\nNot enough data received");
    }
    return -1;
}


// =======================================================
// Set automatic baseline correction
// =======================================================
uint8_t setABC(uint16_t valueHours) {
    int expected_bytes = 6;
    instrSetABC[3] = valueHours / 256;
    instrSetABC[4] = valueHours % 256;
    instrSetABC[5] = calculateChecksum(instrSetABC, 5);

    mySerial.write(instrSetABC, 6);
    delay(100);  // Short wait for response

    byte* response = send_and_receive(instrSetABC, expected_bytes, 6);
    if (response != nullptr) {
        if (response[5] == calculateChecksum(instrSetABC, 5)) {
            Serial.print("New automatic ABC set to ");
            Serial.print(valueHours);
            Serial.println(" hours");
            return 1;
        }
    } else {
        Serial.println("Not enough data received");
    }
    return -1;
}


// =======================================================
// Retrieve firmware
// =======================================================
void getFW() {
    int expected_bytes = 15;
    byte* response = send_and_receive(instrFW, expected_bytes, 4);

    if (response != nullptr && calculateChecksum(response, expected_bytes - 1) == response[expected_bytes - 1]) {
        Serial.print("FW: ");
        for (int i = 3; i < expected_bytes - 2; i++) {
            Serial.print((char)response[i]);
            //printHEX(response[i]);
            Serial.print(" ");
        }
        Serial.println();
        

    } else {
        Serial.println("Invalid FW data");
    }
}

// =======================================================
// Retrieve serial number and production date
// =======================================================
void getSN() {
    int expected_bytes = 11;
    byte* response = send_and_receive(instrSN, expected_bytes, 4);

    if (response != nullptr && calculateChecksum(response, expected_bytes - 1) == response[10]) {
        Serial.print("DC: ");
        Serial.print(response[3]);
        Serial.print("/");
        Serial.print(response[4]);
        Serial.print("/");
        Serial.print(response[5]);

        Serial.print("\tSN: ");
        Serial.print(response[6]);
        Serial.println(response[7]);
    } else {
        Serial.println("Invalid SN data");
    }
}
// =======================================================
// Set alarm threshold
// =======================================================
uint8_t setAlarm(uint16_t upperLimit, uint16_t lowerLimit) {
    int expected_bytes = 8;
    instrAlarmThreshold[3] = upperLimit / 256;
    instrAlarmThreshold[4] = upperLimit % 256;
    instrAlarmThreshold[5] = lowerLimit / 256;
    instrAlarmThreshold[6] = lowerLimit % 256;
    instrAlarmThreshold[7] = calculateChecksum(instrAlarmThreshold, expected_bytes - 1);

    byte* response = send_and_receive(instrAlarmThreshold, expected_bytes, 8);
    if (response != nullptr) {
        if (calculateChecksum(response, expected_bytes - 1) == response[expected_bytes - 1]) {
            return 1;
        }
    }
    return -1;
}

// =======================================================
// Get alarm threshold
// =======================================================
void getAlarm(uint16_t alarmValue[2]) {
    int expected_bytes = 8;
    byte* response = send_and_receive(instrAlarmThreshold, expected_bytes, 8);

    if (response != nullptr) {
        if (calculateChecksum(response, expected_bytes - 1) == response[expected_bytes - 1]) {
            alarmValue[0] = (uint16_t)response[3] * 256 + response[4];
            alarmValue[1] = (uint16_t)response[5] * 256 + response[6];
        }
    } else {
        Serial.println("Invalid alarm threshold data");
    }
}

// =======================================================
// Set baud rate
// =======================================================
uint8_t setBaudrate(uint8_t newBaudrate) {
    /*
        Valid values:
        0x00: 9600 bps
        0x01: 2400 bps
        0x02: 4800 bps
        0x03: 19200 bps
        0x04: 38400 bps
        0x05: 115200 bps
    */
    int expected_bytes = 5;
    instrSetBaud[3] = newBaudrate;
    instrSetBaud[4] = calculateChecksum(instrSetBaud, expected_bytes - 1);

    byte* response = send_and_receive(instrSetBaud, expected_bytes, 5);
    if (response != nullptr) {
        if (calculateChecksum(response, expected_bytes - 1) == response[expected_bytes - 1]) {
            return 1;
        }
    }
    return -1;
}

// =======================================================
// Send and receive data packets
// =======================================================
//
// proceed send and receive seqquence
// does return a array of bytes via pointer

byte* send_and_receive(byte hex_sequence[], uint8_t expected_bytes, uint8_t send_bytes) {
    mySerial.write(hex_sequence, send_bytes);  // Send command
    delay(100);                                // Short wait for response

    static byte response[8];  // Static buffer for response
    if (mySerial.available() >= expected_bytes) {
        mySerial.readBytes(response, expected_bytes);
        return response;
    } else {
        Serial.print("Not enough data received (");
        Serial.println(")");
        return nullptr;
    }
}

// =======================================================
// Calculate checksum
// =======================================================
uint8_t calculateChecksum(uint8_t* ptrData, size_t length) {
    uint8_t checkSum = 0;
    for (size_t i = 0; i < length; i++) {
        checkSum += *(ptrData + i);
    }

    checkSum = ((length * 256) - checkSum) % 256;
    return checkSum;
}

// =======================================================
// Format HEX value for consistent output
// =======================================================
void printHEX(uint8_t dataValue) {
    if (dataValue < 0x10) Serial.print("0");
    Serial.print(dataValue, HEX);
}
