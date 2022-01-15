#include <Stepper.h>
#include <IRremote.h>

// -- IR config --
const int irrecv_pin = 7;
const unsigned long one_press_interval = 100;  // consecutive ir signals are taken as one.
unsigned long previous_receive_time = 0;  
IRrecv irrecv(irrecv_pin);
decode_results results;

// -- Stepper config --
const int steps_per_revolution = 800;  
const int steps_left_bound = -steps_per_revolution/2;
const int steps_right_bound = steps_per_revolution/2;
const unsigned long stepper_stop_interval = 180*1000UL;
int stepper_position = 0;
int stepper_dir = 1;
int stepper_zero_cross_count = 0;
bool is_stepper_move=true;
unsigned long previous_stepper_millis = 0;  
Stepper myStepper(steps_per_revolution, 8, 10, 9, 11);

// -- LED config --
const int green_led_pin = 2;
const int yellow_led_pin = 6;
const int led_on_time = 2; // number of millisecs between blinks
const int led_off_time = 10; // number of millisecs between blinks
byte led_state = LOW;             // used to record whether the LEDs are on or off
const int moving_led_interval = 250;

unsigned long previous_moving_on_time = 0;
bool move_led_status = false;
unsigned long previous_led_millis = 0;   // will store last time the LED was updated


// -- general status --
unsigned long current_millis = 0;    // stores the value of millis() in each iteration of loop()
bool overall_state = false;


void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn();
    irrecv.blink13(true);
    myStepper.setSpeed(20);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(green_led_pin, OUTPUT);
    pinMode(yellow_led_pin, OUTPUT);
  
    digitalWrite(LED_BUILTIN, LOW);  
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(yellow_led_pin, HIGH);
    delay(0.5);
}


void loop() {  
    current_millis = millis(); 
    update_ir_receiver();
      
    if (overall_state){  
        update_stepper();
        if (is_stepper_move){         
            if (current_millis - previous_moving_on_time > moving_led_interval){   
                move_led_status = !move_led_status;
                previous_moving_on_time = current_millis;
            }
         
            if (move_led_status){
               update_led(green_led_pin); 
            }
            else{
               digitalWrite(green_led_pin, LOW);
            }
       }
       else{         
          update_led(green_led_pin); 
       }   
            
       digitalWrite(yellow_led_pin, LOW);
     }
     else{
          digitalWrite(green_led_pin, LOW);
          update_led(yellow_led_pin);  
          stepper_position = 0;
          stepper_dir = 1;
          is_stepper_move=true;
          previous_stepper_millis = current_millis;
     }  
}


void update_ir_receiver(){
        if (irrecv.decode()){
        //Serial.println(results.value, HEX);
        irrecv.resume();          

        if (current_millis - previous_receive_time > one_press_interval){
            overall_state = !overall_state; 
            Serial.print("IR received!\n");        
            stepper_zero_cross_count = 0;
        }
        previous_receive_time = current_millis;    
    }
}


void update_stepper(){
    if (stepper_position == 0){             
       if (is_stepper_move == false){
          if (current_millis - previous_stepper_millis > stepper_stop_interval){
              is_stepper_move = true;
              move_led_status = true;
              previous_stepper_millis = current_millis;
              stepper_zero_cross_count = 2;
          }
      }
      else if (is_stepper_move == true){
          stepper_zero_cross_count += 1;
          if (stepper_zero_cross_count == 3){            
              is_stepper_move = false;
              move_led_status = false;
              previous_stepper_millis = current_millis;                                
          }             
          Serial.println(stepper_zero_cross_count);
      }         
    }   
  
   
    if (!is_stepper_move){
        return;  
    }
  
    if (stepper_position <= steps_left_bound){
        stepper_dir = 1;
    }
    else if (stepper_position >= steps_right_bound){
        stepper_dir = -1;
    }    
    
    myStepper.step(stepper_dir);
    stepper_position += stepper_dir;    
}


void update_led(int led_pin) {
 if (led_state == LOW) {
   if (current_millis - previous_led_millis >= led_off_time) {
      led_state = HIGH;
      previous_led_millis += led_off_time;
   }
 }
 else {   
   if (current_millis - previous_led_millis >= led_on_time) {
      led_state = LOW;
      previous_led_millis += led_on_time;
   }
 }
 digitalWrite(led_pin, led_state);
}
