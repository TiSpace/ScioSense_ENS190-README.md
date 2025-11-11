void runCommunication() {
    static bool inputMode = false;
    static int values[2];
    static int inputCount = 0;

    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();  // Remove whitespace


        switch (line.charAt(0)) {
            case 'a':
                {
                    Serial.print("new baseline: [ppm]");
                    int newValueBL = readIntegerFromSerial();
                    if (newValueBL > 399) {
                        Serial.print("\n");
                        setBaseline(newValueBL);
                        
                    } else {
                        Serial.println("Value not valid");
                    }
                    
                    break;
                }
            case 'c':
                {
                    Serial.print("new automativ baseline correction [24...4230h]");
                    int newValue = readIntegerFromSerial();
                    Serial.print("\n");
                    
                    setBaseline(newValue);

                    break;
                }

            case 'i':
                {
                    Serial.print("set new measurement intervall [sec]");
                    measurementDelay = readIntegerFromSerial();
                    if (measurementDelay < 2) measurementDelay = 2;
                    Serial.print("\n");
                    break;
                }

            case 'm':
                {
                    enableMeasurement = !enableMeasurement;
                    Serial.println(enableMeasurement ? "Measurement started" : "Measurement stopped");
                    break;
                }
            case 'r':// read programmed threshold limits
                {  
                    
                    uint16_t alarmValue[2];
                    getAlarm(alarmValue);
                    Serial.print("\nAlarm threshold upper: ");
                    Serial.print(alarmValue[0]);
                    Serial.print("\nAlarm treshold lower: ");
                    Serial.println(alarmValue[1]);
                    break;
                }

            case 'l':
                {
                    Serial.println("Alarm setting:");
                    Serial.print("Enter lower limit (end with CR):");
                    int lower = readIntegerFromSerial();

                    Serial.print("\nEnter upper limit (end with CR):");
                    int upper = readIntegerFromSerial();
                    if ((upper < lower) | (lower < 400)) {
                        Serial.println("\nInput withdraw, limits are not valid!");
                    } else {
                        setAlarm(upper, lower);
                        Serial.println("\nnew values stored");
                    }
                    break;
                }

            case 'f':
                getFW();
                break;
            case 's':
                getSN();
                break;
            case '?':
                printMenue();
                break;

            default:
                //} else {
                Serial.println("Invalid input.");
                break;
                //}
        }

    }
}


void printMenue() {
    Serial.println("Menu:");
    Serial.println("m - start/stop measurement");
    Serial.println("c - enter automatic baseline intervall [h] (24..4230)");
    Serial.println("a - enter new baseline (>399)");
    Serial.println("l - enter upper and lower alarm limit");
    Serial.println("i - enter measurement interval");
    Serial.println("r - get upper and lower alarm limit");
    Serial.println("f - get firmware");
    Serial.println("s - get serial number");
    Serial.println("? - this menue");
}

int readIntegerFromSerial() {
    String input = "";

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();

            // Abschluss bei Carriage Return
            if (c == '\r') {
                if (input.length() > 0) {
                    return input.toInt();  // Rückgabe des gültigen Integer-Werts
                } else {
                    Serial.println("No input received.");
                    return -1;  // Fehlerwert
                }
            }

            // Nur Ziffern akzeptieren
            if (isDigit(c)) {
                input += c;
                Serial.print(c);  // Echo zur Bestätigung
            } else {
                Serial.println("\nInvalid character. Only digits allowed.");
                input = "";  // Eingabe verwerfen
            }
        }
    }
}