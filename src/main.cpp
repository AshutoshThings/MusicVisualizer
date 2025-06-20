#include <Arduino.h>
#include <arduinoFFT.h>

#define SAMPLES 128
#define SAMPLING_FREQUENCY 20000
#define MIC_PIN 34


const int redLEDs[3] = {13, 12, 14};     // high freq
const int greenLEDs[3] = {21, 22, 23};   // mid freq
const int violetLEDs[2] = {33, 32};      // bass


const int redChannels[3] = {0, 1, 2};
const int greenChannels[3] = {3, 4, 5};
const int violetChannels[2] = {6, 7};

const int PWM_FREQ = 5000;
const int PWM_RES = 8; // 8-bit (0–255)

ArduinoFFT<double> FFT = ArduinoFFT<double>();

double vReal[SAMPLES];
double vImag[SAMPLES];


float redBrightness[3] = {0, 0, 0};
float greenBrightness[3] = {0, 0, 0};
float violetBrightness[2] = {0, 0};

unsigned long lastPatternTime = 0;
bool patternActive = false;
int patternStep = 0;
int direction = 1;     // +1 for forward, -1 for backward
int roundTrips = 0;

void triggerBubblePattern() {
  const int patternDelay = 30; // delay between frames

  const int totalLEDs = 8;
  unsigned long now = millis();

  if (now - lastPatternTime > patternDelay) {
    lastPatternTime = now;

    int brightness = 255;
    int dim = 30;

    // Light only the current step
    for (int i = 0; i < 3; i++) {
      ledcWrite(redChannels[i], (patternStep == i) ? brightness : dim);
    }
    for (int i = 0; i < 3; i++) {
      ledcWrite(greenChannels[i], (patternStep == i + 3) ? brightness : dim);
    }
    for (int i = 0; i < 2; i++) {
      ledcWrite(violetChannels[i], (patternStep == i + 6) ? brightness : dim);
    }

    // Move the bubble
    patternStep += direction;

    // Change direction at ends
    if (patternStep >= totalLEDs - 1 || patternStep <= 0) {
      direction *= -1;
    }

    // Optional: stop the pattern after a full round trip

    if (patternStep == 0 && direction == -1) {
        roundTrips++;
      }
    }

    // Stop after 1 round trip
    if (roundTrips >= 1) {
      patternActive = false;
      roundTrips = 0;
      patternStep = 0;
      direction = 1;
    }
  }
  

void setup() {
  Serial.begin(115200);
  Serial.println("Starting FFT...");

  for (int i = 0; i < 3; i++) {
    ledcSetup(redChannels[i], PWM_FREQ, PWM_RES);
    ledcAttachPin(redLEDs[i], redChannels[i]);

    ledcSetup(greenChannels[i], PWM_FREQ, PWM_RES);
    ledcAttachPin(greenLEDs[i], greenChannels[i]);
  }

  for (int i = 0; i < 2; i++) {
    ledcSetup(violetChannels[i], PWM_FREQ, PWM_RES);
    ledcAttachPin(violetLEDs[i], violetChannels[i]);
  }
}

void loop() {
  
  unsigned long sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
  unsigned long t_start;

  for (int i = 0; i < SAMPLES; i++) {
    t_start = micros();
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0;
    while ((micros() - t_start) < sampling_period_us);
  }

  // Remove DC offset
  double sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += vReal[i];
  double avg = sum / SAMPLES;
  for (int i = 0; i < SAMPLES; i++) vReal[i] -= avg;

  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  // Energy per band
  double bassEnergy = 0, midEnergy = 0, highEnergy = 0;

  for (int i = 1; i <= 6; i++) bassEnergy += vReal[i];         // ~0–1 kHz
  for (int i = 7; i <= 38; i++) midEnergy  += vReal[i];        // ~1–6 kHz
  for (int i = 39; i <= 63; i++) highEnergy += vReal[i];       // ~6–10 kHz

  bassEnergy  /= 6.0;
  midEnergy   /= 32.0;
  highEnergy  /= 25.0;

  // Apply noise threshold
  const double noiseThreshold = 20.0;
  if (bassEnergy < noiseThreshold) bassEnergy = 0;
  if (midEnergy < noiseThreshold)  midEnergy = 0;
  if (highEnergy < noiseThreshold) highEnergy = 0;

  // Linear brightness mapping
  int bassBrightness  = map(bassEnergy, 0, 8000, 0, 255);
  int midBrightness   = map(midEnergy, 0, 12000, 0, 255);
  int highBrightness  = map(highEnergy, 0, 8000, 0, 255);

  if (bassBrightness > 40 && midBrightness > 40 && highBrightness > 40) {
  patternActive = true;
  patternStep = 0;
  // direction = 1;         // ✅ reset direction
  // roundTrips = 0;        // ✅ reset trip count
  lastPatternTime = millis();
}

  const float fadeFactor = 0.5;


  bassBrightness  = constrain(bassBrightness, 0, 255);
  midBrightness   = constrain(midBrightness, 0, 255);
  highBrightness  = constrain(highBrightness, 0, 255);

  // Fading for violet (bass)
for (int i = 0; i < 2; i++) {
  violetBrightness[i] += (bassBrightness - violetBrightness[i]) * fadeFactor;
  ledcWrite(violetChannels[i], (int)violetBrightness[i]);
}

// Fading for green (mid)
for (int i = 0; i < 3; i++) {
  greenBrightness[i] += (midBrightness - greenBrightness[i]) * fadeFactor;
  ledcWrite(greenChannels[i], (int)greenBrightness[i]);
}

// Fading for red (high)
for (int i = 0; i < 3; i++) {
  redBrightness[i] += (highBrightness - redBrightness[i]) * fadeFactor;
  ledcWrite(redChannels[i], (int)redBrightness[i]);
}

  // Debug output
  Serial.print("Bass: "); Serial.print(bassBrightness);
  Serial.print(" | Mid: "); Serial.print(midBrightness);
  Serial.print(" | High: "); Serial.println(highBrightness);

  // if (patternActive) {
  // triggerBubblePattern();
  // return;  // skip normal LED fade this cycle
//}
  
  delay(20);
}
