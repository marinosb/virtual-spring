int torquePin=DAC0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=1000;

boolean serial=true;
boolean autonomous=true;

int rotationDirection=0;

//For intinal position
int zeroTorque=1950;
int zeroGravity=1735;

double stiffness=16.0/600.0;


int lastTorque=zeroTorque;

//veclocity calculation
int calculatedVelocityTicks;
int velocityLastPos;
unsigned long lastVelocitySampleMillis;
int velocitySampleTime=5;

double coulombFactor=0;
double c1Factor=0;
double c2Factor=0;
double c3Factor=0;

boolean overspeed=false;

int zeroPosition=0;
int cpuPosition=0;

int error=0;

int torqueOverride=zeroTorque;

unsigned long cachedMillis;

#define forward 0
#define reverse 1

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
  applyTorque();
  if(serial) performOutput();
  if(serial) processSerialInput();
}

void updatePosition()
{
  cpuPosition=((int)REG_TC0_CV0)-zeroPosition;
  rotationDirection=((REG_TC0_QISR>>8)&0x1);
  //error=(REG_TC0_QISR>>2)&0x1;
}

//int sign(int x)
//{
//  return x>0?1:(x<0?-1:0);
//}

int sign(double x)
{
  return x>0?1:(x<0?-1:0);
}

void applyTorque()
{
  //scale: 1/1
  int adjPos=cpuPosition;
  
  double yDot=((double)calculatedVelocityTicks)/25.0;
  double y=adjPos;

  double coulombComponent=0;
  double linearComponent=y;
  double yComponent=0;
  double yDotComponent=0;
  double yDot2Component=0;
  double yDot3Component=0;

  int torque=zeroTorque;

  if(autonomous)
  {
    double yDot2=yDot*yDot;
    double yDot3=yDot2*yDot;
    
    if(sign(yDot)>0) //down
    {
      coulombComponent=14.121;
      yDotComponent=14.022 * yDot;
      yDot2Component=0.308 * yDot2;
      yDot3Component=-0.3307* yDot3;
    }
    else //up
    {
      coulombComponent=-14.121;
      yDotComponent=14.022 * yDot;
      yDot2Component=-0.308 * yDot2;
      yDot3Component=-0.3307 * yDot3;
    }

    double output=
      -linearComponent*stiffness
      +yDotComponent*(c1Factor) *3.79252
        +yDot2Component*(c2Factor)  *3.79252
          +yDot3Component*(c3Factor) *3.79252
            +coulombComponent*(coulombFactor) *3.79252;
            

    torque=max(1,min(1804.02 + output, 4094)); 
  }
  else
  {
    torque=torqueOverride;
  }

  //safety checks
  if(torque>=4000 || torque<=100 || cpuPosition>7000 || cpuPosition<-9500 || error !=0 || calculatedVelocityTicks>900)
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
      torqueOverride=incomingDutyCycle;
    }
    else if(x=='r')
    {
      zeroPosition=cpuPosition+zeroPosition;
      velocityLastPos=0;
      cpuPosition=0;
      torqueOverride=zeroTorque;
      autonomous=true;
      overspeed=false;
    }
    else if(x=='s')
    {
      double stiffnessIncoming=Serial.parseInt();
      stiffness = 16.0/stiffnessIncoming;
    }
    else if(x=='p')
    {
      int posOffset=Serial.parseInt();
      zeroPosition+=posOffset;
    }
    else if(x=='d')
    {
      c1Factor=getPercentageFromInput();
    }
    else if(x=='c')
    {
      coulombFactor=getPercentageFromInput();
    }
    else if(x=='q')
    {
      c2Factor=getPercentageFromInput();
    }
    else if(x=='b')
    {
      c3Factor=getPercentageFromInput();
    }
    Serial.read();  //newline
  } 
}

double getPercentageFromInput()
{
  int value=Serial.parseInt();
  double percentage=((double)value)/100.0;
  return percentage;
}

int perfSpeedTicks=0;
void performOutput()
{  
  perfSpeedTicks++;
  unsigned long currentMillis=cachedMillis;
  if(currentMillis-lastTriggerMillis>triggerIntervalMillis)
  {
    if(perfSpeedTicks<1000) overspeed=true;
    if(overspeed) Serial.print("!ERR ");
    Serial.print(cpuPosition);
    Serial.print(" T:");
    Serial.print(lastTorque);
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

