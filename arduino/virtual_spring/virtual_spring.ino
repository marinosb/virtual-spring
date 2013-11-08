#include <Servo.h> //see if this helps at all

int posA = A0;
int posB = A1;
int posC = A2;

int torquePin=9;
int velocityPin=9;

const int greyCodeConversion[]={0,1,3,2};

int lastValue=0;
long pos=0;

int lastCValue=0;
int realRevolutions=0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=200;

int errors=0;

int velocity=0;

boolean serial=true;
boolean autonomous=true;

int rotationDirection=0;

int zeroTorque=129;

int stiffness=10;

void setup()
{
  if(serial) Serial.begin(9600);
  pinMode(posA, INPUT_PULLUP);
  pinMode(posB, INPUT_PULLUP);
  pinMode(posC, INPUT_PULLUP);
  pinMode(torquePin, OUTPUT);
  analogWrite(torquePin, zeroTorque);
}

void loop()
{
  updatePosition();
  if(autonomous) applyTorque();
  if(serial) performOutput();
  if(serial) processSerialInput();
}

void applyTorque()
{
  int torque=abs(realRevolutions)>5 ? zeroTorque:max(80,min(zeroTorque-((pos)/stiffness), 180));
  analogWrite(torquePin, torque);
}

void updatePosition()
{
  //A&B
  int newValue=readValue();
  if(newValue!=lastValue)
  {
    if(newValue==2 && lastValue==1)
    {
      pos++;
      rotationDirection=1;
    }
    else if(newValue==0 && lastValue==1)
    {
      pos--;
      rotationDirection=-1;
    }
  }
  lastValue=newValue;
  
  //C
  int newCValue=digitalRead(posC);
  if(newCValue!=lastCValue && newCValue)
  {
    //pos=0;
    realRevolutions+=rotationDirection;
    //realRevolutions++;
  }
  lastCValue=newCValue;
}

int readValue()
{
  int greyCode=(digitalRead(posB)<<1)|digitalRead(posA);
  //Serial.println(greyCode);
  return greyCodeConversion[greyCode];
}

void processSerialInput()
{
 if(Serial.available()>0)
  {
    char x=Serial.read();
    if(x=='v')
    {
      int incomingDutyCycle = Serial.parseInt();
     autonomous=false;
     analogWrite(velocityPin, incomingDutyCycle);
    }
    else if(x=='r')
    {
      realRevolutions=0;
      pos=0;
      autonomous=true;
    }
    else if(x=='s')
    {
      stiffness = Serial.parseInt();
    }
     Serial.read();  //newline
  } 
}

void performOutput()
{  
  unsigned long currentMillis=millis();
  if(currentMillis-lastTriggerMillis>triggerIntervalMillis)
  {
    Serial.print(pos);
    Serial.print(" R:");
    Serial.print(realRevolutions);
    Serial.print(" D:");
    Serial.print(rotationDirection);
    Serial.print("\n");
    lastTriggerMillis=currentMillis;
  }
}

