int posA = 52;
int posB = 50;
int posC = 48;

int torquePin=DAC0;
int velocityPin=A7;

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

int stiffness=90;
int dampingFactor=3;

int lastTorque=zeroTorque;

//veclocity calculation
long calculatedVelocityTicks;
long velocityLastPos;
unsigned long lastVelocitySampleMillis;

void setup()
{
  if(serial) Serial.begin(115200);
  pinMode(posA, INPUT_PULLUP);
  pinMode(posB, INPUT_PULLUP);
  pinMode(posC, INPUT_PULLUP);
  pinMode(torquePin, OUTPUT);
  analogWrite(torquePin, zeroTorque);
  //pinMode(velocityPin, INPUT);
}

void loop()
{
  updatePosition();
  //readVelocityAnalog();
  calculateVelocity();
  if(autonomous) applyTorque();
  if(serial) performOutput();
  if(serial) processSerialInput();
}

void applyTorque()
{
  int adjPos=pos;
  int linearComponent=((adjPos*10)/stiffness);
  int velocityComponent= calculatedVelocityTicks/(dampingFactor/100);
  
  
  int torque=abs(realRevolutions)>20 ? zeroTorque:max(1,min(zeroTorque-linearComponent+velocityComponent, 254));
  
  //save us the extra write
  if(torque!=lastTorque)
  {
    analogWrite(torquePin, torque);
    lastTorque=torque;
  }
  
}

void calculateVelocity()
{
  unsigned long currentTimeMillis=millis();
  if( abs(currentTimeMillis-lastVelocitySampleMillis) >10 )
  {
    lastVelocitySampleMillis=currentTimeMillis;
    calculatedVelocityTicks=(pos-velocityLastPos)+0*calculatedVelocityTicks;
    velocityLastPos=pos;
  }
  
}

void readVelocityAnalog()
{
  velocity=analogRead(velocityPin);
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

void processSerialInput()
{
 if(Serial.available()>0)
  {
    char x=Serial.read();
    if(x=='v')
    {
      int incomingDutyCycle = Serial.parseInt();
     autonomous=false;
     analogWrite(torquePin, incomingDutyCycle);
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
    else if(x=='d')
    {
      dampingFactor=Serial.parseInt();
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
    //Serial.print(" V:");
    //Serial.print(velocity);
    Serial.print(" S:");
    Serial.print(calculatedVelocityTicks);
    Serial.print("\n");
    lastTriggerMillis=currentMillis;
  }
}

