#define   SMP_RATE              10UL           // 10 Hz ( 0.1 sec. gate)
#define   CLK_MAIN       84000000UL
#define   TMR_CNTR       CLK_MAIN / (2 *SMP_RATE)

// Name - Port - Pin(DUE board) 
// TCLK1 = PA4 = AD5
// TIOA1 = PA2 = AD7
// TIOB1 = PA3 = AD6

volatile uint32_t freq = 0;  
volatile uint32_t flag = 0;  

         int debug_osm = 0; // debug over serial monitor 
         int debug_cnt = 0; // debug variable duty period 

void setup()
{
  Serial.begin (115200); 

  Atmr_setup();         
  Btmr_setup();         

  pio_TIOA0();  // Gate OUT, Dig. 2 conect Dig-2 to AN-7.
  pio_TCLK1();  // Counter IN, AN - 5 (max 3.3V -> keep safe !!!)
  pio_TIOA1();  // Gate IN, AN - 7.
  pinMode( 7, OUTPUT); // OUT, Digital 7 -> connect to AN-5 for Testing
}

void loop() 
{
  char in_Byte;
  
  if( Serial.available() > 0 ) {
    in_Byte = Serial.read();
    if( in_Byte == 'd' ) { //debug           
      debug_osm = 1 - debug_osm;
      if (debug_osm) Serial.println(F("\n\tDebug activated"));
      else           Serial.println(F("\n\tDebug de-activ."));
      }
    }

  if( flag && debug_osm ) {
       Serial.print("\n\tfreq: ");
       Serial.print(freq,  DEC);  
       flag = 0;
    }

  debug_cnt += 10;
  if( debug_cnt > 255 ) debug_cnt = 0;
  analogWrite( 7, debug_cnt);
}


void TC1_Handler(void)
{
if ((TC_GetStatus(TC0, 1) & TC_SR_LDRBS) == TC_SR_LDRBS) {
  uint32_t dummy = TC0->TC_CHANNEL[1].TC_RA;
           freq  = TC0->TC_CHANNEL[1].TC_RB;
           flag  = 1;
  }
}

void Atmr_setup() // Gate 1 sec., conect Dig-2 to AN-7.
{
  pmc_enable_periph_clk (TC_INTERFACE_ID + 0 *3 + 0);
  TcChannel * t = &(TC0->TC_CHANNEL)[0];
  t->TC_CCR = TC_CCR_CLKDIS;
  t->TC_IDR = 0xFFFFFFFF;   
  t->TC_SR;                 
  t->TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 | 
              TC_CMR_WAVE |                
              TC_CMR_WAVSEL_UP_RC |        
              TC_CMR_EEVT_XC0 |    
              TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
              TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR;
  t->TC_RC = TMR_CNTR;
  t->TC_RA = TMR_CNTR /2;
  t->TC_CMR = (t->TC_CMR & 0xFFF0FFFF) | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET; 
  t->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; 
}

void Btmr_setup()  // Counter
{
  pmc_enable_periph_clk (TC_INTERFACE_ID + 0 *3 + 1);

  uint32_t reg_val = TC_BMR_TC1XC1S_TCLK1; // Input Capture Pin, AN-5.
  TC0->TC_BMR |= reg_val; 

  TcChannel * t = &(TC0->TC_CHANNEL)[1];
  t->TC_CCR = TC_CCR_CLKDIS;
  t->TC_IDR = 0xFFFFFFFF;   
  t->TC_SR;   
  t->TC_CMR = TC_CMR_TCCLKS_XC1   
             | TC_CMR_LDRA_RISING     
             | TC_CMR_LDRB_FALLING    
             | TC_CMR_ABETRG          
             | TC_CMR_ETRGEDG_FALLING;
             
  t->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; 

  t->TC_IER =  TC_IER_LDRBS;
  t->TC_IDR = ~TC_IER_LDRBS;

  NVIC_DisableIRQ(TC1_IRQn);
  NVIC_ClearPendingIRQ(TC1_IRQn);
  NVIC_SetPriority(TC1_IRQn, 0); 
  NVIC_EnableIRQ(TC1_IRQn);
}

void pio_TIOA0()
{
  PIOB->PIO_PDR   = PIO_PB25B_TIOA0;  
  PIOB->PIO_IDR   = PIO_PB25B_TIOA0;  
  PIOB->PIO_ABSR |= PIO_PB25B_TIOA0;
}

void pio_TCLK1() 
{
  PIOA->PIO_PDR = PIO_PA4A_TCLK1;  
  PIOA->PIO_IDR = PIO_PA4A_TCLK1;
}

void pio_TIOA1() 
{
  PIOA->PIO_PDR = PIO_PA2A_TIOA1;  
  PIOA->PIO_IDR = PIO_PA2A_TIOA1;
}
