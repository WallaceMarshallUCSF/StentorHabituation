/*
  Stentor Habituator
  Output to magnet circuit is on pin 9
  Input from button to pin 2 (other end of button to ground)
  Input from switch to pin 6 (other end of switch to ground)
 */

/********** Constants to set **********/
// Time (in seconds) to wait between automatic pulses
long delaytime = 59.9;
// Time (in milliseconds) to pulse magnet
int pulselength = 500;
// Long blink length of the LED (milliseconds)
int longblinklength = 1000;
// Short blink length of the LED (milliseconds)
int shortblinklength = 100;
// Time before pulse to start long blinking (seconds)
int longblinkbeforepulse = 10;
// Time before pulse to start short blinking (seconds)
int shortblinkbeforepulse = 3;
// Time to wait after prior pulse before checking button press (milliseconds)
// changed from version 3 to make it be 1 minute long so that we can prevent kids from pushing the button again and agian
unsigned long debouncedelay = 60000;
// Time to wait before double checking switch input (milliseconds)
int switchdelaytime = 100;


// Define pins used to drive the onboard LED
// and the magnet circuit
// Magnet circuit should have a flywheel diode for safety
short led = 13;
short magnet = 9;
short button = 2;
short modeswitch = 6;
/********** End constants ************/

// Global variables
unsigned long loopstart;
unsigned long nextpulsetime;
unsigned long ledontime;
unsigned long ledofftime;
unsigned long ledactualontime;
boolean automaticmode;
boolean ledon;
// initialize lastdebouncetime to guarantee you can push button after 1 minute initially
unsigned long lastdebouncetime = 0;
unsigned long lastcountdowntime;


// the setup routine runs once when you press reset:
void setup() {                
  // initialize output digital pins
  pinMode(led, OUTPUT);
  pinMode(magnet, OUTPUT);
  // initialize input digital pins and connect internal pull-up resistors
  pinMode(button, INPUT);
  digitalWrite(button, HIGH);
  pinMode(modeswitch, INPUT);
  digitalWrite(modeswitch, HIGH);
  // Change times defined in seconds to milliseconds
  delaytime = delaytime * 1000;
  longblinkbeforepulse = longblinkbeforepulse * 1000;
  shortblinkbeforepulse = shortblinkbeforepulse * 1000;
  
  Serial.begin(9600);
  Serial.println("Program Started");
  // Initialize variables
  automaticmode = true;
  ledon = false;
  lastdebouncetime=millis();
  initializeautomaticpulseloop();
}

void initializeautomaticpulseloop(){
  loopstart=millis();
  nextpulsetime=loopstart + delaytime;
  turnoffled();
  ledontime = nextpulsetime-longblinkbeforepulse;
  lastcountdowntime=loopstart+1;
  Serial.println("Automatic mode activated");
}

// the loop routine runs over and over again forever:
void loop() {
  // Check to see if it's time to pulse
  if (automaticmode==true && millis()>=nextpulsetime) {
    pulse();
    initializeautomaticpulseloop();
  }
  
  // Check to see if LED needs to be turned on for countdown
  if (automaticmode==true && ledon==false && \
      millis()>=ledontime) {
        turnonled();
        ledactualontime=millis();
        // Set LED off time based on whether in slow or fast blink period
        if (ledactualontime>=(nextpulsetime-longblinkbeforepulse) && \
            ledactualontime< (nextpulsetime-shortblinkbeforepulse)) {
              ledofftime=ledactualontime+longblinklength;
              ledontime=ledofftime+longblinklength;
            }
        else if (ledactualontime>=(nextpulsetime-shortblinkbeforepulse)) {
           ledofftime=ledactualontime+shortblinklength;
           ledontime=ledofftime+shortblinklength;
        }
        // Check if current LED pulse ends after magnet pulse
        // If so shut off LED and reset ontime to the future
        if (ledofftime>=nextpulsetime) {
          turnoffled();
          ledontime=nextpulsetime+delaytime;  
        }
      }
      
  // Check to see if LED needs to be turned off
  if (ledon==true && millis()>=ledofftime){
    turnoffled(); 
  }
  
  // Check to see if modeswitch has changed from auto to manual
  if (automaticmode==true && digitalRead(modeswitch)==LOW) {
     // Delay and then double check reading
     delay(switchdelaytime);
     if (digitalRead(modeswitch)==LOW) {
     automaticmode=false;
     turnonled();
     ledofftime=millis()+longblinklength;
     Serial.println("Manual mode activated");
  }}
  
  // Check to see if modeswitch has changed from manual to auto
  if (automaticmode==false && digitalRead(modeswitch)==HIGH) {
     // Delay and then double check reading
     delay(switchdelaytime);
     if (digitalRead(modeswitch)==HIGH) {
     automaticmode=true;
     initializeautomaticpulseloop();
  }}
  
  // Check for manual pulse button
  if (automaticmode==false && digitalRead(button)==LOW && \
      millis()>=(lastdebouncetime+debouncedelay)) {
        pulse();
        Serial.println("sending manual pulse");
        lastdebouncetime=millis();
  }
  
  // Slow blink LED to indicate manual mode
  if (automaticmode==false && ledon==false && millis()>=ledontime && millis()>=(lastdebouncetime+debouncedelay)) {
    turnonled();
    ledofftime=millis()+longblinklength;
    ledontime=ledofftime+longblinklength;
  }
  
  // Write countdown time to console
  // not sure if we really want to do this since it is annoying
  if (automaticmode==true && (lastcountdowntime+10000)<=millis()) {
    lastcountdowntime=lastcountdowntime+10000;
    Serial.print((nextpulsetime-millis())/1000);
    Serial.println(" seconds till pulse");
  }
}

// Send pulse through magnet
// Uses delay() to help ensure consistent pulse timing without other ongoing polling
void pulse() {
  Serial.println("Pulsing magnet");
  digitalWrite(magnet, HIGH);
  delay(pulselength);
  digitalWrite(magnet, LOW);
}

void turnonled(){
  digitalWrite(led,HIGH);
  ledon=true;
}

void turnoffled(){
  digitalWrite(led,LOW);
  ledon=false;
}

