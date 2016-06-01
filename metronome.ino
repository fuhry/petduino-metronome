#include <LedControl.h>
#include <Petduino.h>

Petduino pet = Petduino();

#define TEMPO_SET_STATE 1
#define CLICK_STATE 2
#define FLASH_TIME 25
#define POLL_INTERVAL 10

#define DIGIT_START_ROW 1
#define DIGIT_HEIGHT 5

byte screen_on[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte screen_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

byte hundreds[DIGIT_HEIGHT] = {
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000
};

byte digits[10][DIGIT_HEIGHT] = {
  {
    B00000010,
    B00000101,
    B00000101,
    B00000101,
    B00000010
  },
  {
    B00000010,
    B00000010,
    B00000010,
    B00000010,
    B00000010
  },
  {
    B00000010,
    B00000101,
    B00000001,
    B00000010,
    B00000111
  },
  {
    B00000110,
    B00000001,
    B00000010,
    B00000001,
    B00000110
  },
  {
    B00000101,
    B00000101,
    B00000111,
    B00000001,
    B00000001
  },
  {
    B00000111,
    B00000100,
    B00000110,
    B00000001,
    B00000110
  },
  {
    B00000011,
    B00000100,
    B00000110,
    B00000101,
    B00000010
  },
  {
    B00000111,
    B00000001,
    B00000010,
    B00000010,
    B00000010
  },
  {
    B00000010,
    B00000101,
    B00000010,
    B00000101,
    B00000010
  },
  {
    B00000010,
    B00000101,
    B00000011,
    B00000001,
    B00000111
  }
};

unsigned int currBpm = 132;
unsigned int lastBpm = 0;
unsigned int bpmCount = 0;
unsigned int beat = 0;
unsigned long timeout = 0;

unsigned int stretch(unsigned int low, unsigned int high, unsigned int curr)
{
  unsigned int gcf = low;
  while (low % gcf > 0 || high % gcf > 0) {
    --gcf;
  }
  high /= gcf;
  low /= gcf;
  // example: 72/132 = 6/11
  if (curr % high < low) {
    return 1;
  }
  return 0;
}

void bpmDelay(int bpm)
{
  unsigned long baseDelay = 60000 / bpm;
  unsigned long modulo = 60000 % bpm;
  unsigned short i, dtime;

  if (lastBpm != bpm) {
    bpmCount = 0;
    lastBpm = bpm;
  }
  if (stretch(modulo, bpm, bpmCount) == 0) {
    baseDelay += 1;
  }
  baseDelay -= FLASH_TIME + 27;

  delay(baseDelay);
}

void drawNumber(int number)
{
  int i;

  for (i = 0; i < 8; i++) {
    screen_buf[i] = 0;
  }

  if (number >= 100) {
    for (i = 0; i < DIGIT_HEIGHT; i++) {
      screen_buf[i + DIGIT_START_ROW] |= hundreds[i];
    }
  }

  int tens = (number / 10) % 10;
  int ones = number % 10;

  for (i = 0; i < DIGIT_HEIGHT; i++) {
    screen_buf[i + DIGIT_START_ROW] |= digits[tens][i] << 4;
  }

  for (i = 0; i < DIGIT_HEIGHT; i++) {
    screen_buf[i + DIGIT_START_ROW] |= digits[ones][i];
  }

  pet.drawImage(screen_buf);
}

void setup() {
  pet.begin();
  pet.setScreenBrightness(8);
  pet.setState(CLICK_STATE);
}

void loop() {
  pet.update();

  // put your main code here, to run repeatedly:
  switch (pet.getState()) {
    case TEMPO_SET_STATE:
      if (pet.isBtn1Pressed()) {
        --currBpm;
        timeout = 0;
      }
      else if (pet.isBtn2Pressed()) {
        ++currBpm;
        timeout = 0;
      }
      /*
      else if (pet.isBtn1Held()) {
        currBpm -= 5;
        delay(500);
      }
      else if (pet.isBtn2Held()) {
        currBpm += 5;
        delay(500);
      }
      */
      else {
        ++timeout;
      }
      if (timeout > 100) {
        pet.setState(CLICK_STATE);
        break;
      }
      if (currBpm < 30) {
        currBpm = 30;
      }
      else if (currBpm > 199) {
        currBpm = 199;
      }
      drawNumber(currBpm);
      break;
    case CLICK_STATE:
      if (pet.isBtn1Pressed() || pet.isBtn2Pressed()) {
        timeout = 0;
        pet.setState(TEMPO_SET_STATE);
      }
      /*
	  // Disabled the beep for now so I don't annoy my coworkers
      pet.playTone(beat == 0 ? 880 : 440, 100);
      if (beat == 3) {
        beat = 0;
      }
      else {
        ++beat;
      }*/
      pet.drawImage(screen_on);
      delay(FLASH_TIME);
      pet.clearScreen();
      bpmDelay(currBpm);
      break;
  }
}
