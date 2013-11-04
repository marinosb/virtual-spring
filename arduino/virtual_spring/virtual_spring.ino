#include <Servo.h> //see if this helps at all

int posA = A0;
int posB = A1;

const int grayCodeConversion[]={0,1,3,2};

int lastValue=0;
int pos=0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=100;

void setup()
{
  Serial.begin(9600);
  pinMode(posA, INPUT_PULLUP);
  pinMode(posB, INPUT_PULLUP);
}

void loop()
{  
  int newValue=readValue();
  switch(newValue-lastValue)
  {
    case 1: //rising edge
      pos++;
      break;
    case -1:
      break;
    case 0:
      break;
    default:
      Serial.println("Error");
    
  }
  lastValue=newValue;
  
  unsigned long currentMillis=millis();
  if(currentMillis-lastTriggerMillis>triggerIntervalMillis)
  {
    Serial.println(pos);
    lastTriggerMillis=currentMillis;
  }
}

int readValue()
{
  return digitalRead(posA);
}

