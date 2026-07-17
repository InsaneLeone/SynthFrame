const int DAC_PIN = 25;

void setup() {
}

void loop() {

  // 0-255 maps to 0-3.3V
  dacWrite(DAC_PIN, 255); 

}