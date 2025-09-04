#define TRIG_PIN 7
#define ECHO_PIN 8
#define RELAY_PIN 4
#define BUTTON_PIN 2
#define BUZZER_PIN 3
#define LED_MODE1_PIN 9   // LED mode 1 menit
#define LED_MODE2_PIN 10  // LED 2 mode 10 menit
#define LED_MODE3_PIN 11  // LED 3 mode 20 menit

// Variabel sensor
long duration;
int distance;

// Timer
int timerMode = 0;  
int selectedMode = 1;
unsigned long timerDuration[] = {0, 60000, 600000, 1200000}; 
unsigned long totalTherapyTime = 0; 
bool lampActive = false;
bool objectInRange = false;
bool therapyStarted = false;

// Button
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long lastButtonPressTime = 0;
bool modeSelectionActive = false;
unsigned long modeSelectionTimeout = 3000;

// Buzzer peringatan posisi
unsigned long lastBeepTime = 0;
unsigned long beepInterval = 1000; // beep tiap 1 detik

// --- Fungsi Setup ---
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Setup LED pins
  pinMode(LED_MODE1_PIN, OUTPUT);
  pinMode(LED_MODE2_PIN, OUTPUT);
  pinMode(LED_MODE3_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW); // Relay OFF saat awal
  
  // Matikan semua LED saat startup
  turnOffAllLEDs();

  // Bunyi startup singkat
  tone(BUZZER_PIN, 1000, 100);
}

// --- Loop utama ---
void loop() {
  handleModeSelection();
  distance = readDistance();

  if (timerMode > 0) {
    bool currentObjectInRange = (distance >= 20 && distance <= 50);

    if (currentObjectInRange != objectInRange) {
      objectInRange = currentObjectInRange;

      if (objectInRange) {
        lampActive = true;
        digitalWrite(RELAY_PIN, HIGH);
        if (!therapyStarted) {
          therapyStarted = true;
          startTherapyBeep();
        }
      } else {
        lampActive = false;
        digitalWrite(RELAY_PIN, LOW);
      }
    }

    if (objectInRange && lampActive) {
      totalTherapyTime += 100;
      if (totalTherapyTime >= timerDuration[timerMode]) {
        completeTherapy();
      }
    }

    // 🔔 Jika objek di luar jarak → buzzer "beeb beeb"
    if (!objectInRange && timerMode > 0) {
      if (millis() - lastBeepTime >= beepInterval) {
        tone(BUZZER_PIN, 1200, 150); // bunyi beep
        lastBeepTime = millis();
      }
    }

  } else {
    digitalWrite(RELAY_PIN, LOW);
  }

  delay(100);
}

// --- Fungsi Baca Jarak ---
int readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 20000); // Timeout 20ms
  int dist = duration * 0.034 / 2;
  if (dist > 400 || dist < 2) return 999;
  return dist;
}

// --- Mode Selection ---
void handleModeSelection() {
  if (timerMode == 0) {
    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState) lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != currentButtonState) {
        currentButtonState = reading;
        if (currentButtonState == LOW) handleModeButtonPress();
      }
    }

    lastButtonState = reading;
    if (modeSelectionActive && (millis() - lastButtonPressTime > modeSelectionTimeout)) {
      confirmModeSelection();
    }
  }
}

void handleModeButtonPress() {
  lastButtonPressTime = millis();

  if (!modeSelectionActive) {
    modeSelectionActive = true;
    selectedMode = 1;
  } else {
    selectedMode++;
    if (selectedMode > 3) selectedMode = 1;
  }

  updateLEDIndicator(selectedMode); // Update LED sesuai mode yang dipilih
  playBuzzerForMode(selectedMode);
}

void confirmModeSelection() {
  if (modeSelectionActive) {
    modeSelectionActive = false;
    timerMode = selectedMode;

    tone(BUZZER_PIN, 1200, 300); // bunyi panjang konfirmasi

    totalTherapyTime = 0;
    therapyStarted = false;
    selectedMode = 1;
    modeSelectionActive = false;
  }
}

// --- Selesai Terapi ---
void completeTherapy() {
  lampActive = false;
  digitalWrite(RELAY_PIN, LOW);

  timerMode = 0;
  selectedMode = 1;
  totalTherapyTime = 0;
  therapyStarted = false;
  objectInRange = false;
  modeSelectionActive = false;

  turnOffAllLEDs(); // Matikan semua LED setelah terapi selesai
  finishTherapyBeep();
}

// --- Fungsi LED Indikator ---
void updateLEDIndicator(int mode) {
  // Matikan semua LED terlebih dahulu
  turnOffAllLEDs();
  
  // Nyalakan LED sebanyak mode yang dipilih
  if (mode >= 1) digitalWrite(LED_MODE1_PIN, HIGH); // Mode 1: nyala 1 LED
  if (mode >= 2) digitalWrite(LED_MODE2_PIN, HIGH); // Mode 2: nyala 2 LED
  if (mode >= 3) digitalWrite(LED_MODE3_PIN, HIGH); // Mode 3: nyala 3 LED
}

void turnOffAllLEDs() {
  digitalWrite(LED_MODE1_PIN, LOW);
  digitalWrite(LED_MODE2_PIN, LOW);
  digitalWrite(LED_MODE3_PIN, LOW);
}

// --- Buzzer ---
void playBuzzerForMode(int mode) {
  for (int i = 0; i < mode; i++) {
    tone(BUZZER_PIN, 1000, 150);
    delay(300);
  }
}

void startTherapyBeep() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1500, 100);
    delay(150);
  }
}

void finishTherapyBeep() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 800, 300);
    delay(400);
  }
}
