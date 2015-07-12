// States
#define NUMBER_OF_STATES 3
#define CENTER 0
#define LEFT 1
#define RIGHT 2

// Outputs
#define BOTH_MOTORS 0x03
#define RIGHT_MOTOR_ONLY 0x01
#define LEFT_MOTOR_ONLY 0x02

// Input Calibration
#define CENTER_LEFT_SENSOR 900  // center define the mid point between sensing the line or not.  Value less than the center indicate on line.
#define CENTER_RIGHT_SENSOR 900
#define LEFT_SENSOR_PIN A0
#define RIGHT_SENSOR_PIN A1

#define DEFAULT_DELAY 10 // 10 milliseconds
#define DEFAULT_MOTOR_SPEED 0x50

#define NUMBER_OF_INPUT_COMBINATIONS 4
enum PossibleInputs {LOST, OFF_TO_THE_LEFT, OFF_TO_THE_RIGHT, ON_LINE};

typedef struct StateData {
  uint8_t output;
  uint16_t timeDelay;
  uint8_t nextState[NUMBER_OF_INPUT_COMBINATIONS]; // Index of next state
} StateInfo;

// StateInfo fsm[NUMBER_OF_STATES] = {
//   {BOTH_MOTORS,DEFAULT_DELAY,{RIGHT, LEFT, RIGHT, CENTER}},  // CENTER
//   {LEFT_MOTOR_ONLY,DEFAULT_DELAY,{LEFT, CENTER, RIGHT, CENTER}},  // OFF TO LEFT
//   {RIGHT_MOTOR_ONLY,DEFAULT_DELAY,{RIGHT, LEFT, CENTER, CENTER}}   // Off to Right
// };
StateInfo fsm[NUMBER_OF_STATES] = {
  {BOTH_MOTORS,DEFAULT_DELAY,{RIGHT, LEFT, RIGHT, CENTER}},  // CENTER
  {LEFT_MOTOR_ONLY,DEFAULT_DELAY,{LEFT, LEFT, RIGHT, CENTER}},  // OFF TO LEFT
  {RIGHT_MOTOR_ONLY,DEFAULT_DELAY,{RIGHT, LEFT, RIGHT, CENTER}}   // Off to Right
};
