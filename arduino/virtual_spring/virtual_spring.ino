#include <Servo.h> //see if this helps at all

int posA = A0;
int posB = A1;

int torquePin=10;
int velocityPin=10;

const int greyCodeConversion[]={0,1,3,2};

long lastValue=0;
long pos=0;

long revolutions=0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=2000;

int errors=0;

int velocity=0;

boolean serial=true;
boolean autonomous=false;

void setup()
{
  if(serial) Serial.begin(9600);
  pinMode(posA, INPUT_PULLUP);
  pinMode(posB, INPUT_PULLUP);
  pinMode(torquePin, OUTPUT);
}

void loop()
{
  updatePosition();
  if(autonomous) applyTorque();
  if(serial) performPeriodicCommunication();
  if(serial &&! autonomous) writeVelocity();
}

void applyTorque()
{
  int torque=revolutions ? 0:max(0,min(pos/600, 50));
  analogWrite(torquePin, torque);
}

void updatePosition()
{
  int newValue=readValue();
  switch(newValue-lastValue)
  {
    case 1: //rising edge
      pos++;
      break;
    case -1:
      pos--;
      break;
    case 0:
      break;
    default:
      errors++;
      break;
    
  }
  lastValue=newValue;
  
  if(pos/6000!=0)
  {
    revolutions+=pos/6000;
    pos=pos%6000;
  }
}

int readValue()
{
  int greyCode=(digitalRead(posB)<<1)|digitalRead(posA);
  //Serial.println(greyCode);
  return greyCodeConversion[greyCode];
}

void writeVelocity()
{
 if(Serial.available()>0)
  {
     int incomingDutyCycle = Serial.parseInt();
     Serial.read();  //newline
     analogWrite(velocityPin, incomingDutyCycle);
  } 
}

void performPeriodicCommunication()
{  
  unsigned long currentMillis=millis();
  if(currentMillis-lastTriggerMillis>triggerIntervalMillis)
  {
    Serial.print(pos);
    Serial.print(" r:");
    Serial.print(revolutions);
    Serial.print("\n");
    lastTriggerMillis=currentMillis;
  }
}

