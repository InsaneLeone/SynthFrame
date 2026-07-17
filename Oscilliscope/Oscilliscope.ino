#include <DIYables_TFT_Round.h>


// ================= COLORS =================

#define BLACK     DIYables_TFT::colorRGB(0,0,0)
#define BLUE      DIYables_TFT::colorRGB(0,0,255)
#define RED       DIYables_TFT::colorRGB(255,0,0)
#define GRAY      DIYables_TFT::colorRGB(80,80,80)
#define WHITE     DIYables_TFT::colorRGB(255,255,255)


// ================= DISPLAY PINS =================

#define PIN_RST 20
#define PIN_DC  21
#define PIN_CS  5


DIYables_TFT_GC9A01_Round TFT_display(
  PIN_RST,
  PIN_DC,
  PIN_CS
);


// ================= SCREEN =================

#define WIDTH 240
#define HEIGHT 240


// ================= ADC =================

// ADC0
#define ADC_PIN 1


// ================= SCOPE SETTINGS =================

// samples displayed
#define SAMPLES 240

// sampling frequency
// 20kHz = good for 1.5kHz signals
#define SAMPLE_DELAY_US 10


// ================= VOLTAGE SCALE =================

// 300mV - 900mV range
// ESP32 ADC 12-bit

#define ADC_MIN 0
#define ADC_MAX 1117



int samples[SAMPLES];



// ================= DRAW GRID =================

void drawGrid()
{

  TFT_display.fillScreen(BLACK);


  // Circular scope grid

  TFT_display.drawCircle(
    120,
    120,
    40,
    GRAY
  );


  TFT_display.drawCircle(
    120,
    120,
    80,
    GRAY
  );


  TFT_display.drawCircle(
    120,
    120,
    115,
    GRAY
  );


  // center horizontal axis

  TFT_display.drawLine(
    5,
    120,
    235,
    120,
    RED
  );


  // vertical center

  TFT_display.drawLine(
    120,
    5,
    120,
    235,
    GRAY
  );


  TFT_display.setTextColor(WHITE);
  TFT_display.setCursor(5,5);
  TFT_display.print("300-900mV");

}



// ================= SAMPLE ADC =================
void acquireWaveform()
{
  int value;

  // Wait for rising edge trigger
  do
  {
    value = analogRead(ADC_PIN);

    //Print ADC Value
    Serial.print("adc_value : ");
    Serial.println(value);

  } while(value < 0);


  // Capture waveform
  for(int i=0;i<SAMPLES;i++)
  {
    samples[i] = analogRead(ADC_PIN);

    delayMicroseconds(SAMPLE_DELAY_US);
  }
}



// ================= DRAW WAVE =================

void drawWaveform()
{

  int oldX = 0;

  int oldY = map(
      samples[0],
      ADC_MIN,
      ADC_MAX,
      220,
      20
  );


  // clamp

  if(oldY < 10)
    oldY = 10;

  if(oldY > 230)
    oldY = 230;



  for(int i=1;i<SAMPLES;i++)
  {


    int y = map(
      samples[i],
      ADC_MIN,
      ADC_MAX,
      220,
      20
    );


    if(y < 10)
      y = 10;

    if(y > 230)
      y = 230;



    TFT_display.drawLine(
      oldX,
      oldY,
      i,
      y,
      BLUE
    );


    oldX=i;
    oldY=y;

  }

}



// ================= SETUP =================

void setup()
{

  Serial.begin(115200);


  TFT_display.begin();

  TFT_display.setRotation(1);


  analogReadResolution(12);


  // ADC speed settings

  analogSetPinAttenuation(
    ADC_PIN,
    ADC_11db
  );


  drawGrid();

}



// ================= LOOP =================

void loop()
{

  // capture waveform

  acquireWaveform();



  // redraw background

  drawGrid();



  // draw waveform

  drawWaveform();



  delay(1);

}