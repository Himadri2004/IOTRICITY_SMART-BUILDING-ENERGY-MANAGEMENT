// Himadri Hackathon Version (Arduino Compatible)

// ADC pin for solar panel voltage sensor
#define PV_VOLT_PIN A0   
// ADC pin for solar panel current sensor
#define PV_CURR_PIN A1   
// PWM output pin to control DC-DC converter MOSFET
#define PWM_PIN 9        

// Calibration factors (adjust based on your sensors)
float V_SCALE = 0.1;   // Voltage sensor scaling (V per ADC unit)
float I_SCALE = 0.01;  // Current sensor scaling (A per ADC unit)

float duty = 0.5;     // Initial duty cycle
float Vprev = 0, Iprev = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PWM_PIN, OUTPUT);
  setDuty(duty);

  Serial.println("Arduino MPPT (Incremental Conductance) Started...");
}

void loop() {
  // 1. Read panel voltage & current
  float Vpv = analogRead(PV_VOLT_PIN) * V_SCALE;
  float Ipv = analogRead(PV_CURR_PIN) * I_SCALE;

  // 2. Calculate deltas
  float dV = Vpv - Vprev;
  float dI = Ipv - Iprev;

  // 3. Incremental Conductance decision
  if (dV == 0) {
    if (dI == 0) {
      // At MPP -> do nothing
    } else if (dI > 0) {
      duty += 0.01; // increase voltage
    } else {
      duty -= 0.01; // decrease voltage
    }
  } else {
    if ((dI / dV) == -(Ipv / Vpv)) {
      // At MPP -> hold duty
    } else if ((dI / dV) > -(Ipv / Vpv)) {
      duty += 0.01; // Left of MPP -> increase
    } else {
      duty -= 0.01; // Right of MPP -> decrease
    }
  }

  // 4. Limit duty cycle
  if (duty > 0.95) duty = 0.95;
  if (duty < 0.05) duty = 0.05;

  // 5. Apply duty cycle
  setDuty(duty);

  // 6. Store values
  Vprev = Vpv;
  Iprev = Ipv;

  // 7. Debug
  Serial.print("V: "); Serial.print(Vpv);
  Serial.print("  I: "); Serial.print(Ipv);
  Serial.print("  Duty: "); Serial.println(duty);

  delay(200); // Sampling delay
}

// Function to set PWM duty cycle (Arduino way)
void setDuty(float dutyCycle) {
  int pwmVal = dutyCycle * 255;  // 8-bit resolution for Arduino
  analogWrite(PWM_PIN, pwmVal);
}


