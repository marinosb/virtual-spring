
int zeroPosition=0;
int cpuPosition=0;
unsigned long lastVelocitySampleMillis;
int calculatedVelocityTicks;
int velocityLastPos;
int velocitySampleTime=5; //5ms
int torque=0;
int rotationDirection=0;

unsigned long lastTriggerMillis=0;
int triggerIntervalMillis=1000;



unsigned long cachedMillis=0;

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(115200);
  
  setupQam();
}

void loop()
{
  cachedMillis=millis();
  readTorque();
  updatePosition();
  
  calculateVelocity();
  performOutput();
  processSerialInput();
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

void readTorque()
{
  torque=analogRead(A0);
}

void updatePosition()
{
  cpuPosition=((int)REG_TC0_CV0)-zeroPosition;
  rotationDirection=((REG_TC0_QISR>>8)&0x1);
  //error=(REG_TC0_QISR>>2)&0x1;
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
    
    if(x=='r')
    {
      zeroPosition=cpuPosition+zeroPosition;
      velocityLastPos=0;
      cpuPosition=0;
    }
  }
}

int perfSpeedTicks=0;
void performOutput()
{  
  perfSpeedTicks++;
  unsigned long currentMillis=cachedMillis;
  if(currentMillis-lastTriggerMillis>triggerIntervalMillis)
  {
    Serial.print(cpuPosition);
    Serial.print(" T:");
    Serial.print(torque);
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


