/* Author Name : Kumar Gaurav Singh
   Roll no. : 108118058
   Domain: Embedded and Analog Electronics
   Sub-Domain: Embedded Systems
   Functions: delay_ms, usart_init, usart_send, usart_send_packedBCD, i2c_init, i2c_start_cond, i2c_stop_cond, i2c_send, i2c_receive,rtc_init, 
               rtc_set_time, rtc_get_time, display_current_time, BCD_to_DEC, DEC_to_BCD, setting_clock_time, set_alarm_or_timer_time,timer2_init, main
   Global Variables: set_hour, set_min, set_sec, alarm_hour, alarm_min, alarm_sec, timer_hour, timer_min, timer_sec */
 
// CPU Clock frequency ---> 16MHz
// RTC Module used ---> DS1307

#include <avr/io.h> //Include AVR standard header file
unsigned char set_hour, set_min, set_sec;  //Declaring Global Variables
unsigned char alarm_hour, alarm_min, alarm_sec;
unsigned char timer_hour, timer_min,timer_sec;
 /* Function Name: delay_ms
    Input: Time of delay required in milliseconds
    Logic: generates required delay through AVR Timer1
    Example Call: delay_ms(200);*/
    
void delay_ms( uint16_t delayTime)
{
  uint16_t NumOfTics;
  NumOfTics= delayTime*15.625 ;
  //Initializing timer 1
  TCCR1B= (1<<WGM12)|(1<<CS12)|(1<<CS10);//1024 bit prescaling, CTC mode
  TCNT1= 0;
  OCR1A= NumOfTics - 1;
  while(!(TIFR1 & (1<<OCF1A))){ }
  TIFR1|= (1<<OCF1A); //Clearing Output compare match flag
}
 /* Function Name: usart_init
    Logic: Initializes USART
    Example Call: usart_init();
  */
void usart_init()
{
  UCSR0A=0x00;
  UCSR0B= (1<<TXEN0); //Enabling transmitter
  UCSR0C= (1<<UCSZ01)|(1<<UCSZ00); // Setting number of data bits as 8
  UBRR0= 103; //Setting baud rate as 9600 bps. UBBR= Fosc/(16*BAUD)-1, Fosc--> System clock frequency= 16MHz
}
/* Function Name: usart_send
   Input: Data(unsigned chr) to be sent
   Logic: Sends data through USART Transmitter
   Example Call: usart_send(':'); */
void usart_send(unsigned char data)
{
  while(!( UCSR0A & (1<<UDRE0))){ } // Wait while the UDRE0 flag is set and transmit buffer (UDR0) is empty to receive new data.
  UDR0= data;
}

/* Function Name: usart_send_packedBCD
   Input:  BCD Data(unsigned char) to be sent
   Logic: Converts packed BCD to unpacked BCD and then to ASCII for transmission
   Example Call: usart_send_packedBCD() */
   
void usart_send_packedBCD(unsigned char data) // To convert packed BCD to ASCII before transmitting
{
  usart_send( '0' + (data>>4));
  usart_send( '0' + (data & 0x0F));
}
/* Function Name: i2c_init
   Logic: Enables TWI and sets Serial Clock(SCL) frequency
   Example Call_ i2c_init(); */
   
void i2c_init()
{ 
  TWSR= 0x00; //Setting prescaler bits to 0
  TWBR= 150; // SCL frequency is about 50k for this value of TBBR and 16MHz CPU Clock frequency. SCL freq= (CPU Clock freq)/(16 + 2*(TBBR)*(Prescaleer value))
  TWCR= (1<<TWEN); 
}
/* Function Name: i2c_start_cond
   Logic: Transmits I2C protocol start condition
   Example Call: i2c_start_cond(); */
void i2c_start_cond()
{
  TWCR= (1<<TWEN)|(1<<TWSTA)|(1<<TWINT);
  while( !(TWCR & (1<<TWINT))){ }
}
/* Function Name: i2c_stop_cond
   Logic: Transmits I2C protocol stop condition
   Example Call: i2c_stop_cond(); */
void i2c_stop_cond()
{
  TWCR= (1<<TWEN)|(1<<TWSTO)|(1<<TWINT);
}
/* Function Name: i2c_send
   Input: Data to be sent to master or slave via SDL
   Logic:To send data via TWI in Serial Data Line (SDL) */
void i2c_send(unsigned char data)
{
  TWDR= data;
  TWCR= (1<<TWEN)|(1<<TWINT);
  while(!(TWCR & (1<<TWINT))){}
} 
/* Function Name: i2c_receive
   Input: Acknowledgemt to be sent(ack) or not(nack) after data reception
   Logic: receives data from Master or Slave via SDL
   Example Call: i2c_receive(1); */
unsigned char i2c_receive(unsigned char ack)
{
 TWCR= (1<<TWEN)|(1<<TWINT)|(ack<<TWEA);
 while(!(TWCR & (1<<TWINT))){}
 return TWDR; 
 }
 /* Function Name: rtc_init
    Logic: Initialzes Real-Time Clock(RTC) Module DS1307 by sending start condition and slave address and setting the control register
    Example Call: rtc_init(); */
void rtc_init()
{
  i2c_init(); //Initializing I2C
  i2c_start_cond(); //Transmitting start condition
  i2c_send(11010000); //Sending slave address(1101000) followed by write bit(0)
  i2c_send(0x07); // Setting the register pointer to 0x07(control register)
  i2c_send(0x00); // Setting value of location 7 to 0
  i2c_stop_cond(); // Transmitting stop condition
  }
 /*Function name: rtc_set_time
   Input: hour, min, sec. That is, the time to be set
   Logic: Sets the time of the RTC
   Example Call: rtc_set_time(set_hour, set_min, set_sec); */
void rtc_set_time(unsigned char hour,unsigned char minute,unsigned char sec)
{
  i2c_start_cond();//Transmitting start condition
  i2c_send(11010000); //Sending slave address of DS1307(1101000) followed by Write bit(0)
  i2c_send(0); //Setting register pointer to 0;
  i2c_send(sec); //Set seconds
  i2c_send(minute);//Set minutes
  i2c_send(hour);//Set hours;
  i2c_stop_cond();
}
/* Function Name: rtc_get_time
   Input: Addresses of variables storing received hour, min, sec from RTC
   Logic: Gets the time from the RTC
   Example Call: rtc_get_time( &set_hour, &set_min, &set_sec); */
void rtc_get_time(unsigned char *hour ,unsigned char *minute ,unsigned char *sec)
{
  i2c_start_cond();
  i2c_send(11010000);
  i2c_send(0); //Setting register pointer to 0
  i2c_stop_cond();

  i2c_start_cond();
  i2c_send(11010001); //Sending address for Read
  *sec= i2c_receive(1); //Read seconds, send acknowledgement
  *minute= i2c_receive(1); // Read minutes, send acknowledgement
  *hour= i2c_receive(0); // Read hours, no acknowledgemnt sent
}
 /* Function Name: display_current_time
    Logic: To display the current time as received from the RTC on serial monitor through USART
    Example Call: display_current_time(); */
  void display_current_time()
  {  
    rtc_init(); 
    rtc_get_time(&set_hour,&set_min,&set_sec);
    usart_send_packedBCD(set_hour);
    usart_send(':');
    usart_send_packedBCD(set_min);
    usart_send(':');
    usart_send_packedBCD(set_sec);
    usart_send('  ');
    usart_send('\n');
    }
 /* Funtion Name: BCD_to_DEC
    Input: BCD value to be converted to decimal format
    Logic: Converts BCD to Decimal by dividing by 16 as shifting 4 places in Binary multiples or divides by 16
    Output: Decimal value of the input BCD
    example Call: BCD_to_DEC(set_hour);  */
 int BCD_to_DEC(unsigned char val) //Converting packed BCD to Decimal
 {
  return ((val/16)*10 + (val%16));
 }
 /* Funtion Name: DEC_to_BCD
    Input: Decimal value to be converted to BCD format
    Logic: Converts Decimal to BCD by dividing by 16 as shifting 4 places in Binary multiples or divides by 16
    Output: BCD value of the input Decimal vslue
    example Call: DEC_to_BCD(set_hour);  */
 unsigned char DEC_to_BCD(int val) //Converting Decimal to packed BCD
 {
  return ( (val/10)*16 + (val%10));
 }
 /*Function Name: setting_clock_time
   Logic: Sets the RTC time
   Example Call: settimg_clock_time(); */
 void setting_clock_time()
 {
  int parameter_select=0,tempH=0,tempM=0,tempS=0;
  int dec_hour, dec_min, dec_sec; // Varibles for storing decimal values of hour, min, sec
  unsigned char set_hour_message[]= {"Set Hours "};
  unsigned char set_min_message[]= {"Set Minutes "};
  unsigned char set_sec_message[]= {"Set Seconds "};
  while(parameter_select < 4)
 {  
  if( !(PIND & (1<<PD4)))
  {  
    parameter_select= parameter_select+1;
    delay_ms(200);
  }
  if(parameter_select==1)
  { 
    if(tempH==0){ 
      usart_send('\n');  
    for( int c=0; set_hour_message[c]!=0x00; c++){
       usart_send(set_hour_message[c]);}
       tempH++;}
    delay_ms(200);
    if( !(PIND & (1<<PD5)))
   { 
    dec_hour= BCD_to_DEC(set_hour);
    if(dec_hour==23){
       set_hour=0;
       rtc_set_time(set_hour,set_min,set_sec);
         }
    else{
       dec_hour++;
       set_hour= DEC_to_BCD(dec_hour);
       rtc_set_time(set_hour,set_min,set_sec);
      display_current_time(); 
     delay_ms(200);}
    }
   if( !(PIND & (1<<PD6)))
   { 
    dec_hour= BCD_to_DEC(set_hour);
    if(dec_hour==0){
       dec_hour=23;
       set_hour= DEC_to_BCD(dec_hour);
       rtc_set_time(set_hour,set_min,set_sec);
        display_current_time(); 
       delay_ms(200);}
     else{
       dec_hour--;
       set_hour= DEC_to_BCD(dec_hour);
       rtc_set_time(set_hour,set_min,set_sec);
       display_current_time(); 
      delay_ms(200);}
    }
      display_current_time(); 
      delay_ms(200);
   }
  if(parameter_select==2)
  {   
    if(tempM==0){
     usart_send('\n');
    for( int c=0; set_min_message[c]!=0x00; c++){
       usart_send(set_min_message[c]);}
       tempM++;}
    if( !(PIND & (1<<PD5)))
   { delay_ms(200);
    dec_min= BCD_to_DEC(set_min);
    if(dec_min==59){
       set_min=0;
       rtc_set_time(set_hour,set_min,set_sec);
         }
  else{
    dec_min++;
    set_min= DEC_to_BCD(dec_min);
    rtc_set_time(set_hour,set_min,set_sec);
        }
    }
   if( !(PIND & (1<<PD6)))
   {
    delay_ms(200);
    dec_min= BCD_to_DEC(set_min);
    if(dec_min==0){
      dec_min=59;
      set_min= DEC_to_BCD(dec_min);
      rtc_set_time(set_hour,set_min,set_sec);
           }
     else{
      dec_min--;
      set_min= DEC_to_BCD(dec_min);
      rtc_set_time(set_hour,set_min,set_sec);
       }
   }
    display_current_time(); 
    delay_ms(200);
  }
  if(parameter_select==3)
  {    
      if(tempS==0){
      usart_send('\n');
      for( int c=0; set_sec_message[c]!=0x00; c++){
       usart_send(set_sec_message[c]);}
       tempS++;}
    if( !(PIND & (1<<PD5)))
   {
    delay_ms(200);
    dec_sec= BCD_to_DEC(set_sec);
    if(dec_sec==59){
      set_sec=0;
     rtc_set_time(set_hour,set_min,set_sec);
     } 
  else{
    dec_sec++;
    set_sec=DEC_to_BCD(dec_sec);
    rtc_set_time(set_hour,set_min,set_sec);
     }
    }
   if( !(PIND & (1<<PD6)))
   { 
    delay_ms(200); 
    dec_sec= BCD_to_DEC(set_sec);
    if(dec_sec==0){
      dec_sec=59;
      set_sec= DEC_to_BCD(dec_sec);
      rtc_set_time(set_hour,set_min,set_sec);
      }
     else{
      dec_sec--;
      set_sec= DEC_to_BCD(dec_sec);
      rtc_set_time(set_hour,set_min,set_sec);
      }
   }
     display_current_time(); 
    delay_ms(200);
  }
 }
}
/*Function Name: set_alarm_or_timer_time
  Input: Addresses of the varibales string the set hour, min and sec
  Logic: Sets the Alarm time or Timer time
  Example Call: set_alarm_or_timer_time( &alarm_hour, &alarm_min, &alarm_sec); */
void set_alarm_or_timer_time( unsigned char *hour, unsigned char *minute, unsigned char *second)
{ 
  int parameter_select=0, tempH=0, tempM=0, tempS=0;
  int dec_hour, dec_min, dec_sec;// Varibles for storing decimal values of hour, min, sec
  unsigned char set_hour_message[]= {"Set Hours "};
  unsigned char set_min_message[]= {"Set Minutes "};
  unsigned char set_sec_message[]= {"Set Seconds "};
   usart_send_packedBCD(*hour);
   usart_send(':');
   usart_send_packedBCD(*minute);
   usart_send(':');
   usart_send_packedBCD(*second);
   usart_send(' ');
  while(parameter_select < 4)
 { delay_ms(200);
  if( !(PIND & (1<<PD4)))
  { 
    parameter_select= parameter_select+1;
  }
  if(parameter_select==1)
  { if(tempH==0){
      usart_send('\n');
     for( int c=0; set_hour_message[c]!=0x00; c++){
       usart_send(set_hour_message[c]);}
       tempH++;} 
    if( !(PIND & (1<<PD5)))
   { dec_hour= BCD_to_DEC(*hour);
    if(dec_hour==23){
        *hour=0;}
    else{
       dec_hour++;
        *hour= DEC_to_BCD(dec_hour);}
        usart_send_packedBCD(*hour);
        usart_send(':');
        usart_send_packedBCD(*minute);
        usart_send(':');
        usart_send_packedBCD(*second);
        usart_send(' ');
    }
   if( !(PIND & (1<<PD6)))
   { dec_hour= BCD_to_DEC(*hour);
    if(dec_hour==0){
       dec_hour=23;
       *hour= DEC_to_BCD(dec_hour);}
     else{
       dec_hour--;
        *hour= DEC_to_BCD(dec_hour);}
        usart_send_packedBCD(*hour);
        usart_send(':');
        usart_send_packedBCD(*minute);
        usart_send(':');
        usart_send_packedBCD(*second);
        usart_send(' ');
   } 
   delay_ms(200);
  }
  if(parameter_select==2)
  { if(tempM==0){
      usart_send('\n');
    for( int c=0; set_min_message[c]!=0x00; c++){
       usart_send(set_min_message[c]);}
       tempM++;}
    if( !(PIND & (1<<PD5)))
   { dec_min= BCD_to_DEC(*minute);
    if(dec_min==59){
       *minute=0;}
  else{
    dec_min++;
      *minute= DEC_to_BCD(dec_min);}
      usart_send_packedBCD(*hour);
        usart_send(':');
        usart_send_packedBCD(*minute);
        usart_send(':');
        usart_send_packedBCD(*second);
        usart_send(' ');
    }
   if( !(PIND & (1<<PD6)))
   {dec_min= BCD_to_DEC(*minute);
    if(dec_min==0){
      dec_min=59;
      *minute= DEC_to_BCD(dec_min);}
     else{
      dec_min--;
       *minute= DEC_to_BCD(dec_min);}
       usart_send_packedBCD(*hour);
        usart_send(':');
        usart_send_packedBCD(*minute);
        usart_send(':');
        usart_send_packedBCD(*second);
        usart_send(' ');
   } 
   delay_ms(200);
  }
  if(parameter_select==3)
  { if(tempS==0){
      usart_send('\n');
    for( int c=0; set_sec_message[c]!=0x00; c++){
       usart_send(set_sec_message[c]);}
       tempS++;}
    if( !(PIND & (1<<PD5)))
   {dec_sec= BCD_to_DEC(*second);
    if(dec_sec==59){
      *second=0;}
  else{
    dec_sec++;
     *second= DEC_to_BCD(dec_sec);}
     usart_send_packedBCD(*hour);
        usart_send(':');
        usart_send_packedBCD(*minute);
        usart_send(':');
        usart_send_packedBCD(*second);
        usart_send(' ');
    }
   if( !(PIND & (1<<PD6)))
   { dec_sec= BCD_to_DEC(*second);
    if(dec_sec==0){
      dec_sec=59;
      *second= DEC_to_BCD(dec_sec);}
     else{
      dec_sec--;
       *second= DEC_to_BCD(dec_sec);}
       usart_send_packedBCD(*hour);
        usart_send(':');
        usart_send_packedBCD(*minute);
        usart_send(':');
        usart_send_packedBCD(*second);
        usart_send(' ');
   } 
    delay_ms(200);
  }
 }
}
/* Function Name: timer2_init()
   Logic: Initializes AVR TIMER 2 in Non-inverting fast PWM mode for playing a tone
   Example Call: timer2_init(); */
void timer2_init()
{ //Initializing TIMER2 in fast PWM mode
  TCCR2A= (1<<COM2B1)|(1<<WGM21)|(1<<WGM20);//Non-inverting mode
  TCCR2B= (1<<CS22)|(1<<CS21); //256 bit prescaling
  TCNT2=0;
}
/*-------------------------------------------------------------------------------***--------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------***-----------------------------------------------------------------------------------------------*/
 int main(void)
{  
  int i,k,m;
  int dec_sec, dec_ref_sec;
  int dec_timer_hour, dec_timer_min, dec_timer_sec;
 unsigned char message_one []= {"Current Time:"};
 unsigned char message_two[]= {"Set Time:"};
 unsigned char message_three[]={"Set Alarm:"};
 unsigned char message_four[]= {"Set Timer:"};
 unsigned char message_five[]= {"Stopwatch Started:"};
 unsigned char message_six[]= {"Stopwatch Stopped."};
 unsigned char message_seven[]= {"Select mode:)"};
 unsigned char message_eight[]= {"Alarm Enabled"};
 unsigned char message_nine[]= {"Stopwatch Resetted"};
 unsigned char message_ten[]={"Stopwatch Mode."};
 unsigned char ref_hour, ref_min, ref_sec; 
   DDRD= (1<<PD3); //Setting OC2B pin as OUTPUT
   usart_init();
   for( int c=0; message_one[c]!=0x00; c++){
       usart_send(message_one[c]);}
       usart_send('\n');
  display_current_time();
     for( int c=0; message_seven[c]!=0x00; c++){
       usart_send(message_seven[c]);}
       usart_send('\n');
 while(1){
  i=k=m=0;
  delay_ms(200);
  if(!(PIND & (1<<PD6))){ //To set clock time
       for( int c=0; message_two[c]!=0x00; c++){
        usart_send(message_two[c]);}
        usart_send('\n');
       setting_clock_time();
       for( int c=0; message_seven[c]!=0x00; c++){
       usart_send(message_seven[c]);}
       usart_send('\n');} 
         
   if(!(PIND & (1<<PD7))){ // To set Alarm time
           for( int c=0; message_three[c]!=0x00; c++){
       usart_send(message_three[c]);}
       usart_send('\n');
      set_alarm_or_timer_time(&alarm_hour, &alarm_min, &alarm_sec);
       usart_send('\n');
          for( int c=0; message_seven[c]!=0x00; c++){
       usart_send(message_seven[c]);}
       usart_send('\n');} 
        
   if(!(PIND &(1<<PD5))){ //To keep displaying Current Time
        for( int c=0; message_one[c]!=0x00; c++){
       usart_send(message_one[c]);}
       usart_send('\n'); 
       
          if( (alarm_hour!=0) || (alarm_min !=0) || (alarm_sec !=0)){ // To prompt for Alarm being enabled
            for( int c=0; message_eight[c]!=0x00; c++){
              usart_send(message_eight[c]);}
                usart_send('\n'); }
        
         while(i<1){
            display_current_time();
              delay_ms(200);
           rtc_get_time(&set_hour, &set_min,&set_sec);
            if( (set_hour == alarm_hour) && (set_min == alarm_min) && ( set_sec== alarm_sec)) //To check for Alarm
          { for(int j=0; j<=50;j++){
             if(!(PIND & (1<<PD7))){
            TCCR2A=0x00;
            TCCR2B= 0x00;
            alarm_hour= alarm_min= alarm_sec= 0;
             break;}
             timer2_init(); // PLAY ALARM TONE
            OCR2B= 127;// 50% duty cycle
            delay_ms(200);
            TCCR2A=0x00;
            TCCR2B= 0x00;
            delay_ms(200);
         }
       }
      if( !(PIND & (1<<PD4))){
          delay_ms(200);
          i++;
       for( int c=0; message_seven[c]!=0x00; c++){
       usart_send(message_seven[c]);}
       usart_send('\n');
          }
        }
     }
     delay_ms(200);
    if( !(PIND & (1<<PD4))) //For TIMER mode
   { 
     for( int c=0; message_four[c]!=0x00; c++){
         usart_send(message_four[c]);}
         usart_send('\n');
    set_alarm_or_timer_time(&timer_hour, &timer_min, &timer_sec); //Set timer
    rtc_get_time(&ref_hour ,&ref_min , &ref_sec);   
   while( (timer_hour !=0) || (timer_min !=0) || (timer_sec!=0) )
   {
    rtc_get_time(&set_hour,&set_min, &set_sec);
     dec_sec= BCD_to_DEC(set_sec);
     dec_ref_sec=BCD_to_DEC(ref_sec);
   
    if( (dec_sec==(dec_ref_sec + 1 ))|| ( dec_sec==0 && dec_ref_sec==59)||( dec_sec==0 && dec_ref_sec==59)) // After passing of each SECOND
      {
      dec_timer_sec= BCD_to_DEC(timer_sec);
      dec_timer_sec--;
      timer_sec= DEC_to_BCD(dec_timer_sec);
      ref_sec= set_sec;
      }
    if(dec_timer_sec== -1)
     {
      dec_timer_sec=59;
      timer_sec= DEC_to_BCD(dec_timer_sec);
      dec_timer_min= BCD_to_DEC(timer_min);
      dec_timer_min--;
      timer_min= DEC_to_BCD(dec_timer_min);
      }
    if(dec_timer_min== -1)
     {
      dec_timer_min=59;
      timer_min= DEC_to_BCD(dec_timer_min);
      dec_timer_hour= BCD_to_DEC(timer_hour);
      dec_timer_hour--;
      timer_hour= DEC_to_BCD(dec_timer_hour);
       }
      usart_send_packedBCD(timer_hour);
      usart_send(':');
      usart_send_packedBCD(timer_min);
      usart_send(':');
      usart_send_packedBCD(timer_sec);
      usart_send('\n');
      delay_ms(200);
     } 
 
      while( (timer_hour==0) && (timer_min==0) && (timer_sec==0)) {
        if(!(PIND & (1<<PD7))){
        TCCR2A=0x00;
        TCCR2B= 0x00; //Stop Sound
       for( int c=0; message_seven[c]!=0x00; c++){
       usart_send(message_seven[c]);}
       usart_send('\n');
          break;
         }
        timer2_init(); // Play Sound
        OCR2B= 200;
        delay_ms(200);
        TCCR2A=0x00;
        TCCR2B= 0x00;
        delay_ms(100); 
       }   
      }
   // STOPWATCH setting
   if( !(PIND & (1<<PD2)))
  { delay_ms(100);   
    for( int c=0; message_ten[c]!=0x00; c++){
       usart_send(message_ten[c]);}
       usart_send('\n'); 
    while(m<1){
     k=0;
     delay_ms(200);
    if( !(PIND & (1<<PD2))){
     for( int c=0; message_five[c]!=0x00; c++){
       usart_send(message_five[c]);}
       usart_send('\n');
     timer2_init(); // PLAY TONE
     OCR2B= 200;
     delay_ms(200);
     TCCR2A=0x00;
     TCCR2B= 0x00;
   rtc_get_time(&ref_hour ,&ref_min , &ref_sec);   
   while(k<1)
   { 
    if( !(PIND & (1<<PD2))){ 
     timer2_init(); // PLAY TONE
     OCR2B= 200;
     delay_ms(200);
     TCCR2A=0x00;
     TCCR2B= 0x00; } 
    rtc_get_time(&set_hour,&set_min, &set_sec);
     dec_sec= BCD_to_DEC(set_sec);
     dec_ref_sec=BCD_to_DEC(ref_sec);
   
    if( (dec_sec==(dec_ref_sec + 1 ))|| ( dec_sec==0 && dec_ref_sec==59)||( dec_sec==0 && dec_ref_sec==59))
      {
      dec_timer_sec= BCD_to_DEC(timer_sec);
      dec_timer_sec++;
      timer_sec= DEC_to_BCD(dec_timer_sec);
      ref_sec= set_sec;
      }
    if(dec_timer_sec== 60)
     {
      dec_timer_sec=0;
      timer_sec= DEC_to_BCD(dec_timer_sec);
      dec_timer_min= BCD_to_DEC(timer_min);
      dec_timer_min++;
      timer_min= DEC_to_BCD(dec_timer_min);
      }
    if(dec_timer_min== 60)
     {
      dec_timer_min=0;
      timer_min= DEC_to_BCD(dec_timer_min);
      dec_timer_hour= BCD_to_DEC(timer_hour);
      dec_timer_hour++;
      timer_hour= DEC_to_BCD(dec_timer_hour);
       }
      usart_send_packedBCD(timer_hour);
      usart_send(':');
      usart_send_packedBCD(timer_min);
      usart_send(':');
      usart_send_packedBCD(timer_sec);
      usart_send('\n');
      delay_ms(200);
       
      if(!(PIND & (1<<PD2)))//Stop the Stopwatch
      {  
       timer2_init(); // PLAY TONE
       OCR2B= 200;
       delay_ms(200);
       TCCR2A=0x00;
       TCCR2B= 0x00;
       for( int c=0; message_six[c]!=0x00; c++){
        usart_send(message_six[c]);}
        usart_send('\n');
       k++;}
          }
        }
       if( !( PIND & (1<<PD6))) //Reset Stopwatch
       {
       timer_hour=0;
       timer_min=0;
       timer_sec=0;
      usart_send_packedBCD(timer_hour);
      usart_send(':');
      usart_send_packedBCD(timer_min);
      usart_send(':');
      usart_send_packedBCD(timer_sec);
      usart_send('\n');
        for( int c=0; message_nine[c]!=0x00; c++){
       usart_send(message_nine[c]);}
       usart_send('\n');
        for( int c=0; message_seven[c]!=0x00; c++){
       usart_send(message_seven[c]);}
       usart_send('\n');
      m++; }
          }
        }
      }   
 return 0;
 }   
