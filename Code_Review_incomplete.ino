//Soccer Robots Team 21 Code Review
//z5255884 Nathan Trinh
//--------------------------------------------------------------

//Pins
//==============================================================

//L298N Motor Controller
//Motor one
int enA = 10;
int in1 = 8;
int in2 = 9;
//Motor two
int enB = 5;
int in3 = 7;
int in4 = 6;

//Piston
int pistonCharge = 2;
int pistonFire = 3;

//Pixy
#include <Pixy2.h> //Pixy2 Arduino Library
Pixy2 pixy;

//Defence/Offence switch
int defoff = 4;

//Globals
//==============================================================
bool moving = false; //indicator for if the robot is currently moving
long ticksCharged = 0; //how long the capacitor has been charging

//Initialisation
//==============================================================
void setup() {
  Serial.begin(115200);
  //Print pin locations into serial monitor
  Serial.println("Motor controller | 6-10");
  Serial.println("Compass          | A4,A5");
  Serial.println("Piston           | 2,3");
  Serial.println("Defense/Offence  | 4");

  //Initialise pin settings
  //motor strength
  pinMode(enA, OUTPUT); //integer [0,255]
  pinMode(enB, OUTPUT); //integer [0,255]
  //motor directionals
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  //Set startup defaults
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  
  //Verify initialisation of Pixy
  Serial.println("Initialising Pixy...");
  pixy.init();
  Serial.println("Camera Initialised.");

  //Initialise piston activator pins
  pinMode(pistonCharge, OUTPUT);
  pinMode(pistonFire, OUTPUT);
  //Set default states
  digitalWrite(pistonCharge, LOW);
  digitalWrite(pistonFire, LOW);

  //Initialise defence/offense signal pin
  pinMode(defoff, INPUT); //HIGH = offence LOW = defence

}
//Is the ball on screen?
//=============================================================
bool ballFound() {
  //Locals
  float area = 0;
  int maxArea = 0;
  bool result = false;
  int ballArea = 0;

  //Scan for signature blocks
  pixy.ccc.getBlocks();

  //Enumerate any blocks
  for (int i = 0; i < pixy.ccc.numBlocks; i++) {
    //Is it the ball
    if (pixy.ccc.blocks[i].m_signature == 1) {
      area = pixy.ccc.blocks[i].m_width * pixy.ccc.blocks[i].m_height;
    
      //Is it the biggest block
       if ((area > 400) && (area > maxArea)) {
         maxArea = area;
         ballArea = area;
         result = true;
      }
    }
    return result;
  }
}

//Ball location by bearing [0,316]
//=============================================================
int ballBearing() {
  //Locals
  int bearing = 0;

  //Get blocks
  pixy.ccc.getBlocks();

  //Determine where the ball in on the x-axis
  for (int i = 0; i < pixy.ccc.numBlocks; i++) {
    bearing = pixy.ccc.blocks[i].m_x;
    return bearing;
  }
}

//Ball distance judged by width of detected block [0,316]
//=============================================================
int ballDistance() {
  //Locals
  int distance = 0;

  //Get blocks
  pixy.ccc.getBlocks();

  //Determine how wide block is
  for (int i=0; i < pixy.ccc.numBlocks; i++) {
    distance = pixy.ccc.blocks[i].m_width;
    return distance;
  }
}

//Piston activate
//=============================================================
void pistonActivate() {
  //Cut off charging
  digitalWrite(pistonCharge, LOW);
  //Fire
  digitalWrite(pistonFire, HIGH);
  ticksCharged = 0;
  //Wait 0.1 sec
  delay(100);
  //Reset signal
  digitalWrite(pistonFire, LOW);

}


//=============================================================
//WHEEL POWER PROFILES
//=============================================================


//Spin clockwise
//=============================================================
void spinAroundCW() {
  //Set slower speed for both motors so ball isn't blurry
  analogWrite(enA, 50);
  analogWrite(enB, 50);
  //Set forward for motor A
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  //Set backward for motor B
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

//Turn wheels off
//=============================================================
void wheelsOFF() {
  //Set motor speeds to zero
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  //Set all outputs to LOW
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

//Move forward
//=============================================================
void wheelsDEFAULT() {
  //Set motor speeds to medium speed
  analogWrite(enA, 200);
  analogWrite(enB, 200);
  //Set both wheels to forward
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

//Full power forward
//=============================================================
void wheelsPOWER() {
  //Set motor speeds to maximum speed
  analogWrite(enA, 255);
  analogWrite(enB, 255);
  //Set both wheels to forward
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

//Inch left
//=============================================================
void inchLEFT() {
  //Reduce power in left wheel
  analogWrite(enA, 150);
  analogWrite(enB, 200);
  //Make sure both wheels are forward
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

//Inch right
//=============================================================
void inchRIGHT() {
  //Reduce power in right wheel
  analogWrite(enA, 200);
  analogWrite(enB, 150);
  //Make sure both wheels are forward
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);  
}

//Turn CW **needs engine power tuning**
//=============================================================
void turnCW() {
  //Ensure power states are equal in both wheels
  analogWrite(enA, 100);
  analogWrite(enB, 100);
  //Reverse power in right wheel
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  //Forward power in left wheel
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
}

//Turn ACW **needs engine power tuning**
//=============================================================
void turnACW() {
  //Ensure power states are equal in both wheels
  analogWrite(enA, 100);
  analogWrite(enB, 100);
  //Reverse power in left wheel
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  //Forward power in right wheel
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}


//Main Loop
//=============================================================
void loop() {
  //Begin charging capacitor unless charged
  if (ticksCharged <= 4500) {
    digitalWrite(pistonCharge, HIGH);
    ticksCharged ++;
  }
  else {
    digitalWrite(pistonCharge, LOW);
  }
  int currentMode = digitalRead(defoff);
  //Defence mode
  if (currentMode = LOW){
    //find the ball
    ballFound();
    if (ballFound() == false) {
      spinAroundCW; //find ball routine
    }
    else if (ballFound() == true) { //if ball is found
      int bearing = ballBearing(); //find out where the ball is
      //keep ball in centre of camera
      if (bearing < 148) {
        turnACW();
      }
      else if (bearing > 168) {
        turnCW();
      }
      //wait until ball is 300mm away
      else if (bearing >= 148 && bearing <= 168) {
        int distance = ballDistance();
        if (distance <= 79) { //**NEEDS TO BE TUNED TO 300mm
          wheelsPOWER(); //charge at the ball
          delay(300);
          turnCW(); 
          delay(500); //**NEEDS TO BE TUNED TO A HALF TURN
          wheelsDEFAULT();
          delay(400); //**NEEDS TO BE TUNED TO HOW FAR IT TRAVELLED
          wheelsOFF();
          delay(5000);
        }
      }
    }
  }
  //Offence mode
  else if (currentMode = HIGH) { 
    //Check if ball has been detected
    ballFound();
    if (ballFound() == false) {
      spinAroundCW(); //run find ball routine
      moving = false; //tell system it isn't currently moving
    }
    else if (ballFound == true) { 
      int bearing = ballBearing();
      //Centre ball with error of +-10 pixels
      if (bearing < 148) {
        if (moving == false) {
          turnACW();
        }
        else if (moving == true) {
          inchLEFT();
        }
      }
      else if (bearing > 168) {
        if (moving == false) {
          turnCW();
        }
        else if (moving == true) {
          inchRIGHT();
        }
      }
      else if (bearing >= 148 && bearing <= 168) { //if centred move forward and begin checking distance of ball
        wheelsDEFAULT(); //move ball forward
        moving = true; //tell system it is currently moving
        int distance = ballDistance;
        if (distance >= 253) { //if ball comes close enough kick it
          pistonActivate();        
        }
      }
    }
  }
}
