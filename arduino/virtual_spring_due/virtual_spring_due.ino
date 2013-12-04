int posA = 52;
int posB = 50;
int posC = 48;

int torquePin=DAC0;
int velocityPin=DAC0;

const int greyCodeConversion[]={0,1,3,2};

boolean lastValueA=0;
boolean lastValueB=0;
long pos=0;

int lastCValue=0;
int realRevolutions=0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=1000;

int errors=0;

int velocity=0;

boolean serial=true;
boolean autonomous=true;

int rotationDirection=0;

int zeroTorque=127;

int stiffness=10;

int lastTorque=zeroTorque;

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
  int torque=abs(realRevolutions)>20 ? zeroTorque:max(1,min(zeroTorque-((pos*10)/stiffness), 254));
  
  //save us tjhe extra write
  if(torque!=lastTorque)
  {
    analogWrite(torquePin, torque);
    lastTorque=torque;
  }
  
}

void updatePosition()
{
  //A&B
  boolean newValueA=digitalRead(posA);
  if(newValueA!=lastValueA)
  {
    boolean newValueB=digitalRead(posB);
    if(!newValueA && (newValueB!=lastValueB))
    {
      rotationDirection=newValueB?1:-1;
      pos+=rotationDirection;
    }
    lastValueB=newValueB;
  }
  lastValueA=newValueA;
  
  //C
  int newCValue=digitalRead(posC);
  if(newCValue!=lastCValue && !newCValue)
  {
    realRevolutions+=rotationDirection;
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
    else if(x=='p')
    {
      int posOffset=Serial.parseInt();
      pos+=posOffset;
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

