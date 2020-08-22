

#include <AccelStepper.h>
#include <FlexiTimer2.h>
#define dirPin 2 //digital pin 2 on arduino connects to dir pin on A4988
#define stepPin 3 //digital pin 3 on arduino connects to step pin on A4988
#define homeButton 5 //digital pin 5 on arduino connects to normally closed (green) wire on endstop
#define motorInterfaceType 1

long steps;
long init_homing=-1;
long stepto;
long mspeed;
long maccel;
long mdir;
long mpos;
long interval;
long elapsed=0;
long defaultrate;
long defaultaccel;
long timenow;
long timethen;

int msg_length;
bool connected =false;
bool datarvd = false;
char  userInput[20]; 
char userInput2[6];

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

void setup()
{  
    //set up serial connection
    Serial.begin(9600);
    Serial.setTimeout(1000); 
    pinMode(dirPin,OUTPUT);
    pinMode(stepPin,OUTPUT);
}
void loop()
{    
    while(Serial.available()>0){
      
      char handshake = Serial.read();
      switch (handshake){
        case 'a':
        //connect to app
          delay(100);
          Serial.println("hello winform");
          delay(100);
          getdefaults();
        break;
        case 'b':
        //home sequence
        homestepper();
        break;
        case 'c':
        //movement
          moveto();
        break;
        case 'e':
        //change acceleration
          changeaccel();
        break;
        case 'f':
        //timed injection
        timedaction();
        break;
        case 'd':
        //stop current operation
        break;

        default:

        break;
      }
    }
}
void timedaction(){
  while (!datarvd){
    if(Serial.readBytes(userInput,20)==20){
      for (int i=1; i<10; i++){
        steps=steps*10+userInput[i]-'0';
      }
      for (int j=11;j<20;j++){
          interval=interval*10+userInput[j]-'0';
        }
      elapsed=0;
      stepto=stepper.currentPosition()-1;
      stepper.setMaxSpeed(10000);
      stepper.setAcceleration(5000);
      while (elapsed<interval){
        if (Serial.available()>0){
          interval=0;
          steps=0;
          break;
        }else{
          delay(steps);
          stepper.moveTo(stepto);  
          stepper.run();
          elapsed=elapsed+steps;
          stepto=stepto-1;
        }
      }
      interval=0;
      steps=0;
      datarvd=true;
    }
  }
  datarvd=false;
}

void moveto(){
  while (!datarvd){
    if (Serial.readBytes(userInput,17)==17){
      for(int i=1; i<8;i++){
        steps=steps*10+userInput[i]-'0';
        }
        for (int j=9;j<15;j++){
          mspeed=mspeed*10+userInput[j]-'0';
        }
        mdir = userInput[16]-'0';      
        if(mdir==1){
          mpos = steps*1;
        }else if (mdir==2){
          mpos = steps*-1;
        }
            stepper.move(mpos);
            stepper.setMaxSpeed(mspeed);
            while(stepper.distanceToGo()!=0){
              stepper.run();
              if (Serial.available()>0){
                break;
              }
            }
        steps=0;
        mspeed=0;
        mdir=0;
        datarvd=true;
    }
  }
  datarvd=false;
}
void homestepper(){
  //home sequence
    init_homing=-1;
    pinMode(homeButton, INPUT_PULLUP);

    delay(5);
    //These speed/acceleration values are for 1/16 step on the A4988 (MS1, MS2, MS3 connected to +5V)
    stepper.setMaxSpeed(3000.0);
    stepper.setAcceleration(3000.0);

    while (digitalRead(homeButton)){
      stepper.moveTo(init_homing);
      init_homing--;
      stepper.run();
     // delay(5);
    }
    stepper.setCurrentPosition(0);
    stepper.setMaxSpeed(3000.0);
    stepper.setAcceleration(3000.0);
    //Serial.print(init_homing);
    init_homing=1;

    while (!digitalRead(homeButton)){
      stepper.moveTo(init_homing);
      stepper.run();
      init_homing++;
    }

    stepper.setCurrentPosition(0);
    Serial.write("yup hom");  
}
void changeaccel(){
  while (!datarvd){
    if (Serial.readBytes(userInput,6)==6){
      for(int i=1; i<7;i++){
        maccel=maccel*10+userInput[i]-'0';
        }
            stepper.setAcceleration(maccel);
        maccel=0;
        datarvd=true;
    }
  }
  datarvd=false;
}
void getdefaults(){
  while (!datarvd){
    if (Serial.readBytes(userInput,20)==20){
      for (int i=1; i<10; i++){
        defaultaccel=defaultaccel*10+userInput[i]-'0';
      }
      for (int j=11;j<20;j++){
        defaultrate=defaultrate*10+userInput[j]-'0';
      }
      stepper.setAcceleration(defaultaccel);
      stepper.setMaxSpeed(defaultrate);
      defaultaccel=0;
      defaultrate=0;
      datarvd=true;
    }
  }
  datarvd=false;
}
