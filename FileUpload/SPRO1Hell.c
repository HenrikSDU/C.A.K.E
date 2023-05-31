#define F_CPU 16000000UL //needs to be defined for the delay functions to work.
#define BAUD 9600
#define NUMBER_STRING 1001
#define FIFTEENTHCIRCUMFERENCE 0.01382300768
#define EIGTHCIRCUMFERENCE 0.02589182
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h> //here the delay functions are found
#include "usart.h"

#include <stdbool.h>

// Variables for the interrupts
volatile unsigned long int timer = 0, counter = 0;                        // Timer: variable for the time; counter: counter to count timeroverflows
volatile bool car_move_flag = false, stringbeginflag=false;               // Variable to indicate whether the car is moving
volatile int i, distancecounter = 0, readBufferindex = 0, buffersize, stages_driven = 0;     // For for loop in interrupt
volatile char readBuffer[100]= {0}, rxexpect=0x71;
volatile double seconds, secondstogo, secondsgone = 0, speed = 0, neededspeed=0, prev_speed = 0, eigthcircumference = 0.02589182, distance = 0, distancetogo;

//sonic distance
volatile unsigned char sonicoverflowcount = 0;
volatile float sonicseconds = 0, sonic_distance = 0;
volatile unsigned long int sonictime = 0;
volatile bool active_pulse = false, began_measurement = false, timerecieve = false;
// Variables for the functions

char acceleration_flag = 0;                 // Variable for indicating the state of aceleration
char savereadBuffer[100]= {0};
int currentpagenumber = 0, stagesexpexted = 0, stagenumber = 0, ocr0asetter=100;
uint32_t setspeed=0;


unsigned int adcval;
int digitalVolt = 0;
float Volt, totalvolt;

typedef struct{
    long double stagetime;
    double stagedistance;
    double stagespeed;
    bool direction_flag;

}rallystage_t;

volatile rallystage_t rallystages[10];
char acceleration_index(double, double);    // Function for checking the state of acceleration
/* Declare functions */
void initialize(void);      //Function for initializing the timer and interrupts
char displaysave(void);
void getpage(void);
void updatedata(void);
void PWM_Motor(unsigned char);
unsigned int read_adc(void);
void cardriver(int);
float voltagecalc(void);
void batteryalert(void);
void sonicdistance(void);

//interrupts

ISR(INT0_vect){
    
    if(began_measurement==true){
        TCCR2B &= ~(/*(1<<CS22)|*/(1<CS21)|(1<<CS20)); //reset timer2 
        sonictime = TCNT2+sonicoverflowcount*255;
        sonicseconds = (float)sonictime*2.0f;
    
        sonic_distance = sonicseconds*0.034;
        //sonic_distance = sonicseconds/58;

        began_measurement=false;
        active_pulse=false;
        TCNT2=0;
        sonicoverflowcount=0;
    }
    if(began_measurement==false&&active_pulse==true){
        TCCR2B |= (/*(1<<CS22)|*/(1<CS21)|(1<<CS20)); //start timer2 with 32
        began_measurement = true;
    }

}

ISR(TIMER2_OVF_vect){
    sonicoverflowcount++;  
    /*if(sonicoverflowcount>117){
        sonicoverflowcount = 0;
        active_pulse = false;
    }*/

}

ISR(USART_RX_vect){
    scanf("%c", &readBuffer[readBufferindex]);

    if(stringbeginflag==false){                    //making sure that only first indicator gets detected
        if(readBuffer[readBufferindex]==rxexpect){//saving first element of string at index 0
            readBufferindex=0;
            readBuffer[0]=rxexpect;
            stringbeginflag=true;
        }
    }

    readBufferindex++;
    switch(rxexpect){
        case 0x71:
        buffersize=8;
        break;
        case 0x65:
        buffersize=7;
        break;
        case 0x66:
        buffersize=5;
    }

    if(readBufferindex == buffersize){//maybe adding expected_byte_count - buffersize
    readBufferindex = 0;
    stringbeginflag = false;
    /*UCSR0B &= ~(1<<RXEN0);
    UCSR0B |= 1<<RXEN0;*/
    }
}

ISR(TIMER1_CAPT_vect){
    timer=ICR1+65535*counter;// Updating timer value
    //printf("Input Capture EVENT!!!"); line used for debugging
    TCNT1=0;                // Reseting the timer to zero
    TIFR1|=1<<ICF1;         // Reseting the input capture flag
    counter=0;              // Reseting overflow counter
    car_move_flag = true;   // Car is being moved
    distancecounter++;
    seconds = ((double)timer*1000)/15625000;    // Time calculation (Seconds)
    secondsgone = seconds + secondsgone;
    //secondstogo = secondstogo - seconds;
    speed = FIFTEENTHCIRCUMFERENCE/seconds;
    distance = (double)distancecounter*FIFTEENTHCIRCUMFERENCE;  // Distance calculation
    distancetogo = (double)rallystages[stages_driven].stagedistance - distance;
    if(!distancecounter)
    secondstogo = rallystages[stages_driven].stagetime;
}

ISR(TIMER1_OVF_vect){
    counter++;            // Adding one to the overflow counter
    //TCNT1=0;
    if(counter>=2){        // The car has not really moved for a long time
        car_move_flag = 0;// So the move-flag is reset
        speed = 0;
    }
}

//main function
int main(void) {    
    uart_init();   // Open the communication to the microcontroller
	io_redirect(); // Redirect input and output to the communication
    initialize();
   
    

    while(1){
    // Reseting all the values
    speed=0;
    prev_speed=0;
    stagenumber=0;
    acceleration_flag=0;
    car_move_flag = false;
    currentpagenumber=0;
    stages_driven = 0;
    ocr0asetter = 0;
    secondstogo = 0;
    seconds = 0;
    counter = 0;

    //printf("%lf", speed); 
        rxexpect=0x65;
        while(!(readBuffer[0]==0x65 && readBuffer[1]==0x00 && readBuffer[2]==0x01)){
            _delay_ms(1000);
            batteryalert();
        }

       
        

        while(!(readBuffer[0]==0x65 && readBuffer[1]==0x04 && readBuffer[2]==0x06)){
        _delay_ms(1000);
        batteryalert();
        }
        _delay_ms(250);
        rxexpect=0x71;
        printf("get %s.val%c%c%c","etap.n0",255,255,255);	//sends "get secpag.n0.val"
            _delay_ms(500);
            
            if(readBuffer[0] == 0x71 && readBuffer[5] == 0xFF && readBuffer[6] == 0xFF && readBuffer[7] == 0xFF){
                
                stagesexpexted = readBuffer[1] | (readBuffer[2] << 8) | (readBuffer[3] << 16)| (readBuffer[4] << 24);                 
            }
        
        //Page rallystages
        
        printf("page 3%c%c%c",255,255,255);
        printf("Rallystages.n2.val=%d%c%c%c", stagesexpexted, 255, 255, 255);
        stagenumber=0;
        
        do{
        printf("Rallystages.n0.val=%d%c%c%c", stagenumber+1, 255, 255, 255);

        //wait for button
        if(voltagecalc()<=6.6)
        batteryalert();
        rxexpect=0x65;
        while(!(readBuffer[0]==0x65 && readBuffer[1]==0x03 && readBuffer[2]==0x04));
        //reading the confirmed values
        if(voltagecalc()<=6.6)
        batteryalert();
        _delay_ms(51);
        rxexpect=0x71;
        printf("get %s.val%c%c%c","Rallystages.x0",255,255,255);	//sends "get secpag.n0.val"
            _delay_ms(51);
            
            if(readBuffer[0] == 0x71 && readBuffer[5] == 0xFF && readBuffer[6] == 0xFF && readBuffer[7] == 0xFF){
                
                rallystages[stagenumber].stagedistance = (readBuffer[1] | (readBuffer[2] << 8) | (readBuffer[3] << 16)| (readBuffer[4] << 24));
                rallystages[stagenumber].stagedistance /= 10;
                rallystages[stagenumber].direction_flag = true;
                if((readBuffer[4]&0x01)==0x01){
                    rallystages[stagenumber].stagedistance /= -1.0f;
                    rallystages[stagenumber].direction_flag = false;
                }
                //printf("v:%f",rallystages[stagenumber].distance);
                //printf("Rallystages.x0.val=%d%c%c%c",/*(int)rallystages[stagenumber].distance*10*/0 , 255,255,255);
            }

        printf("get %s.val%c%c%c","Rallystages.n1",255,255,255);	//sends "get secpag.n0.val"
            _delay_ms(51);

            if(readBuffer[0] == 0x71 && readBuffer[5] == 0xFF && readBuffer[6] == 0xFF && readBuffer[7] == 0xFF){
                //printf("debug2");
                rallystages[stagenumber].stagetime = (long double)(readBuffer[1] | (readBuffer[2] << 8) | (readBuffer[3] << 16)| (readBuffer[4] << 24));
                //printf("v:%f",rallystages[stagenumber].stagetime);             
            }
        
        //calculating the expected average speed
        //rallystages[stagenumber].stagespeed = rallystages[stagenumber].stagedistance/rallystages[stagenumber].stagetime;
        
        stagenumber++;
        }while(stagesexpexted>stagenumber);//check condition
        //printf("%f",rallystages[0].stagespeed);
        stagenumber=0;

        //starting the car
        //go to next page
        printf("page 5%c%c%c",255,255,255);

        if(voltagecalc()<=6.6)
        batteryalert();

        rxexpect=0x65;
        while(!(readBuffer[0]==0x65 && readBuffer[1]==0x05 && readBuffer[2]==0x01 && readBuffer[3]==0x01));//stops the car form doing anything until start button is pressed
       // _delay_ms(51);
        
        if(voltagecalc()<=6.6)
        batteryalert();
        cardriver(stagesexpexted);
        printf("progress.x0.val=%lu%c%c%c", (unsigned long int)(speed*1000), 255,255,255);
        printf("progress.x1.val=%lu%c%c%c", (unsigned long int)(distance*1000), 255,255,255);
        printf("progress.x2.val=%lu%c%c%c", (unsigned long int)(distancetogo*1000), 255,255,255);
        printf("progress.n1.val=%u%c%c%c",(unsigned int)(voltagecalc()*10),255,255,255);
        printf("progress.x3.val=%lu%c%c%c", (unsigned long int)(secondstogo*1000), 255,255,255);
        printf("progress.j0.val=%lu%c%c%c", (unsigned long int)((distance/rallystages[stages_driven].stagedistance)*100), 255,255,255);
        
        rxexpect = 0x65;
        while(!(readBuffer[0]==0x65 && readBuffer[1]==0x06 && readBuffer[2]==0x10));

        printf("page 0%c%c%c",255,255,255);
        }
            return 0;
        }

/* Function descriptions */

//function for initializing interrups and the timer for the optocoupler
inline void initialize(void){
    sei();  // Enable global interrupts
    
    // Usart interrupts
    SREG |= 1<<SREG_I;
    UCSR0B |=1<<RXCIE0; // Enabeling interrupt for rx complete
    
    //TIMSK1 |= (1<<ICIE1)|(1<<TOIE1);    // Timer interrupts must be enabled
    TCCR1A = 0x00;
    TCCR1B = (1<<ICNC1)/*(1<<ICES1)|*/;//noise cancel-/*falling*/ raising edge - 1024 prescaling
    DDRB &= ~0x01;//opto
    PORTB |= 0x01;
    DDRD = (0b00111000|0x60);
    PORTD |=0b11000111;//enable pullups and 
    TIFR1 |= 1<<ICF1;                   // Reseting input capture flag

    ADMUX = ADMUX | 0x40;//ADC0 single ended input on PortC0
    ADCSRB = ADCSRB & (0xF8);//Free running mode
    ADCSRA = ADCSRA | 0xE7; //Enable, Start conversion, slow input clock

    //Distancecalc for not crashing - virtual airbag
    TCCR2A = 0x00; //normal timer operation
    TIMSK2 |= (1<<TOIE2); //overflow interrupt
    EICRA |= (/*(1<<ISC01)|*/(1<<ISC00)); //any logical change causes interrupt

    //LEDs
    DDRC |= ((1<<PINC1)|(1<<PINC2));
   // DDRC = 0xFF;
    //PORTC = 0;
    
}   

//potential function to save input
inline char displaysave(void){
    for(i=0;i<100;i++){
        savereadBuffer[i] = readBuffer[i];
    }
}

//checking acceleration with optocoupler data
inline char acceleration_index(double current_speed, double previous_speed){
    char acceleration_flag;
    if(current_speed == 0 && previous_speed == 0){
        acceleration_flag=0;
    }else if(current_speed < prev_speed){
        acceleration_flag=1;
    }
        
    if(current_speed > prev_speed){
        acceleration_flag = 2;
    }
    if(speed==0){
        acceleration_flag = 0;
        //printf("debug");
    }
    return acceleration_flag;
}

 
inline void PWM_Motor(unsigned char duty){  
    DDRD |= 0x60;    // Set Port D as output for the ENA (Motor) 0b0010 0000

    TCCR0A |= 0X83;  // Fast PWM
    TCCR0B |= 0X05;    // 1024 Prescaler

    OCR0A = duty;
}

inline void updatedata(void){
    printf("progress.x0.val=%ld%c%c%c", (long int)(speed*1000), 255,255,255);
    printf("progress.x1.val=%ld%c%c%c", (long int)(distance*1000), 255,255,255);
    printf("progress.x2.val=%ld%c%c%c", (long int)(distancetogo*1000), 255,255,255);
    printf("progress.n1.val=%ld%c%c%c", (long int)(voltagecalc()*10),255,255,255);
    printf("progress.x3.val=%ld%c%c%c", (long int)(secondstogo*1000), 255,255,255);
    printf("progress.j0.val=%lu%c%c%c", (unsigned long int)((distance/rallystages[stages_driven].stagedistance)*100), 255,255,255);
    printf("progress.n2.val=%u%c%c%c", (unsigned int)(sonic_distance/100),255,255,255);
}

inline void getpage(void){
    rxexpect=0x66;
    printf("sendme%c%c%c",255,255,255);
    _delay_ms(51);
    currentpagenumber=readBuffer[1];
}

void cardriver(int stagecount){
    
    PORTD |=0b00010000;
    PORTD &= ~(0b00100000);
    
    secondstogo = 0;
        seconds = 0;
    printf("progress.n0.val=%d%c%c%c",stages_driven+1,255,255,255);
    bool stagecompleteflag = false;
    //PORTC = 0b00000100;
    PORTC &= ~(1<<PINC1);
    PORTC |= (1<<PINC2);
    if(!rallystages[stages_driven].direction_flag){
        PORTD &= ~(0b00010000);
        PORTD |=0b00100000;
        PORTC &= ~(1<<PINC2);
        PORTC |= (1<<PINC1);
    }

    distancetogo = rallystages[stages_driven].stagedistance;    
    printf("progress.x2.val=%ld%c%c%c", (long int)(distancetogo*1000), 255,255,255);
    
    secondstogo = rallystages[stages_driven].stagetime;    
    neededspeed = distancetogo/secondstogo;     
    printf("progress.x3.val=%ld%c%c%c", (unsigned long int)(secondstogo*1000), 255,255,255);
    counter = 0; //reseting overflow counter
    seconds=0;
    speed=0;
    distance=0;
    secondsgone = 0;
    if(rallystages[stages_driven].stagedistance>=2)
    ocr0asetter = 60;
    if(rallystages[stages_driven].stagedistance<=2)
    ocr0asetter = 50;
    TIFR1 |= (1<<ICF1); //reseting input capture flag
    TCNT1 = 0;
    ICR1 = 0;
    //maybe move secondstogo calc into function
    PWM_Motor(ocr0asetter);
    TCCR1B|= (1<<CS12)|(1<<CS10);
    TIMSK1 |= (1<<ICIE1)|(1<<TOIE1);    // Timer interrupts must be enabled
    EIMSK |= (1<<INT0); //external interrupt 0 enable
    while(!stagecompleteflag){
        sonicdistance();
        if(voltagecalc()<=6.6)
        batteryalert();
        
        
        secondstogo = rallystages[stages_driven].stagetime - secondsgone;
        updatedata();
        neededspeed = distancetogo/secondstogo;
        if(secondstogo<=0)
        ocr0asetter = 255;
        //PWM_Motor(255);
        if(speed == 0)
        PWM_Motor(150);
        if (speed<neededspeed && ocr0asetter<253){
           
            ocr0asetter+=1;
        }
        if (speed>neededspeed && ocr0asetter>46){
            ocr0asetter-=1;
        }

    PWM_Motor(ocr0asetter);
    
    acceleration_flag = acceleration_index(speed, prev_speed);
    prev_speed = speed;
    switch(acceleration_flag){

        case 1: 
        printf("progress.t4.txt=%s%c%c%c","\"breaking\"",255,255,255);
        break;
        case 2:
        printf("progress.t4.txt=%s%c%c%c","\"accelerating\"",255,255,255);
    }
    
        if(distance >= rallystages[stages_driven].stagedistance){
        stagecompleteflag = true;
        distancecounter = 0;
        distance=0;
        secondsgone = 0;
        stages_driven++;
        }
    }
    
    //rallystages[stages_driven].stagespeed
    /*
    Get distance and time
    Run motor (set speed using time given) 
    Stop motor when distance has been reached*/
    
    TCCR1B&= ~((1<<CS12)|(1<<CS10));
    if(stages_driven < stagecount){
        
        cardriver(stagecount);
    }
    secondstogo = 0;
    seconds = 0;
    PWM_Motor(0);
    PORTC = 0;
    
    TIMSK1 &= ~((1<<ICIE1)|(1<<TOIE1));
    TCNT1=0;
    EIMSK &= ~(1<<INT0);
    

    
}

inline unsigned int read_adc(void){

    unsigned int adclow = ADCL;
    return (adclow + ((ADCH & 0x03) << 8));//need to ensure that ADCL is //read first as it is not updated otherwise
}

float voltagecalc(void){ // 90

   digitalVolt = read_adc();
   Volt = (float)digitalVolt/1023.0f*(4.96-1.0f);
   totalvolt = Volt/15.0f*45.0f;

    return totalvolt;
}

void batteryalert(void){
   if(((int)(voltagecalc()*10)<=66)){
    PWM_Motor(0);
    printf("page 7%c%c%c",255,255,255);
    while(1){
    printf("battery.x0.val=%d%c%c%c", (int)(voltagecalc()*10),255,255,255);
    _delay_ms(500);
    }
    }
}

inline void sonicdistance(void){
    //before make pulse
    if(active_pulse == false){

    PORTD &= ~(0b00001000);
    _delay_us(2);

    PORTD |= 0b00001000;//maybe to short
    _delay_us(10);
    PORTD &= ~(0b00001000);

    active_pulse=true;

    }
    
}