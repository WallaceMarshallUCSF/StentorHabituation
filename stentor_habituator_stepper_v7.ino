/****************************************************************************** 

v7 change enable and disable so that they apply after every step i.e. if it is in enable
then it always is left in enable after a step and if it is in disable mode then it is always left 
disabled after a step

v6 fixed direction of taps to go down first and then up
v5 added raise and lower function
v4  control settings for automatic mode added

overall contorl flow is based on the previous electromaget version of the Stentor habituation
written by Wallace Marshall  with contributions to software from Kyle Barlow and Patrick Harrigan


motor control was based on the:  "SparkFun Big Easy Driver Basic Demo"
Toni Klopfenstein @ SparkFun Electronics
February 2015
https://github.com/sparkfun/Big_Easy_Driver

Simple demo sketch to demonstrate how 5 digital pins can drive a bipolar stepper motor,
using the Big Easy Driver (https://www.sparkfun.com/products/12859). Also shows the ability to change
microstep size, and direction of motor movement.

Example based off of demos by Brian Schmalz (designer of the Big Easy Driver).
http://www.schmalzhaus.com/EasyDriver/Examples/EasyDriverExamples.html
******************************************************************************/
//Declare pin functions on Arduino
#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define MS3 6
#define EN  7
#define GreenLED 8
#define RedLED 9
short button = 11;
short modeswitch = 10;
int switchdelaytime = 100;
// Time (in seconds) to wait between automatic pulses
long delaytime = 10; // default is 59.9 for 1 minute
String user_input_string;
float time_minutes; // used for inputting time delay in minutes



//Declare variables for functions
char user_input;
int x;
int y;
int z;
int state;
int stepsize; // microstep size 1=full 2=half 3=1/4 4=1/8 5=1/16

// Global variables
unsigned long loopstart;
unsigned long nextpulsetime;
unsigned long ledontime;
unsigned long ledofftime;
unsigned long ledactualontime;
boolean automaticmode;
boolean buttonmode;
boolean ledon;
// initialize lastdebouncetime to guarantee you can push button after 1 minute initially
unsigned long lastdebouncetime = 0;
unsigned long lastcountdowntime;
boolean enablemode;



void setup() {
  //motor controls
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);
  //switches and LEDs for control panel
  pinMode(GreenLED,OUTPUT);
  pinMode(RedLED,OUTPUT);
  pinMode(button, INPUT); // set internal pull up
  digitalWrite(button, HIGH); // set internal pull up
  pinMode(modeswitch, INPUT);
  digitalWrite(modeswitch, HIGH);
// Change times defined in seconds to milliseconds
  delaytime = delaytime * 1000;

  
  resetBEDPins(); //Set step, direction, microstep and enable pins to default states
                  //BED refers to Big Easy Driver
  Serial.begin(9600); //Open Serial connection for debugging
  Serial.println("Begin Stentor Habituation Device operation");
  Serial.println();
  //Print function list for user selection
  Serial.println("Enter number for control option:");
  Serial.println("i. initialize automatic mode settings.");
  Serial.println("e. enable motor to lock arm ");
  Serial.println("d. disable motor so you can move the arm.");
  Serial.println("r. raise the arm by a step.");
  Serial.println("l. lower the arm by a step");
  Serial.println("h. print help menu");
  Serial.println();

  stepsize = 4;  //default take a 1/8 microstep
  automaticmode = false;
  buttonmode = false;
  enablemode = false; // default is motor is disabled when not stepping
}

void initializeautomaticpulseloop(){
  loopstart=millis();
  nextpulsetime=loopstart + delaytime;
  lastcountdowntime=loopstart+1;
  Serial.println("Pulse delivered, new cycle starting");
}

//Main loop
void loop() {



  if (automaticmode==false && digitalRead(modeswitch)==HIGH) {
       // Delay and then double check reading
       delay(switchdelaytime);
      if (digitalRead(modeswitch)==HIGH) {
      digitalWrite(GreenLED,HIGH);
      // switch on auto mode
      automaticmode = true;
      Serial.println("Automatic Mode Activated");
  }}
  if (automaticmode==true &&digitalRead(modeswitch)==LOW) {
       // Delay and then double check reading
       delay(switchdelaytime);
      if (digitalRead(modeswitch)==LOW) {
      digitalWrite(GreenLED,LOW);
      automaticmode = false;
      Serial.println("Automatic Mode Inactivated");
  }}

  if (buttonmode == false && digitalRead(button)==LOW) {
    Serial.println("button pressed");
       // Delay and then double check reading
       delay(switchdelaytime);
      if (digitalRead(modeswitch)==LOW) {
      
      digitalWrite(EN, LOW); //Pull enable pin low to set FETs active and allow motor control
      if (stepsize ==1)
      {
         LargeTap();
      }
      else if(stepsize ==2)
      {
         SmallTap();
      }
      else if(stepsize ==3)
      {
        SmallStepMode4();
      }
      else if(stepsize ==4)
      {
        SmallStepMode8();
      }
      else if(stepsize ==5)
      {
        SmallStepMode16();
      }
      else
      {
        
        Serial.println("Invalid step size");
      }
      resetBEDPins();
      buttonmode = true; // prevents a second button press until delaytime later
  }}

  if (buttonmode == true && digitalRead(button)==HIGH) {
       // Delay and then double check reading
       delay(switchdelaytime);
      if (digitalRead(modeswitch)==LOW) {
      
      
      buttonmode = false; // can't trigger button till its been released
  }}
  
  while(Serial.available()){
      user_input = Serial.read(); //Read user input and trigger appropriate function
      //user_input_string = Serial.readString();
      
      if (user_input =='i')
      {
         delay(1000);
         Serial.println("enter the step size:  1, 2, 3, 4 or 5");
         Serial.println("where 1 = full, 2=half, 3=1/4, 4=1/8 and 5=1/16");
         while (Serial.available()==0){}             // wait for user input
           stepsize = Serial.parseInt(); 
           Serial.println("step size set to ");
           Serial.println(stepsize);

         delay(1000);
         Serial.println("enter the stimulus period in minutes");
         while (Serial.available()==0){}             // wait for user input
           time_minutes = Serial.parseFloat(); 
           Serial.println("time in minutes set to ");
           Serial.println(time_minutes);
           // convert from minutes to milliseconds
           delaytime = time_minutes*60*1000;
           Serial.println(delaytime);
      }
      else if (user_input =='d')
      {
        digitalWrite(EN, HIGH); // disable current to motor so it can be turned
        Serial.println("motor disabled ");
        enablemode = false;
      }
      else if (user_input =='e')
      {
        digitalWrite(EN, LOW); // restore current to motor so it locks
        Serial.println("motor enabled ");
        enablemode = true;
      }
      else if (user_input =='l')
      {
        
        Serial.println("microstep down ");
        
        digitalWrite(dir, HIGH); //Pull direction pin low to move "backwards" which is down
        digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
        digitalWrite(MS2, HIGH);
        digitalWrite(MS3, HIGH);
        digitalWrite(EN, LOW);
        for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
        {
          digitalWrite(stp,HIGH); //Trigger one step forward
          delay(1);
          digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
          delay(1);
        }
        resetBEDPins();
      }
      else if (user_input =='r')
      {
        
        Serial.println("microstep up ");
        
        digitalWrite(dir, LOW); //Pull direction pin low to move "forward" which is up 
        digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
        digitalWrite(MS2, HIGH);
        digitalWrite(MS3, HIGH);
        digitalWrite(EN, LOW);
        for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
        {
          digitalWrite(stp,HIGH); //Trigger one step forward
          delay(1);
          digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
          delay(1);
        }
        resetBEDPins();
      }
      else if (user_input == 'h') // help option
      {
         Serial.println("Enter number for control option:");
         Serial.println("i. initialize automatic mode settings.");
         Serial.println("e. enable motor to lock arm ");
         Serial.println("d. disable motor so you can move the arm.");
         Serial.println("r. raise the arm by a step.");
         Serial.println("l. lower the arm by a step");
         Serial.println("h. print help menu");
         Serial.println();
      }
      
  }

// Check to see if it's time to pulse
  // first check if we need to set warning light 1 second before pulse
  if (nextpulsetime - millis() < 1000) {
    digitalWrite(RedLED,HIGH);
  } else
  {
    digitalWrite(RedLED,LOW);
  }
  if (millis()>=nextpulsetime) {
    //buttonmode = false; // reset button timer to allow manual activation
    if (automaticmode == true) {
      digitalWrite(EN, LOW); //Pull enable pin low to set FETs active and allow motor control
    if (stepsize ==1)
      {
         LargeTap();
      }
      else if(stepsize ==2)
      {
         SmallTap();
      }
      else if(stepsize ==3)
      {
        SmallStepMode4();
      }
      else if(stepsize ==4)
      {
        SmallStepMode8();
      }
      else if(stepsize ==5)
      {
        SmallStepMode16();
      }
      else
      {
        
        Serial.println("Invalid step size");
      }
      resetBEDPins();
    initializeautomaticpulseloop();
    } // execute if automatic mode is true
  }

  
}

//Reset Big Easy Driver pins to default states
void resetBEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
  //digitalWrite(EN, HIGH);
  if (enablemode) {
  digitalWrite(EN, LOW); //this allows the armature to hold position after a tap
  } 
  else
  {
    digitalWrite(EN, HIGH); // disable motor so it won't jitter between taps
  }
}

//Default microstep mode function
void LargeTap()
{
  Serial.println("Moving forward at default step mode.");
  digitalWrite(dir, HIGH); //Pull direction pin low to move "forward"
  for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  
  Serial.println("Moving in reverse at default step mode.");
  digitalWrite(dir, LOW); //Pull direction pin high to move in "reverse"
  for(x= 1; x<10; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  
  Serial.println("Enter new option");
  Serial.println();
}


void SmallTap()
{
  Serial.println("Moving forward at default step mode.");
  digitalWrite(dir, HIGH); //Pull direction pin low to move "forward"
  for(x= 1; x<2; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  
  Serial.println("Moving in reverse at default step mode.");
  digitalWrite(dir, LOW); //Pull direction pin high to move in "reverse"
  for(x= 1; x<2; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  
  Serial.println("Enter new option");
  Serial.println();
}




//Reverse default microstep mode function
void ReverseStepDefault()
{
  Serial.println("Moving in reverse at default step mode.");
  digitalWrite(dir, HIGH); //Pull direction pin high to move in "reverse"
  for(x= 1; x<1000; x++)  //Loop the stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("Enter new option");
  Serial.println();
}


// 1/4th microstep foward mode function
void SmallStepMode4()
{
  Serial.println("Stepping at 1/4th microstep mode.");
  digitalWrite(dir, HIGH); //Pull direction pin low to move "forward"
  digitalWrite(MS1, LOW); //Pull MS1,MS2, and MS3 high to set logic to 1/4th microstep resolution
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, LOW);
  for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
   digitalWrite(dir, LOW); //Pull direction pin low to move "reverse"
  for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("Enter new option");
  Serial.println();
}


// 1/8th microstep foward mode function
void SmallStepMode8()
{
  Serial.println("Stepping at 1/8th microstep mode.");
  digitalWrite(dir, HIGH); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, LOW);
  for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
   digitalWrite(dir, LOW); //Pull direction pin low to move "reverse"
  for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("Enter new option");
  Serial.println();
}



// 1/16th microstep foward mode function
void SmallStepMode16()
{
  Serial.println("Stepping at 1/16th microstep mode.");
  digitalWrite(dir, HIGH); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);
  for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
   digitalWrite(dir, LOW); //Pull direction pin low to move "reverse"
  for(x= 1; x<10; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  Serial.println("Enter new option");
  Serial.println();
}

//Forward/reverse stepping function
void ForwardBackwardStep()
{
  Serial.println("Alternate between stepping forward and reverse.");
  for(x= 1; x<5; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    //Read direction pin state and change it
    state=digitalRead(dir);
    if(state == HIGH)
    {
      digitalWrite(dir, LOW);
    }
    else if(state ==LOW)
    {
      digitalWrite(dir,HIGH);
    }
    
    for(y=1; y<1000; y++)
    {
      digitalWrite(stp,HIGH); //Trigger one step
      delay(1);
      digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
      delay(1);
    }
  }
  Serial.println("Enter new option");
  Serial.println();
}
