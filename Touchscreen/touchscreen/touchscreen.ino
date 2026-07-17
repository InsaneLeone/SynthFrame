// --- Configuration ---
const int TOUCH_PIN = 4;        // GPIO 4 (Touch Channel 0)
const int TOUCH_THRESHOLD = 10; // Lower values = touched. Adjust based on your Serial Monitor.

// --- Timing Constants (Milliseconds) ---
const unsigned long DEBOUNCE_TIME = 30;    // Filter out signal noise
const unsigned long LONG_PRESS_TIME = 600; // Time held down to qualify as a long press
const unsigned long DOUBLE_TAP_GAP = 300;  // Maximum time window between two taps

// --- State Variables ---
bool lastTouchState = false;       // Tracked state: false = released, true = touched
unsigned long touchStartTime = 0;   // When the current touch event began
unsigned long touchEndTime = 0;     // When the last touch event ended
int tapCount = 0;                  // Counter for multiple taps
bool waitingForDoubleTap = false;  // Timing window status

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- ELEGOO ESP32 Touch Gesture Detector ---");
  Serial.print("Initial Baseline Value: ");
  Serial.println(touchRead(TOUCH_PIN));
}

void loop() {
  // 1. Read and smooth the sensor state
  int rawTouch = touchRead(TOUCH_PIN);
  Serial.println(rawTouch);
  bool currentTouchState = (rawTouch < TOUCH_THRESHOLD);
  unsigned long currentMillis = millis();

  // 2. Detect Edge Changes (State transitions)
  if (currentTouchState != lastTouchState) {
    delay(DEBOUNCE_TIME); // Basic software debounce to confirm state change
    
    if (currentTouchState) {
      // --- PIN JUST TOUCHED ---
      touchStartTime = currentMillis;
    } else {
      // --- PIN JUST RELEASED ---
      touchEndTime = currentMillis;
      unsigned long pressDuration = touchEndTime - touchStartTime;

      // Filter out accidental micro-touches
      if (pressDuration > DEBOUNCE_TIME) {
        if (pressDuration >= LONG_PRESS_TIME) {
          // It was a long press, clear any pending double-tap tracking
          Serial.println("Gesture Captured: [ LONG PRESS ]");
          tapCount = 0;
          waitingForDoubleTap = false;
        } else {
          // It was a short tap, log it for double-tap evaluation
          tapCount++;
          if (!waitingForDoubleTap) {
            waitingForDoubleTap = true;
            touchStartTime = currentMillis; // Re-use as the window timer start
          }
        }
      }
    }
    lastTouchState = currentTouchState;
  }

  // 3. Evaluate Tap Window (When user is NOT currently holding the pin)
  if (waitingForDoubleTap && !currentTouchState) {
    if (currentMillis - touchEndTime > DOUBLE_TAP_GAP) {
      // Time window closed! Evaluate how many taps happened
      if (tapCount >= 2) {
        Serial.print("Gesture Captured: [ DOUBLE TAP ] -> Total Taps: ");
        Serial.println(tapCount);
      } else if (tapCount == 1) {
        Serial.println("Gesture Captured: [ SHORT PRESS ]");
      }
      
      // Reset counters for the next event
      tapCount = 0;
      waitingForDoubleTap = false;
    }
  }
  delay(500);
}
