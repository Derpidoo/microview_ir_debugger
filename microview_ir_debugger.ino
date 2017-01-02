#include <IRremote.h>
#include <MicroView.h>

#define FONT 0

const int RECV_PIN = A1;
const int BUTTON_PIN = 2;
const int STATUS_PIN = 3;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state
int lastButtonState;

void setup() {
  Serial.begin(9600);
  uView.begin();
  clearscreen();
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  irrecv.enableIRIn(); // Start the receiver
  irrecv.blink13(true);
}

void clearscreen() {
  uView.clear(ALL); // erase hardware memory inside the OLED controller
  uView.clear(PAGE);  // erase the memory buffer, when next uView.display() is called, the OLED will be cleared.
  uView.setFontType(FONT);
  uView.setCursor(0,0);
  uView.invert(0);
  uView.contrast(0);
  uView.display();
}

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
      }
    }
  }
  else {
    if (codeType == NEC) {
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        return;
      }
      Serial.print("Received NEC: ");
      uView.println("Rx NEC: ");
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
      uView.println("Rx SONY: ");
    } 
    else if (codeType == PANASONIC) {
      Serial.print("Received PANASONIC: ");
      uView.println("Rx PSIC: ");
    }
    else if (codeType == JVC) {
      Serial.print("Received JVC: ");
      uView.println("Rx JVC: ");
    }
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
      uView.println("Rx RC5: ");
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
      uView.println("Rx RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
      uView.print("Rx ???: ");
      uView.print(codeType, DEC);
      uView.println("");
    }
    Serial.println(results->value, HEX);
    uView.println(results->value, HEX);
    codeValue = results->value;
    codeLen = results->bits;
  }
}


void sendCode(int repeat) {
  if (codeType == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, codeLen);
      Serial.println("Sent NEC repeat");
      uView.println("Sent NEC repeat");
    } else {
      Serial.print("Sent NEC: ");
      Serial.println(codeValue, HEX);
      uView.print("Sent NEC: ");
      uView.println(codeLen, DEC);
      uView.println(codeValue, HEX);
      irsend.sendNEC(codeValue, codeLen);
    }
  } 
  else if (codeType == SONY) {
    irsend.sendSony(codeValue, codeLen);
  } 
  else if (codeType == PANASONIC) {
    irsend.sendPanasonic(codeValue, codeLen);
  }
  else if (codeType == JVC) {
    irsend.sendPanasonic(codeValue, codeLen);
  }
  else if (codeType == RC5 || codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      irsend.sendRC5(codeValue, codeLen);
    } 
    else {
      irsend.sendRC6(codeValue, codeLen);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
  }
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW) {
    irrecv.enableIRIn(); // Re-enable receiver
  }

  if (buttonState) {
    clearscreen();
    Serial.println("Pressed, sending");
    uView.println("Command transmit");
    sendCode(lastButtonState == buttonState);
    delay(100); // Wait a bit between retransmissions
  }
  else if (irrecv.decode(&results)) {
    clearscreen();
    storeCode(&results);
    delay(100);
    sendCode(0);
    irrecv.enableIRIn(); // Re-enable receiver
    irrecv.resume(); // Receive the next value
  }
  uView.display();
  lastButtonState = buttonState;
}
