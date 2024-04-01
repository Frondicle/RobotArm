/*UFactory xArm 6 clay extruder tool firmware pins
Modbus ,LCD functions
Chris Morrey 2024
*/
//--I/O-------------------------------------------------------------------------------------- 
#define TI0                3 //extruder digital output to xArm TI0, LOW for busy, red wire on standard harness
#define TI1                 4 //spare digital output to xArm TI1, purple wire on standard harness
#define TO0               5//to reset pin for arm reset control
#define TO1                 6 //digital output from robot arm: high when arm is not moving
//--Stepper----------------------------------------------------------------------------------
#define dirPinStepper       9 //stepper direction, HIGH for forward: yellow wire
#define stepPinStepper      8  //stepper pin: high/low sequence makes one step: green wire
#define stepsPerRevolution  200
