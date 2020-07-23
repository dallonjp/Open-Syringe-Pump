
#include <AccelStepper.h>
#define dirPin 2 //digital pin 2 on arduino connects to dir pin on A4988
#define stepPin 3 //digital pin 3 on arduino connects to step pin on A4988
#define homeButton 5 //digital pin 5 on arduino connects to normally closed (green) wire on endstop
#define motorInterfaceType 1

char dirinput[1];
long positionX;
long init_homing=-1;
long mspeed;
long maccel;
int mdir;
long mpos;

int msg_length;
bool connected =false;
bool datarvd = false;
char  userInput[17]; 
char userInput2[6];//={'g','0','0','0','0','0'};

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

void setup()
{  
    //set up serial connection
    Serial.begin(9600);
    Serial.setTimeout(1000); 
    //set defaults
    stepper.setCurrentPosition(0);
    stepper.setMaxSpeed(3000);
    stepper.setAcceleration(2000);
    
}
void loop()
{    
    while(Serial.available()>0){
      
      char handshake = Serial.read();
      //Serial.print(handshake);
      switch (handshake){
        case 'a':
        //connect to app
          delay(100);
          Serial.println("hello winform");
          delay(100);
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
        case 'd':
        //stop current operation
        break;

        default:

        break;
      }
    }
}
void moveto(){
  while (!datarvd){
    if (Serial.readBytes(userInput,17)==17){
      for(int i=1; i<8;i++){
        positionX=positionX*10+userInput[i]-'0';
        //Serial.println(userInput[i]);
        }
        for (int j=9;j<15;j++){
          mspeed=mspeed*10+userInput[j]-'0';
        }
        dirinput[0]=char(userInput[16]);
        mdir = atoi(dirinput);      
        if(mdir==1){
          mpos = positionX*1;
        }else if (mdir==2){
          mpos = positionX*-1;
        }
            stepper.move(mpos);
            stepper.setMaxSpeed(mspeed);
            while(stepper.distanceToGo()!=0){
              stepper.run();
              if (Serial.available()>0){
                break;
              }
            }
        positionX=0;
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

    //Serial.print("Pump motor is homing....");
    //move in -x until endstop is pressed
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
    //move in +x until endstop is no longer pressed 
    while (!digitalRead(homeButton)){
      stepper.moveTo(init_homing);
      stepper.run();
      init_homing++;
    }
    //set zero position
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
