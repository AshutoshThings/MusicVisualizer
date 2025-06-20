// #include <Arduino.h>

// const int micPin = 34;
// const int green = 4;
// const int yellow = 18;
// const int orange = 19;
// const int red = 23;

// const int freq = 5000;
// const int resolution = 12; // 12-bit = 0–4095

// const int greenChannel = 0;
// const int yellowChannel = 1;
// const int orangeChannel = 2;
// const int redChannel = 3;

// float smoothed = 0;
// #define ALPHA 0.1

// void setup()
// {
//   Serial.begin(115200);

//   ledcSetup(greenChannel, freq, resolution);
//   ledcSetup(yellowChannel, freq, resolution);
//   ledcSetup(orangeChannel, freq, resolution);
//   ledcSetup(redChannel, freq, resolution);

//   ledcAttachPin(green, greenChannel);
//   ledcAttachPin(yellow, yellowChannel);
//   ledcAttachPin(orange, orangeChannel);
//   ledcAttachPin(red, redChannel);
// }

// void loop()
// {
//   int reading = analogRead(micPin);
//   smoothed = ALPHA * reading + (1 - ALPHA) * smoothed;

//   int value = max(0, (int)(reading - 2000));

//   // Clear all channels first
//   ledcWrite(greenChannel, 0);
//   ledcWrite(yellowChannel, 0);
//   ledcWrite(orangeChannel, 0);
//   ledcWrite(redChannel, 0);

//   // Light up based on intensity
//   if (value < 200) {
//     ledcWrite(greenChannel, value);
//   } else if (value < 600) {
//     ledcWrite(greenChannel, 4095);
//     ledcWrite(yellowChannel, value - 200);
//   } else if (value < 1000) {
//     ledcWrite(greenChannel, 4095);
//     ledcWrite(yellowChannel, 4095);
//     ledcWrite(orangeChannel, value - 600);
//   } else {
//     ledcWrite(greenChannel, 4095);
//     ledcWrite(yellowChannel, 4095);
//     ledcWrite(orangeChannel, 4095);
//     ledcWrite(redChannel, value - 1000);
//   }

//   // Plotting both raw and smoothed for debug
//   Serial.print(smoothed);
//   Serial.print(",");
//   Serial.println(reading);

//   //delay(50);
// }
