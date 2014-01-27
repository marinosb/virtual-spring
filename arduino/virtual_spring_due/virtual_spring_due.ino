int torquePin=DAC0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=1000;

boolean serial=true;
boolean autonomous=true;

int rotationDirection=0;

int zeroTorque=1950;

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

int zeroPosition=0;
int cpuPosition=0;

int error=0;

unsigned long cachedMillis;

void setup()
{
  delay(1000);
  if(serial) Serial.begin(115200);
  
  setupQam();
  
  analogWriteResolution(12);
  
  pinMode(torquePin, OUTPUT);
  analogWrite(torquePin, zeroTorque);
}

void setupQam()
{
        // Setup Quadrature Encoder with Marker
  REG_PMC_PCER0 = (1<<27); // activate clock for TC0
  REG_PMC_PCER0 = (1<<28); // activate clock for TC1
  
  // select XC0 as clock source and set capture mode
  REG_TC0_CMR0 = 5; 
  
// activate quadrature encoder and position measure mode, no filters
    REG_TC0_BMR = (1<<9)|(1<<8); //|(1<<12)
    
    // select XC0 as clock source and set capture mode
    REG_TC0_CCR0 = (1<<2) | (1<<0) | (1<<14);
}

void loop()
{
  cachedMillis=millis();
  updatePosition();
  
  calculateVelocity();
  if(autonomous) applyTorque();
  if(serial) performOutput();
  if(serial) processSerialInput();
}

void updatePosition()
{
  cpuPosition=((int)REG_TC0_CV0)-zeroPosition;
  rotationDirection=((REG_TC0_QISR>>8)&0x1);
  //error=(REG_TC0_QISR>>2)&0x1;
}

int sign(int x)
{
  return x>0?1:(x<0?-1:0);
}

void applyTorque()
{
  //scale: 1/1
  int adjPos=cpuPosition;
  int linearComponent=((adjPos*16)/stiffness);
  int velocityComponent= (calculatedVelocityTicks*10*16)/dampingFactor;
  int coulombComponent=(sign(calculatedVelocityTicks)*coulombFactor);
  
  int torque=max(1,min(zeroTorque-linearComponent+velocityComponent+coulombComponent, 4094));
  
  //safety checks
  if(cpuPosition>12000 || cpuPosition<-15500 || error !=0 || calculatedVelocityTicks>800)
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

void calculateVelocity()
{
  unsigned long currentTimeMillis=cachedMillis;
  if( abs(currentTimeMillis-lastVelocitySampleMillis) >velocitySampleTime )  
  {
    lastVelocitySampleMillis=currentTimeMillis;
    calculatedVelocityTicks=(cpuPosition-velocityLastPos);
    velocityLastPos=cpuPosition;
  }
  
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
      zeroPosition=cpuPosition+zeroPosition;
      velocityLastPos=0;
      cpuPosition=0;
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
      zeroPosition+=posOffset;
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

int perfSpeedTicks=0;
void performOutput()
{  
  perfSpeedTicks++;
  unsigned long currentMillis=cachedMillis;
  if(currentMillis-lastTriggerMillis>triggerIntervalMillis)
  {
    if(perfSpeedTicks<100000) overspeed=true;
    if(overspeed) Serial.print("!ERR ");
    Serial.print(cpuPosition);
    Serial.print(" D:");
    Serial.print(rotationDirection);
    Serial.print(" S:");
    Serial.print(calculatedVelocityTicks);
    Serial.print(" H:");
    Serial.print(perfSpeedTicks);
    Serial.print("\n");
    lastTriggerMillis=currentMillis;
    perfSpeedTicks=0;
  }
}

