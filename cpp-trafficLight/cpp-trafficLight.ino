#define RED 0
#define YELLOW_BEFORE_GREEN 1
#define GREEN 2
#define YELLOW_BEFORE_RED 3

unsigned long time_end;
//----timer stuff----
#include <avr/io.h>
#include <avr/interrupt.h>
volatile unsigned long passes=100000;
volatile unsigned long count=0;
ISR(TIMER1_COMPA_vect){
  count++;
}

void initTimer(){
  //cli();
TCCR1A=0;
TCCR1B=0;
OCR1A = 65535;
TCCR1B |= (1<<WGM12)|(1<<CS10);
TIMSK1 |= (1<<OCIE1A);
sei();
Serial.println("timer1 Ready!");
}

void reset_timer(){
  TCNT1 = 0;
  count = 0;
  //sei();
}

unsigned long cycles(){
  long temp = TCNT1;
  return (count*65535) + temp;
}
long i = 0;
//----timer stuf end--

// this is our class for a traffic light lane.
// Everything inside it will be uniqie per instantiated object.
class Traff {

  int state;
  int gre;
  int yel;
  int red;

public:
  Traff(int Gre, int Yel, int Red) {
    // here we assign the received variables to the values of the global variables.
    this->gre = Gre;
    this->yel = Yel;
    this->red = Red;

    pinMode(gre, OUTPUT);  // Green led
    pinMode(yel, OUTPUT);  // yellow led
    pinMode(red, OUTPUT);  // red led
  };

public:
  void setState(int &state)  // we use this method to change states of each traffic lane.
  {
    this->state = state;
  }

public:
  void trafficLight() {
    int &red = this->red;
    int &yel = this->yel;
    int &gre = this->gre;


    switch (this->state) {
      case RED:
        //Serial.println("red");  // red light on.
        digitalWrite(red, HIGH);
        digitalWrite(yel, LOW);
        break;

      case YELLOW_BEFORE_GREEN:  // yellow light on.
        //Serial.println("Yellow1");
        digitalWrite(yel, HIGH);
        break;

      case GREEN:  // green light on.
        //Serial.println("Green");
        digitalWrite(red, LOW);
        digitalWrite(yel, LOW);
        digitalWrite(gre, HIGH);
        break;

      case YELLOW_BEFORE_RED:  // yellow light on.
        //Serial.println("Yellow2");
        digitalWrite(yel, HIGH);
        digitalWrite(gre, LOW);
        break;
    }
  }
};  // class ends here. back to "normal" programming.

Traff traffic1(10, 9, 8);  // trafficlight object instantiated
Traff traffic2(6, 5, 4);   // one more.
int light_state;
int prev_state;
const int green_red_time = 5;
const int yellow_time = 1;
unsigned long traffic_timer;
int msRed = 1000;
int msGreen = 1000;
int button1 = 2;
int button2 = 3;

void setup()  // open serial for feedback, set each light to yellow.
{
  Serial.begin(9600);
  traffic_timer = millis();  // Initiate traffic_timer
  progression(YELLOW_BEFORE_GREEN, YELLOW_BEFORE_GREEN, YELLOW_BEFORE_RED);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  initTimer();
}

void loop() {
  bool but1 = digitalRead(button1);
  bool but2 = digitalRead(button2);

  switch (light_state)  // Change states and handle output
  {
    case RED:
      msGreen = msChange(msGreen, but1);
      if (millis() - traffic_timer > green_red_time * msRed) {
        reset_timer();
        msRed = 1000;
        progression(YELLOW_BEFORE_GREEN, YELLOW_BEFORE_GREEN, YELLOW_BEFORE_RED);
        time_end = cycles();
        String exec ="execution time: " + String(time_end);
        Serial.println(light_state);
        Serial.println(exec);
      }
      break;
    case YELLOW_BEFORE_GREEN:
      msRed = msChange(msRed, but2);
      msGreen = msChange(msGreen, but1);
      if (millis() - traffic_timer > yellow_time * 1000) {
        reset_timer();
        progression(GREEN, GREEN, RED);
        time_end = cycles();
        String exec = "execution time: " + String(time_end);
        Serial.println(light_state);
        Serial.println(exec);
      }
      break;
    case GREEN:
      msRed = msChange(msRed, but2);
      if (millis() - traffic_timer > green_red_time * msGreen) {
        reset_timer();
        msGreen = 1000;
        progression(YELLOW_BEFORE_RED, YELLOW_BEFORE_RED, YELLOW_BEFORE_GREEN);
        time_end = cycles();
        String exec = "execution time: " + String(time_end);
        Serial.println(light_state);
        Serial.println(exec);
      }
      break;
    case YELLOW_BEFORE_RED:
      msRed = msChange(msRed, but2);
      msGreen = msChange(msGreen, but1);
      if (millis() - traffic_timer > yellow_time * 1000) {
        reset_timer();
        progression(RED, RED, GREEN);
        time_end = cycles();
        String exec = "execution time: " + String(time_end);
        Serial.println(light_state);
        Serial.println(exec);
      }
      break;
    default:
      break;
  }
}

void progression(int newstate, int light1, int light2)  //handles state change forwarding.
{
  light_state = newstate;
  lightStates(light1, light2);
  traffic_timer = millis();
}

void lightStates(int light1,int light2)  // changes state of each object and executes light-change accordingly.
{
  traffic1.setState(light1);  // set state for light one.
  traffic2.setState(light2);
  traffic1.trafficLight();  // enact light change for light one.
  traffic2.trafficLight();
}

int msChange(int msThing, int but) {
  return msThing == 1000 ? lightbutton(but) : msThing;
}

int lightbutton(int butt) {
  return !butt ? 2000 : 1000;
}