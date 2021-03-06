#include <Arduino.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <ServoTimer2.h>
//this code takes input from a myoelctric and toggles the hand between the contracted
// and extended positions when there is a spike above a user-defined level
// future projects will include running this code with low power

//#include <Servo.h>     //library used for Servos

ServoTimer2 thumb;            //defines the name of the five Servos and them as Servo objects
ServoTimer2 pointer;
ServoTimer2 middle;
ServoTimer2 ring;
ServoTimer2 pinky;

#define extendmin 2000    //static integers of extendminimum and extendmaximum Servo extension
#define extendmax 1000

int trigger = 600; //default value incase if all fails
int rotation = 2; //set to 4 for more opitons
int isLocked = 0; //0 is not locked

enum pins
{
    THUMB_PIN = 3,
    INDEX_PIN = 5,
    MIDDLE_PIN = 6,
    RING_PIN = 9,
    PINKY_PIN = 10,
    VBAT_PIN = A0,
    MYO_PIN = A5,
    LED_PIN = 11,
    BUTTON_PIN = 7
};

enum hand_state
{
    OPEN, FIST, POINT, PINCH
};

#define overload 1028

int state = OPEN;    // sets default state to be open
#define threshold .4    // this is the percentage of the extendmax read we use to define a flex

int setTrigger();
int check(int);
void collect(int *, int *, int *);
void handPosition(int, int, int, int, int);
void locked();

// THIS PART ONLY RUNS ONCE
void setup() {
    Serial.begin(9600);     // starts up serial communication between arduino and computer

    Serial.println("linking Servos");
	thumb.attach(THUMB_PIN);
    pointer.attach(INDEX_PIN);
    middle.attach(MIDDLE_PIN);
    ring.attach(RING_PIN);
    pinky.attach(PINKY_PIN);

    // thumb.attach(THUMB_PIN, extendmin, extendmax);    //defines where the Servos are and their extendmax/extendmins
    // pointer.attach(INDEX_PIN, extendmin, extendmax);
    // middle.attach(MIDDLE_PIN, extendmin, extendmax);
    // ring.attach(RING_PIN, extendmin, extendmax);
    // pinky.attach(PINKY_PIN, extendmin, extendmax);

    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), locked, RISING);

    Serial.println("Starting");       //makes the hand relax all of the way
    handPosition(extendmax, extendmax, extendmax, extendmax, extendmax);

    Serial.println("Reading in 3");
    delay(1000);
    Serial.println("Reading in 2");
    delay(1000);
    Serial.println("Reading in 1");
    delay(1000);

    trigger = setTrigger();
}

//setup starts by linking the Servo objects to each pin and detailing the limits per each argument. Then it opens the hand for a default position and will call the setTrigger function. Once it's collected, the setup is complete

void loop() {
    Serial.println(analogRead(MYO_PIN));

    float currentVoltage = 0;

    if (check(currentVoltage = analogRead(MYO_PIN)) && isLocked == 0) { // collects voltage, then assigns to currentVoltage, then passes currentVoltage to check function, then checks the check return value to see if it's higher than trigger value

        state = (++state) % (FIST+1); // increment through available state values

        Serial.print("state is ");
        Serial.println(state);

        //Serial.println(currentVoltage); // prints in the serial moniter

        switch (state) {
        case OPEN:
            Serial.println("Open");
            handPosition(extendmax, extendmax, extendmax, extendmax, extendmax);
            break;
        case FIST:
            Serial.println("Fist");
            handPosition(extendmin, extendmin, extendmin, extendmin, extendmin);
            break;
        case POINT:
            Serial.println("Point");
            handPosition(extendmax, extendmin, extendmax, extendmax, extendmax);
            break;
        case PINCH:
            Serial.println("Pinch");
            handPosition(extendmin, extendmin, extendmin, extendmax, extendmax);
            break;
        default:
            state = FIST; //edgecase if logic fail
            handPosition(extendmax, extendmax, extendmax, extendmax, extendmax);
            break;
        }
    }
}

//the loop function will continously check the users input and assign it to currentVoltage, then it's verified through the check function to see if its higher than the set trigger value from setup, if so then it will switch between the four hand positions in a looping structure

/*int setTrigger(){
  float average = 0;
  int i, j;
  do{
    average = 0;//resets collection value if it failed prior
    for(j = 1; j <= 3; j++){//loops to collect the three impulses
      Serial.print("Reading impulse ");
      Serial.print(j);
      Serial.println(" in the next 3 seconds");
      for(i = 3; i != 0; i--){//counts down 3 seconds
        Serial.println(i);
        delay(1000);
      }
      Serial.println("Reading");
      average = average + analogRead(myoIn);//adds up collected values with previous values
      Serial.print("Total is:");
      Serial.println(average);
    }
      average = average/3;
      Serial.print("Average is:");
      Serial.println(average);
  } while((average) < 200);//if it's less than 200 than we reattempt collection
  return (int)(average);//returns average upon success
  }*/


int setTrigger() {
    float average = 0;
    int i, j;
    int max1, max2, max3;
    int loopcount, starttime, endtime;
    do {
        average = 0;//resets collection value if it failed prior
        max1 = 0;
        max2 = 0;
        max3 = 0;
        loopcount = 0;
        starttime = millis();
        endtime = starttime;
        while ((endtime - starttime) < 3000) // do this loop for about 1 second
        {
            collect(&max1, &max2, &max3);
            loopcount = loopcount + 1;
            endtime = millis();
        }
        average = (max1 + max2 + max3) / 3;
        Serial.println("max1");
        Serial.print(max1);
        Serial.println("max2");
        Serial.print(max2);
        Serial.println("max3");
        Serial.print(max3);
        Serial.println("average");
        Serial.println(average);
        Serial.println("loop");
        Serial.println(loopcount);
    } while ((average) < 200); //if it's less than 200 than we reattempt collection
    return (int)(average);//returns average upon success
}

void collect(int * max1, int * max2, int * max3) {
    float prevVolt = analogRead(MYO_PIN);
    float currVolt = analogRead(MYO_PIN);
    float nextVolt = analogRead(MYO_PIN);
    if (currVolt < overload && prevVolt < currVolt && currVolt > nextVolt) {
        if (currVolt > *max1) {
            *max1 = currVolt;
            return;
        }
        else if (currVolt > *max2) {
            *max2 = currVolt;
            return;
        }
        else if (currVolt > *max3) {
            *max3 = currVolt;
            return;
        }
    }
}

//setTrigger collects 3 inputs by the user with a 3 second delay between each one. It calculates the average and if it's lower than 200, it's an assumed error and will reset the collection. Once completed it returns the average

int check(int voltage) {
    if (voltage > trigger && voltage < overload) {
        return 1;
    }
    return 0;
}

//check collects the voltage, and if it's greater than the trigger value, will return 1 for true, else 0 for false

void locked() {
    isLocked++;
    isLocked = isLocked % 2;
}

void handPosition(int thumbPos, int pointerPos, int middlePos, int ringPos, int pinkyPos) {
    //function hand position takes in 5 integer inputs for each finger and moves the Servos according to
    //each input. for the Servos, extendmaximum extension is 1000 and extendminimum (all the way in) is 2000.
    //this is a function from the Servo.h library basically gives the pulse width
    pointer.write(pointerPos);
    middle.write(middlePos);
    ring.write(ringPos);
    pinky.write(pinkyPos);
    delay(200);
    thumb.write(thumbPos);
    delay(1000);       //this is so the Servos have time to get to their set position
    thumb.write(thumb.read()); //adjusts Servos to adjust desired position to actual position reached to avoid burnout
    pointer.write(pointer.read());
    middle.write(middle.read());
    ring.write(ring.read());
    pinky.write(pinky.read());
    delay(250);
}





// const int led_pin = 12;

// const uint16_t t1_load = 0;
// const uint16_t t1_comp = 31250;

// void setup() {
// 	// st LED to be output
// 	DDRB |= (1 << led_pin);

// 	// Reset Timer1 Control Reg A
// 	TCCR1A = 0;

// 	// SEt to prescaler of 256
// 	TCCR1B |= (1<< CS12);
//	TCCR1B &= ~(1 <<CS11);
// 	TCCR1B &= ~(1 <<CS10);

// 	// Reset Timer1 and set compare values
// 	TCNT1 = t1_load;
// 	OCR1A = t1_comp;

// 	// Enable Timer1 compare interrupt
// 	TIMSK1 = (1 << OCIE1A);

// 	// enable global interrupts
// 	sei();
// }

// void loop() {
// 	delay(500);
// }

// ISR(TIMER1_COMPA_vect){
// 	// rest timer 1 to zero
// 	TCNT1 = t1_load;
// 	// do whatever here...
// }