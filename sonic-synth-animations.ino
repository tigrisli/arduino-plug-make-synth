// ARDUINO: PLUG & MAKE KIT
// SONIC SYNTH PROJECT MODIFIED BY TIGRIS
// SUNDAY, OCTOBER 20, 2024

#include "arduino_secrets.h"
#include <Modulino.h>
#include "thingProperties.h"
#include <Arduino_LED_Matrix.h>
#include <Arduino_CloudConnectionFeedback.h>

// Create object instances
ArduinoLEDMatrix matrix;
ModulinoBuzzer buzzer;
ModulinoButtons buttons;
ModulinoKnob knob;
ModulinoPixels pixels;

int lastKnobVal = 0;
int currentKnobVal = 0;

int minFrequency = 262;
int maxFrequency = 523;

int ledPosition = 0;

bool isAnimationPlaying = false;  // Track whether an animation is currently playing
unsigned long lastFrameTime = 0;  // Record time of last frame update
unsigned long frameInterval = 100;  // Time between each frame

const uint32_t (*currentAnimation)[4];  // Store current animation
int currentFrame = 0;
int totalFrames = 0;  // Total frames in the current animation

// List of animations
const uint32_t (*animations[])[4] = {
  LEDMATRIX_ANIMATION_TETRIS_INTRO,
  LEDMATRIX_ANIMATION_ATMEGA,
  LEDMATRIX_ANIMATION_LED_BLINK_HORIZONTAL,
  LEDMATRIX_ANIMATION_LED_BLINK_VERTICAL,
  LEDMATRIX_ANIMATION_ARROWS_COMPASS,
  LEDMATRIX_ANIMATION_AUDIO_WAVEFORM,
  LEDMATRIX_ANIMATION_BATTERY,
  LEDMATRIX_ANIMATION_BUG,
  LEDMATRIX_ANIMATION_CHECK,
  LEDMATRIX_ANIMATION_DOWNLOAD,
  LEDMATRIX_ANIMATION_HEARTBEAT_LINE,
  LEDMATRIX_ANIMATION_HEARTBEAT,
  LEDMATRIX_ANIMATION_LOCK,
  LEDMATRIX_ANIMATION_NOTIFICATION,
  LEDMATRIX_ANIMATION_SPINNING_COIN,
  LEDMATRIX_ANIMATION_TETRIS
};

const int animationLengths[] = {
  20,  // Number of frames for LEDMATRIX_ANIMATION_TETRIS_INTRO
  6,   // Number of frames for LEDMATRIX_ANIMATION_ATMEGA
  4,   // Number of frames for LEDMATRIX_ANIMATION_LED_BLINK_HORIZONTAL
  4,   // Number of frames for LEDMATRIX_ANIMATION_LED_BLINK_VERTICAL
  8,   // Number of frames for LEDMATRIX_ANIMATION_ARROWS_COMPASS
  8,   // Number of frames for LEDMATRIX_ANIMATION_AUDIO_WAVEFORM
  5,   // Number of frames for LEDMATRIX_ANIMATION_BATTERY
  4,   // Number of frames for LEDMATRIX_ANIMATION_BUG
  9,   // Number of frames for LEDMATRIX_ANIMATION_CHECK
  2,   // Number of frames for LEDMATRIX_ANIMATION_DOWNLOAD
  12,   // Number of frames for LEDMATRIX_ANIMATION_HEARTBEAT_LINE
  12,   // Number of frames for LEDMATRIX_ANIMATION_HEARTBEAT
  4,   // Number of frames for LEDMATRIX_ANIMATION_LOCK
  4,   // Number of frames for LEDMATRIX_ANIMATION_NOTIFICATION
  6,   // Number of frames for LEDMATRIX_ANIMATION_SPINNING_COIN
  30    // Number of frames for LEDMATRIX_ANIMATION_TETRIS
};

void setup() {
  Serial.begin(9600);
  initProperties();

  matrix.begin();

  // Set up Arduino Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  waitForArduinoCloudConnection(matrix);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Initialize other components
  Modulino.begin();
  buzzer.begin();
  buttons.begin();
  knob.begin();
  pixels.begin();

  knob.set(0);
  buttons.setLeds(true, true, true);  // Set button LEDs
}

// Play the sound for a short duration
void playSound(int newFrequency) {
  buzzer.tone(newFrequency, 200); 
}

// Select a random animation
void playRandomAnimation() {
  int randomIndex = random(0, sizeof(animations) / sizeof(animations[0]));
  currentAnimation = animations[randomIndex];

  // Load selected animation
  matrix.loadSequence(currentAnimation);

  currentFrame = 0;  // Start from first frame
  totalFrames = animationLengths[randomIndex];  // Get the total frames for the selected animation
  isAnimationPlaying = true;
  lastFrameTime = millis();  // Start frame timer
}

// Update animation frame by frame
void updateAnimation() {
  if (isAnimationPlaying) {
    unsigned long currentTime = millis();

    // Check if update to the next frame
    if (currentTime - lastFrameTime >= frameInterval) {
      lastFrameTime = currentTime;

      // Load current frame
      matrix.loadFrame(currentAnimation[currentFrame]);

      // Move to next frame + loop if needed
      currentFrame++;
      if (currentFrame >= totalFrames) {
        currentFrame = 0;  // Loop animation
      }
    }
  }
}

int readKnob() {
  lastKnobVal = currentKnobVal;
  currentKnobVal = knob.get();
  if (currentKnobVal > lastKnobVal) {
    return 1;
  } else if (currentKnobVal < lastKnobVal) {
    return -1;
  } else {
    return 0;
  }
}

void loop() {
  ArduinoCloud.update();

  // Always update animation (frame by frame)
  updateAnimation();

  // Check for button presses
  if (buttons.update()) {
    if (buttons.isPressed(0)) {
      playSound(frequency);  // Play sound for button 0
      playRandomAnimation();  // Start new animation
      buttons.setLeds(true, false, false);
    }
    if (buttons.isPressed(1)) {
      playSound(frequency * 1.25);  // Play sound for button 1
      playRandomAnimation();  // Start new animation
      buttons.setLeds(false, true, false);
    }
    if (buttons.isPressed(2)) {
      playSound(frequency * 1.5);  // Play sound for button 2
      playRandomAnimation();  // Start new animation
      buttons.setLeds(false, false, true);
    }
  }

  // Handle knob control for adjusting frequency
  int knobDirection = readKnob();

  if (knobDirection == 1) {
    frequency = frequency * 1.12;
  } else if (knobDirection == -1) {
    frequency = frequency / 1.12;
  }

  if (frequency > maxFrequency) {
    frequency = maxFrequency;
  } else if (frequency < minFrequency) {
    frequency = minFrequency;
  }

  // Update LED pixels
  ledPosition = ledPosition + knobDirection;

  pixels.clear();
  pixels.set(ledPosition, BLUE, 10);
  pixels.show();

  if (ledPosition < 0) {
    ledPosition = 0;
  } else if (ledPosition >= 7) {
    ledPosition = 7;
  }
}

// The functions below are called callback functions but we don't need them for this project.

void onDurationChange() {}

void onFrequencyChange() {}
