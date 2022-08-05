#undef HID_ENABLED

// Arduino Due ADC->DMA->USB 1MSPS
// by stimmer
// from http://forum.arduino.cc/index.php?topic=137635.msg1136315#msg1136315
// Input: Analog in A0
// Output: Raw stream of uint16_t in range 0-4095 on Native USB Serial/ACM

// on linux, to stop the OS cooking your data: 
// stty -F /dev/ttyACM0 raw -iexten -echo -echoe -echok -echoctl -echoke -onlcr

volatile int bufn,obufn;
uint16_t buf[4][256];   // 4 buffers of 256 readings

void ADC_Handler(){     // move DMA pointers to next buffer
  int f=ADC->ADC_ISR;
  if (f&(1<<27)){
   bufn=(bufn+1)&3;
   ADC->ADC_RNPR=(uint32_t)buf[bufn];
   ADC->ADC_RNCR=256;
  } 
}

void setup(){
  SerialUSB.begin(0);
  Serial.begin(115200);
  //while(!SerialUSB);
  pmc_enable_periph_clk(ID_ADC);
  adc_init(ADC, SystemCoreClock, 2000000, ADC_STARTUP_FAST);
  ADC->ADC_MR |=0x80; // free running

  ADC->ADC_CHER=0x80; 

  NVIC_EnableIRQ(ADC_IRQn);
  ADC->ADC_IDR=~(1<<27);
  ADC->ADC_IER=1<<27;
  ADC->ADC_RPR=(uint32_t)buf[0];   // DMA buffer
  ADC->ADC_RCR=256;
  ADC->ADC_RNPR=(uint32_t)buf[1]; // next DMA buffer
  ADC->ADC_RNCR=256;
  bufn=obufn=1;
  ADC->ADC_PTCR=1;
  ADC->ADC_CR=2;
}

unsigned long _time = millis();
uint16_t last_sample = 0;
unsigned long count = 0;
int consec_count = 0, last_consec_count = 0;
const int n = 100;
uint16_t j = 0;

void loop(){
  while(obufn==bufn); // wait for buffer to be full
  uint16_t avgbuf[n];
  for(int i=0; i < 256; i++)
  {
    avgbuf[j%n] = buf[obufn][i];
    j++;
    unsigned long avg = 0;
    for(int k=0; k < n ; k++)
      avg += avgbuf[k];
    avg /= n;
    avg = (avg > 2048 ? 4096 : 0);
    buf[obufn][i] = avg;
    if (avg == last_sample)
      consec_count ++;
    else
    {
      if (consec_count > 10 & last_consec_count > 10)
      {
        count ++;
        buf[obufn][i] = 512;
      }
      last_consec_count = consec_count;
      consec_count = 0;
    }
    last_sample = avg;
  }
  SerialUSB.write((uint8_t *)buf[obufn],512); // send it - 512 bytes = 256 uint16_t
  if(millis() - _time > 300)
  {
    Serial.println(count*2);
    count = 0;
    _time = millis();
  }
  obufn=(obufn+1)&3; 
}
