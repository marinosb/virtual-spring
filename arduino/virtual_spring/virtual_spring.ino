#include <Servo.h> //see if this helps at all

int posA = A0;
int posB = A1;

const int grayCodeConversion[]={0,1,3,2};

int lastValue=0;
int pos=0;

void setup()
{
  Serial.begin(9600);
  pinMode(posA, INPUT);
  pinMode(posB, INPUT);
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
  
  if(millis()%1000)
  {
    Serial.println(pos);
  }
}

int readValue()
{
  return digitalRead(posA);
}
