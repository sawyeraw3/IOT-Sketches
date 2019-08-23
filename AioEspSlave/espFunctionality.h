class EspFunctionality {
  public:
  virtual void setUp() {}
  virtual void performTask() {}
};

class RelayEspFunction : public EspFunctionality {
  private:
  bool isOn = false;
  int relayPin;
  public:
  RelayEspFunction(int pinVal) {
    relayPin = pinVal;
  }

  virtual void setUp() {
    pinMode(relayPin, OUTPUT);
  }
  
  virtual void performTask() {
    isOn ? digitalWrite (relayPin, 0) : digitalWrite(relayPin, 1);
    isOn ? digitalWrite (16, 0) : digitalWrite(16, 1);
    isOn = !isOn;
  }
};
