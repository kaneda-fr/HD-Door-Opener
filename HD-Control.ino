// This has been developped for an Arduino Pro Micro and may need adaptation for other type of controlers.

#include <OBD2.h>
#include <mcp_can.h>
#include <SPI.h>

// Timing Variables
#define REMOTE_ON_TIMER 500 // defines how long the remote is activated
#define LED_TIMER_DEFAULT 1000 // default led period
#define LED_TIMER_FAST 100 // led period when remote is on
#define SIGNAL_DELAY 1000 // max timing between signal and beam
#define REMOTE_DELAY 1000 // min delay between remote activations

unsigned long prevTx = 0;
unsigned int invlTx = LED_TIMER_DEFAULT;
unsigned long leftSignalTx = 0;
unsigned long rightSignalTx = 0;
unsigned long beamTx = 0;
unsigned long remoteOnTx = 0;

bool remoteOn = false; // state of remote door opener signal

// CAN RX Variables
unsigned long rxID;
byte dlc;
byte rxBuf[8];
char msgString[128];                        // Array to store serial string

// CAN Interrupt and Chip Select Pins
#define CAN0_INT 2                              /* Set INT to pin 2 (This rarely changes)   */
MCP_CAN CAN0(10);                                /* Set CS to pin 9 (Old shields use pin 10) */

// define CAN Mask & Filter
#define MSG_MASK  0x5300000
#define MSG_FILTER 0x5300000

/// Define IO pin
#define LED_PIN 9
#define REMOTE_PIN_1 8
#define REMOTE_PIN_2 7

// macros (TXLED1, TXLED0) to control blinking.
// (We could use the same macros for the RX LED too -- RXLED1,
//  and RXLED0.)

bool led_state=true;

// the setup function runs once when you press reset or power the board
void setup() {
  RXLED0;
  TXLED0;
 
  RXLED1;   
  TXLED1; 
 
  Serial.begin(115200);

  // initialize digital pin LED as an output.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // initialize remote pin as an output.
  pinMode(REMOTE_PIN_1, OUTPUT);
  pinMode(REMOTE_PIN_2, OUTPUT);

  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) 
    {
      Serial.println("MCP2515 Initialized Successfully!");
    }
  else 
    {
      Serial.println("Error Initializing MCP2515...");
      while (1){
        digitalWrite(LED_PIN, HIGH); 
        RXLED1;
        TXLED1;
        delay(LED_TIMER_FAST);
        digitalWrite(LED_PIN, LOW); 
        RXLED0;
        TXLED0; 
        delay(LED_TIMER_FAST);
      }
    }

  TXLED0;
  // Setup CAN mask & Filter
  // Standard ID Filters
  CAN0.init_Mask(0,MSG_MASK);                // Init first mask...
  CAN0.init_Filt(0,MSG_FILTER);                // Init first filter...
  CAN0.init_Filt(1,MSG_FILTER);                // Init second filter...

  // Allow all Extended IDs
  CAN0.init_Mask(1,0x80000000);                // Init second mask...
  CAN0.init_Filt(2,0x80000000);                // Init third filter...
  CAN0.init_Filt(3,0x80000000);                // Init fourth filter...
  CAN0.init_Filt(4,0x80000000);                // Init fifth filter...
  CAN0.init_Filt(5,0x80000000);                // Init sixth filter...

  CAN0.setMode(MCP_NORMAL);                      // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                          // Configuring pin for /INT input

  RXLED0;   
  TXLED0; 
}

// the loop function runs over and over again forever
void loop() {
  if(!digitalRead(CAN0_INT)){
    if (CAN0.readMsgBuf(&rxID, &dlc, rxBuf) == CAN_OK) { // Read CAN data: dlc = data length, rxBuf = data byte(s)
      if (rxID == 0x530) {
        int i = 2;
        //Serial.print(msgString);
        if (rxBuf[i] == 0x81){
          sprintf(msgString,"");
        } else if (rxBuf[i] == 0x91){
          Serial.println("LEFT");
          leftSignalTx = millis();
        } else if (rxBuf[i] == 0x89){
          Serial.println("RIGHT");          
          rightSignalTx = millis();
        } else if (rxBuf[i] == 0xC1){
          Serial.println("BEAM");
          beamTx = millis();
          if (remoteOn == false && ((beamTx - leftSignalTx) < SIGNAL_DELAY) && ((millis() - remoteOnTx)) > REMOTE_DELAY){
            remoteOn = true;
            remoteOnTx = beamTx;
            Serial.println("REMOTE ON");
            digitalWrite(REMOTE_PIN_1, HIGH);
            invlTx = LED_TIMER_FAST;
          }
          if (remoteOn == false && ((beamTx - rightSignalTx) < SIGNAL_DELAY) && ((millis() - remoteOnTx)) > REMOTE_DELAY){
            remoteOn = true;
            remoteOnTx = beamTx;
            Serial.println("REMOTE ON");
            digitalWrite(REMOTE_PIN_2, HIGH);
            invlTx = LED_TIMER_FAST;
          }
        }
      }
    } else
			Serial.println(F("Interrupt is asserted, but there were no messages to process..."));      
    
    if(remoteOn == true  && (millis() - remoteOnTx) >= REMOTE_ON_TIMER){
      remoteOn = false;
      Serial.println("REMOTE OFF");
      digitalWrite(REMOTE_PIN_1, LOW);
      digitalWrite(REMOTE_PIN_2, LOW);
      //digitalWrite(LED_PIN, HIGH); 
      invlTx = LED_TIMER_DEFAULT;
    }   
  } 

  if((millis() - prevTx) >= invlTx){
      prevTx = millis();
      //digitalWrite(LED_PIN, digitalRead(LED_PIN) == HIGH ? LOW : HIGH);
      if (led_state){
        TXLED0;
        RXLED1;
      } else {
        TXLED1;
        RXLED0;
      }
      led_state = ! led_state;
    }
}
