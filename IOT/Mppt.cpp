// Incremental Conductance MPPT for ESP32
// Himadri Hackathon Project

#define V_SENSOR 34   // ADC pin for voltage
#define I_SENSOR 35   // ADC pin for current
#define PWM_PIN 25    // PWM output to DC-DC converter

float V_now, I_now, V_prev=0, I_prev=0;
float dV, dI;
float duty = 0.5; // initial duty cycle

void setup() {
  Serial.begin(115200);
  ledcSetup(0, 20000, 10); // Channel 0, 20kHz, 10-bit resolution
  ledcAttachPin(PWM_PIN, 0);
  ledcWrite(0, duty * 1023);
}

float readVoltage() {
  int adc = analogRead(V_SENSOR);
  return (adc / 4095.0) * 50.0; // scale to panel voltage
}

float readCurrent() {
  int adc = analogRead(I_SENSOR);
  return (adc / 4095.0) * 10.0; // scale to current sensor
}

void loop() {
  V_now = readVoltage();
  I_now = readCurrent();

  dV = V_now - V_prev;
  dI = I_now - I_prev;

  if (dV == 0) {
    if (dI == 0) {
      // At MPP → no change
    } else if (dI > 0) {
      duty -= 0.01; // increase voltage
    } else {
      duty += 0.01; // decrease voltage
    }
  } else {
    if ((dI / dV) == -(I_now / V_now)) {
      // At MPP → no change
    } else if ((dI / dV) > -(I_now / V_now)) {
      duty -= 0.01; // left of MPP → increase voltage
    } else {
      duty += 0.01; // right of MPP → decrease voltage
    }
  }

  // Clamp duty cycle between 0 and 1
  if (duty > 1) duty = 1;
  if (duty < 0) duty = 0;

  ledcWrite(0, duty * 1023);

  Serial.print("V: "); Serial.print(V_now);
  Serial.print(" I: "); Serial.print(I_now);
  Serial.print(" Duty: "); Serial.println(duty);

  V_prev = V_now;
  I_prev = I_now;

  delay(200);
}
