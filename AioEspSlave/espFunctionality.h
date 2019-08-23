class EspFunctionality {
  public:
  int ONBOARD_LED_PIN = 16;
  virtual void setUp() {}
  virtual void performTask() {}
};

class RelayEspFunction : public EspFunctionality {
  private:
  bool isOn = false;
  int relayPin;

  void toggleRelayAndOboardLed() {
    digitalWrite (relayPin, int(isOn));
    digitalWrite(ONBOARD_LED_PIN, int(isOn));
    isOn = !isOn;
  }
  
  public:
  RelayEspFunction(int pinVal) {
    relayPin = pinVal;
  }

  virtual void setUp() {
    pinMode(relayPin, OUTPUT);
  }
  
  virtual void performTask() {
    toggleRelayAndOboardLed();
  }
};