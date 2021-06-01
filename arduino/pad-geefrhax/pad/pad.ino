/*For more information see www.ladyada.net/learn/sensors/fsr.html */

#include <map>

#define ENABLE_EEPROM

#ifdef ENABLE_EEPROM
# include <EEPROM.h>
#endif

// Define to log fsr values to serial (slow)
#define DEBUG_FSR_VALUES

// TODO: LED Output totally broken right now
// #define ENABLE_LEDS

//Sides: left 0, right 1
#define SIDE '0'

#define BASE_PRESSURE  0

#define CALIBRATE_LOW_THRESHOLD 150
#define CALIBRATE_MEDIUM_THRESHOLD 350
#define CALIBRATE_HIGH_THRESHOLD 450

#define INITIAL_LEFT_PRESSURE  1000
#define INITIAL_DOWN_PRESSURE  1000
#define INITIAL_RIGHT_PRESSURE 1000
#define INITIAL_UP_PRESSURE    1000

#ifdef ENABLE_LEDS
int LED_pins[4] = {0, 2, 3, 1};
#endif

// geefr's pad
// analog
// - 0 up
// - 1 right
// - 2 down
// - 3 left
// - 4 up2?
// - 5 right2?
// - 6 Down2?
// - 7 Left2?

// The number of pins used for the panels
enum Direction {
  Left,
  Down,
  Right,
  Up
};

std::map<Direction, int> dirPinMap;
std::map<int, int> pinValueMap;
std::map<int, bool> pinStateMap;
std::map<int, int> pinPressureMap;
std::map<int, int> pinCalibrationMap;


// int LURD_pins[4] = {3, 0, 1, 2};
int LURD_values[4] = {0, 0, 0, 0};
int LURD_state[4] = {0, 0, 0, 0};
int LURD_pressures[4] = {INITIAL_LEFT_PRESSURE, INITIAL_UP_PRESSURE, INITIAL_RIGHT_PRESSURE, INITIAL_DOWN_PRESSURE};
int LURD_calibration_pressures[4] = {0, 0, 0, 0};
int startupCalibrationThreshold = CALIBRATE_MEDIUM_THRESHOLD;

//Vibration detection calibration mode: VDCM
bool VDCM_enabled = false;
int VDCM_vibrationMaxConsecutiveReads = 10;
int VDCM_LURD_consecutiveReads[4] = {0, 0, 0, 0};
int VDCM_LURD_maxPressureValues[4] = {0, 0, 0, 0};
int VDCM_vibrationPadding = 20;

int oldValueWeight = 1;
float releaseMultiplier = 0.9f;

char LURD_Keys[4] = {1, 2, 3, 4};
const unsigned int MAX_INPUT = 50;

#ifdef ENABLE_EEPROM
// If enabled make the contents of LURD_pressures persistent
// Note that EEPROM can hande ~100k writes, so hopefully
// that's enough given infrequent calibrations
// If it wears out just increment this by sizeof(EEPromData) to shift along the block
unsigned int EEPromAddress = 0x00;

/// Data structure to write to eeprom
struct EEPromData {
  int LPressure = 0;
  int UPressure = 0;
  int RPressure = 0;
  int DPressure = 0;
};

void saveCalibration() {
  EEPromData data;
  
  data.LPressure = LURD_pressures[0];
  data.UPressure = LURD_pressures[1];
  data.RPressure = LURD_pressures[2];
  data.DPressure = LURD_pressures[3];
  
  for( auto i = 0u; i < 4; ++i ) {
    EEPROM.put(EEPromAddress, data);
  }
  
  // Serial.printf("Calibration Saved: %i, %i, %i, %i\n", LURD_pressures[0], LURD_pressures[1], LURD_pressures[2], LURD_pressures[3] ); 
}

void loadCalibration() {
  EEPromData data;
  EEPROM.get(EEPromAddress, data);
  LURD_pressures[0] = data.LPressure;
  LURD_pressures[1] = data.UPressure;
  LURD_pressures[2] = data.RPressure;
  LURD_pressures[3] = data.DPressure;
  
  // Serial.printf("Calibration Loaded: %i, %i, %i, %i\n", LURD_pressures[0], LURD_pressures[1], LURD_pressures[2], LURD_pressures[3] );
}

#endif

void setup(void) {
  Serial.begin(9600);
#ifdef ENABLE_LEDS
  setupLedOutputs();
#endif  
  calibrate();
}

#ifdef ENABLE_LEDS  
void setupLedOutputs()
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(LED_pins[i], OUTPUT);
  }

}
#endif

void initDataForCalibration() {
  for (int i = 0; i < 50; i++)
  {
    
    
    for( auto& p : pinValueMap )
    {
      p.second = (p.second * oldValueWeight + analogRead(LURD_pins[i])) / (oldValueWeight + 1);
    }
    
    delay(20);
  }
  memcpy(&LURD_calibration_pressures, &LURD_values, sizeof LURD_values) ;
}

void calibrate() {
  initDataForCalibration();
#ifdef ENABLE_EEPROM
  loadCalibration();
#else
  setCalibrationThresholds(startupCalibrationThreshold);
#endif
  //printPressures();
}

void setCalibrationThresholds(int threshold)
{
  for (int i = 0; i < 4; i++)
  {
    LURD_pressures[i] = LURD_values[i] + threshold;
  }
}

// process data after null terminator is received
void process_data (char * data)
{
  //do some string parsing  
  data[4]=0;
  int index = data[0]-48;

  //pad side query (9) or set pressures (0-3)???
  if (index == 9) {
	  //I'm right or left? #Defined at the start of the file
    Serial.println(SIDE);
  } else if (index == 21) {
      //Given command char was 'E'eanble vibration detection calibration mode
      VDCM_enabled = true;
  } else if (index == 20) {
      //Given command char was 'D'isable vibration detection calibration mode
      VDCM_enabled = false;
  }
  else
  {
    if (index == 19) {
      //Given command char was 'C'alibrate (ASCII value 67 - 48 -> index 19). Wow, such amazing code.

      int threshold = atoi((const char *)&(data[1]));
      setCalibrationThresholds(threshold);
#ifdef ENABLE_EEPROM
      saveCalibration();
#endif
    }
    else if (index < 5)
    {
      LURD_pressures[index] = atoi((const char *)&(data[1]));
#ifdef ENABLE_EEPROM
      saveCalibration();
#endif
    }

    printPressures();
  }
}

void printPressures()
{
  Serial.print("L pressure: ,");
  if (LURD_pressures[0] < 100) Serial.print("0");
  if (LURD_pressures[0] < 10) Serial.print("0");
  Serial.print(LURD_pressures[0]);
  Serial.println(",");

  Serial.print("U pressure: ,");
  if (LURD_pressures[1] < 100) Serial.print("0");
  if (LURD_pressures[1] < 10) Serial.print("0");
  Serial.print(LURD_pressures[1]);
  Serial.println(",");

  Serial.print("R pressure: ,");
  if (LURD_pressures[2] < 100) Serial.print("0");
  if (LURD_pressures[2] < 10) Serial.print("0");
  Serial.print(LURD_pressures[2]);
  Serial.println(",");

  Serial.print("D pressure: ,");
  if (LURD_pressures[3] < 100) Serial.print("0");
  if (LURD_pressures[3] < 10) Serial.print("0");
  Serial.print(LURD_pressures[3]);
  Serial.println(",");
  Serial.println("");
}


void processIncomingByte (const byte inByte)
{
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;

  switch (inByte)
  {

    case '\n':   // end of text
      input_line [input_pos] = 0;  //null terminator
      
      process_data (input_line);
      
      input_pos = 0;  
      break;

    case '\r':   // discard carriage return
      break;

    default:
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;

  } 
} 

void VDCM_pressed(int LURD_index)
{
  if (LURD_values[LURD_index] > VDCM_LURD_maxPressureValues[LURD_index])
  {
    VDCM_LURD_maxPressureValues[LURD_index] = LURD_values[LURD_index];
  }
  VDCM_LURD_consecutiveReads[LURD_index]++;
}

void VDCM_pressReleased(int LURD_index)
{
  if (VDCM_LURD_consecutiveReads[LURD_index] == 0) return;

  //Was the released panel press shorter than what is considered a vibration?
  if (VDCM_LURD_consecutiveReads[LURD_index] < VDCM_vibrationMaxConsecutiveReads)
  {
    //Set the threshold above the vibration max pressure value
    LURD_pressures[LURD_index] = VDCM_LURD_maxPressureValues[LURD_index] + VDCM_vibrationPadding;

    //Debug
    /*
    Serial.println("Vibration detected:");

    Serial.print("  LURD_index: ");
    Serial.println(LURD_index);

    Serial.print("  Consecutive reads before release: ");
    Serial.println(VDCM_LURD_consecutiveReads[LURD_index]);

    Serial.print("  Pressure value set to: ");
    Serial.println(LURD_pressures[LURD_index]);
    */
  }

  VDCM_LURD_maxPressureValues[LURD_index] = 0;
  VDCM_LURD_consecutiveReads[LURD_index] = 0;
}



void updateAnalogValues(float dT) {
  for (int i = 0; i < 4; i++)
  {
    LURD_values[i] = (LURD_values[i] * oldValueWeight + analogRead(LURD_pins[i])) / (oldValueWeight + 1);

    float padGradient = static_cast<float>(LURD_values[i]) / dT;
    auto borderValue = (LURD_pressures[i] + BASE_PRESSURE);

    float rocLevel = 1000.0;
    
    if( padGradient > rocLevel ) {
      if (LURD_state[i] == 0)
      {
        Joystick.button(LURD_Keys[i], 1);
#ifdef ENABLE_LEDS
        digitalWrite(LED_pins[i], HIGH);
#endif        
        LURD_state[i] = 1;
      }
    }
    else if( padGradient < - rocLevel || LURD_values[i] < borderValue )
    {
if (VDCM_enabled) VDCM_pressReleased(i);
      if (LURD_state[i] == 1 && LURD_values[i] < borderValue * releaseMultiplier)
      {
        Joystick.button(LURD_Keys[i], 0);
#ifdef ENABLE_LEDS
        digitalWrite(LED_pins[i], LOW);
#endif
        LURD_state[i] = 0;
      }
    } 
  }
}

int counter = 0;
uint32_t lastUpdate = 0;
void loop(void) {

  counter = (counter+1) % 50;
  if (counter == 0 && Serial.available() > 0)
  {
    processIncomingByte(Serial.read());
  }

  auto now = millis();
  updateAnalogValues( now - lastUpdate );
  lastUpdate = now;

#ifdef DEBUG_FSR_VALUES
    if(counter % 50 == 0) {
    int fsrReading = analogRead(0);
    Serial.print(fsrReading);
    Serial.print(";");
    
    fsrReading = analogRead(1);
    Serial.print(fsrReading);
    Serial.print(";");
  
    fsrReading = analogRead(2);
    Serial.print(fsrReading);
    Serial.print(";");
  
    fsrReading = analogRead(3);
    Serial.print(fsrReading);
    Serial.print(";");

    Serial.print(LURD_state[0]);
    Serial.print(";");
    Serial.print(LURD_state[1]);
    Serial.print(";");
    Serial.print(LURD_state[2]);
    Serial.print(";");
    Serial.print(LURD_state[3]);
    Serial.print(";");

    Serial.println(";");
    
  }
#endif
}
