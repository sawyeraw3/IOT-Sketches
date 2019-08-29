// Example messages to ESP:
// - esp0:relay -> toggle relay power via esp0
// - esp1:neo -> toggle NeoPixel power via esp1
// - esp1:neo:b:100 -> set NeoPixel brightness to 100 via esp1

class EspFunctionality {
  public:
    // Currently works for single IO
    int pinIsActive = 0;
    char functionName[15];
    const int ONBOARD_LED_PIN = 16;
    virtual void setUp() {}
    virtual void performTask(const char** tokens) {}
};

//TODO add support to control multiple relay pins
// i.e esp0:relay:2 -> toggle relay #2 power via esp0
class RelayEspFunction : public EspFunctionality {
  private:
    int relayPin;

    void toggleRelayAndOboardLed() {
      digitalWrite (relayPin, pinIsActive);
      digitalWrite(ONBOARD_LED_PIN, pinIsActive);
      pinIsActive = !pinIsActive;
    }

  public:
    RelayEspFunction(int pinVal) {
      relayPin = pinVal;
      strcpy(functionName, "relay");
    }

    void setUp() {
      pinMode(ONBOARD_LED_PIN, OUTPUT);
      pinMode(relayPin, OUTPUT);
    }

    void performTask(const char** tokens) {
			//TODO check if tokens is well formed & valid
      if (tokens) {
        //TODO handle multiple relays controlled by int ID
        // i.e. tokens[0] = int which relay
      } else {
        toggleRelayAndOboardLed();
      }
    }
};

class NeopixelEspFunction : public EspFunctionality {
  private:
    int pixelPin;
    int numNeopixels;
    const char* BRGTNS_CMD = "b";

    void toggleNeopixelAndOboardLed(const char** tokens) {
      if (tokens) {
        const char* messageTyp = tokens[0];
        if (!strcmp(messageTyp, BRGTNS_CMD)) {
          int inputBrightness = atoi(tokens[1]);
          handleBrightness(inputBrightness);
        }
      } else {
        digitalWrite(ONBOARD_LED_PIN, pinIsActive);
        pinIsActive = !pinIsActive;
      }
    }

    void handleBrightness(int b) {
      Serial.println(b);
    }

  public:
    NeopixelEspFunction(int pinVal, int nNeopixels) {
      pixelPin = pinVal;
      numNeopixels = nNeopixels;
      strcpy(functionName, "neo");
    }

    void setUp() {
      pinMode(ONBOARD_LED_PIN, OUTPUT);
      pinMode(pixelPin, OUTPUT);
    }

    void performTask(const char** tokens) {
			//TODO check if tokens is well formed & valid
      toggleNeopixelAndOboardLed(tokens);
    }
};
