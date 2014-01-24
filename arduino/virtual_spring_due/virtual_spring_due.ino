int posA = 52;
int posB = 50;
int posC = 48;

int torquePin=DAC0;
int velocityPin=A7;

boolean lastValueA=0;
boolean lastValueB=0;
int pos=0;

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
int dampingFactor=10000;

int lastTorque=zeroTorque;

//veclocity calculation
int calculatedVelocityTicks;
int velocityLastPos;
unsigned long lastVelocitySampleMillis;
int velocitySampleTime=10;

int coulombFactor=0;

boolean overspeed=false;

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
  //scale: 1/4
  int adjPos=pos;
  int linearComponent=((adjPos*4)/stiffness);
  int velocityComponent= (calculatedVelocityTicks)/dampingFactor;
  int coulombComponent=sign(calculatedVelocityTicks)*coulombFactor;
  
  int torque=max(1,min(zeroTorque-linearComponent+velocityComponent+coulombComponent, 254));
  
  if(realRevolutions>20 ||realRevolutions<-20)
  {
    overspeed=true;
  }
  
  if(overspeed)
  {
    torque=zeroTorque;
  }
  
  //save us the extra write
  if(torque!=lastTorque)
  {
    analogWrite(torquePin, torque);
    lastTorque=torque;
  }
  
}

int sign(int x)
{
  return x>0?1:(x<0?-1:0);
}

void calculateVelocity()
{
  unsigned long currentTimeMillis=millis();
  if( abs(currentTimeMillis-lastVelocitySampleMillis) >velocitySampleTime )  
  {
    lastVelocitySampleMillis=currentTimeMillis;
    calculatedVelocityTicks=((pos-velocityLastPos)*40)+calculatedVelocityTicks/2;
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
      overspeed=false;
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
    else if(x=='c')
    {
      coulombFactor=Serial.parseInt();
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

