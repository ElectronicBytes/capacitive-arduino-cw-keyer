// Capacitive CW KEYER
//
// Kinda Important notes:
// You need the capacitive sensor library here:
// Website is at http://playground.arduino.cc/Main/CapacitiveSensor?from=Main.CapSense
// Install instructions:
// Download .zip at top of page, and unzip, then plop into Arduino library folder
// --OR--
// Sketch -> Include Library -> Add .ZIP Library... -> find your .zip file.
// 
// Electronic Bytes Youtube Channel - 99 subscribes as of 4/2/17
// Find the video?
// I don't care what you do with the code, but please do share your improvements or bug fixes!
// There should be enough comments for you to figure out what's happening?
//
// Hardware setup:
// (Pin 10) -- <PIEZO BUZZER or mini speaker>
// (Pin 13) -- Base of NPN transistor
//             Emitter -> Gnd, Collector -> key line
//             (optional resistor somewhere in the transistor setup)
//
// <METAL WIRE OR PLATE>--(Pin 2)--/\/\/\--(Pin 4)--/\/\/\--(Pin 6)--<METAL WIRE OR PLATE, (I used an AC wall plug)>
//                                 470k             470k
//      (or choose another high value, google the capacitiveSensor library for details.
//
// Change the variabls below to whatever's convenient for your setup
//

// EXPERIMENTAL: (I didn't really need this, so... you get what you get. Help improve it!
// You can ALTER THE WPM SPEED with a pot connected, but UNCOMMENT SOME CODE BELOW (look for the appropriately titled section)
// (Vcc) --/\/\/\/\/\/\/\/\/\-- (Gnd)
//                  / \
//                   |
//               (Pin A0)
// Above: connect a potentiomter (any value?) from vcc to gnd, with wiper going to pin A0.
// Remember to uncomment the code if you want this to work.
//
// Open the serial monitor. If you hit the reset button at the right time, you should see numbers under 40-ish scroll by. Hold the paddle again and hit reset if values higher than that.
// It also helps to have your bare feet on the ground.
// Chang the sensitivity of the paddles by altering the numbers described in the video...


#include <CapacitiveSensor.h>
//#define DAHPIN 8 //pins 8 and 9 were for the "boring" connect-to-ground wiring
//#define DITPIN 9 // Shouldn't be too hard to alter the code to get it back to this, just change all the calls to "readCapDit and readCapDah" to reading these two pins instead.

#define DEBUG false
int buzzerPin=10;
float wpm=15;
int  waitms   = 1200/wpm;
int dahLength = 3*waitms;
int ditLength = waitms; 
int frequency = 764;
boolean alternating=false; //a tracker variable
boolean dahDown=false;
boolean ditDown=false;
unsigned long timeDone=0;

//was dit pressed first, or was dah pressed first during alternating?
unsigned long dahTime = 0;
unsigned long ditDime = 0;

//for catching people's early dit taps
boolean wasDit=false;
boolean wasDah=false;

boolean busyAlternating=0;

CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
CapacitiveSensor   cs_4_6 = CapacitiveSensor(4,6);        // 10M resistor between pins 4 & 6, pin 6 is sensor pin, add a wire and or foil

void setup() {
  Serial.begin(115200);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(13,OUTPUT); //the LED pin
  pinMode(A0, INPUT);
}

void loop() {
  //read the pins ONCE and forget about it until next run.
  //helps with early presses for dits. Send slowly, you'll notice that you hit the dits wayyy before it needs to come.
  //also saves digitalReads -> "faster" (probably undetectable by humans)




/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

  //WPM speed change. 
  // EXPERIMENTAL: UNCOMMENT OUT BELOW IF YOU WANT TO OPERATE AT VARIABLE SPEEDS
  // You need to connect a pot (any value...) between Vcc and Gnd, wiper goes to pin A0
  //*******
  //wpm = analogRead(A0)/255*20+5; //change numbers to change wpm range
  //calculateSpeed(wpm);//just call it to update the speed variables. That's it.
  //*******
  //get new pin status
  ditDown=readCapDit();
  dahDown=readCapDah();


  if(!ditDown && !dahDown){
    //both keys are down, I don't know which came first?!
    if(DEBUG) Serial.println("ALTERNATING");
    busyAlternating=true;

    if(wasDit){
      wasDit = false;
      playDit();
      //playDah();
    }else{
      playDah();
      delay(ditLength);
      playDit();  
    }
  }else if(wasDit || (!ditDown && dahDown)){
    wasDit = false;
    
    //dit only, so play a dit.
    playDit();   
  }else if(!dahDown && ditDown){
    //dah only, so play a dah.
    playDah();
  }
  
  delay(ditLength);//spac between characters
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
boolean readCapDit(){
  //returns false if key pressed, like "grounded".
  int check=500;
  if(busyAlternating) check=600;//slightly higher to avoid random blips because our finger is on the othr paddle, making the one on this paddle high too.
  int reading = cs_4_2.capacitiveSensor(30);
  if(DEBUG) {Serial.print(reading); Serial.print("\t");}
  return reading<check;
}
boolean readCapDah(){
  //also returns false if key pressed.
  int check=500;
  int reading = cs_4_6.capacitiveSensor(30);
  if(busyAlternating) check=600;
  if(DEBUG) Serial.println(reading);
  return reading<check;
}
void playDit(){
  //start the sound after recording time
  unsigned long timeNow = millis();
  tone(buzzerPin,frequency,ditLength);
  digitalWrite(13,1);
  delay(ditLength);
  digitalWrite(13,0);
}
void playDah(){
  //start the sound after recording time
  unsigned long timeNow = millis();
  tone(buzzerPin,frequency,dahLength);
  digitalWrite(13,1);
  
  //just wait a bit before checking for dah.
  delay(dahLength/2);

  //for the rest of the time, check if we hit the dit
  while(millis()<timeNow+dahLength){
      if(!readCapDit() && !busyAlternating){//dont do this if we're alternating, too many dits.
        if(DEBUG) Serial.println("DIT during DAH");
        //if we hit dit before end of dah, we'll line it up to play next.
        wasDit=true;  
      }
      delay(5);
  }
  busyAlternating = false;
  digitalWrite(13,0);
}


//After user adjusts pot, this function updats the element timings
void calculateSpeed(int wpm){
    waitms = 1200/wpm;
    dahLength = 3*waitms;
    ditLength = waitms;
}
