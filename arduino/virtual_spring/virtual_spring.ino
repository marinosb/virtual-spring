#include <Servo.h> //see if this helps at all

int posA = A0;
int posB = A1;

const int greyCodeConversion[]={0,1,3,2};

int lastValue=0;
int pos=0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=100;

int errors=0;

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
      pos--;
      break;
    case 0:
      break;
    default:
      errors++;
      break;
    
  }
  lastValue=newValue;
  
  unsigned long currentMillis=millis();
  if(currentMillis-lastTriggerMillis>triggerIntervalMillis)
  {
    Serial.print(pos);
    //Serial.print(" E:");
    //Serial.print(errors);
    //Serial.print("\n");
    lastTriggerMillis=currentMillis;
  }
}

int readValue()
{
  int greyCode=(digitalRead(posB)<<1)|digitalRead(posA);
  //Serial.println(greyCode);
  return greyCodeConversion[greyCode];
}

