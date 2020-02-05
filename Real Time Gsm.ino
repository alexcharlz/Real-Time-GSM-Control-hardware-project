#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

// Pin definitions
#define FONA_RX 9
#define FONA_TX 8
#define FONA_RST 4
#define main_ldr A0
#define ldr1 A1
#define ldr2 A2
#define ldr3 A3
#define ldr4 A4
#define ldr5 A5

#define str 6
#define power_check 11
String gsm_state = "OFF";
int main_ldr_value;
int _ldr1;
int _ldr2;
int _ldr3;
int _ldr4;
int _ldr5;
String check = "NEUTRAL";
String off_check = "HERE";
String data;

// Message buffers
char smsBuffer[255];
char senderBuffer[255];

// Numbers for send and receive
char allowedNumbers[1][15] =
{
    "+2348118569383"
};

// Fona software serial
SoftwareSerial FSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *FSerial = &FSS;

// Fona
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


void setup()
{
  // Set action pin modes
  pinMode(main_ldr, INPUT);
  pinMode(ldr1, INPUT);
  pinMode(ldr2, INPUT);
  pinMode(ldr3, INPUT);
  pinMode(ldr4, INPUT);
  pinMode(ldr5, INPUT);
  pinMode(str, OUTPUT);

  // Wait for serial
  while (!Serial);

  // Init serial 
  Serial.begin(115200);

  // Init module serial
  Serial.println("Initializing serial connection to module");
  FSerial->begin(9600);
  if(!fona.begin(*FSerial))
  {
    Serial.println("Init ERROR: Could not find SIM module!");
    while (1);
  }

  //Give the module some time so that SMS has initialized before trying to read messages
  Serial.println("Waiting for the module to fully initialize...");
  fona.type();
  delay(30000);
  fona.getNumSMS();
  delay(10000);
  Serial.println("Init done");
}



void loop()
{
  
  // If we have SMS messages they are handled
  int8_t SMSCount = fona.getNumSMS();
  if (SMSCount > 0)
  {
        Serial.println("Found SMS messages!");
        uint16_t smslen;
        for (int8_t smsIndex = 1 ; smsIndex <= SMSCount; smsIndex++)
        {
          Serial.print("Handling message ");
          Serial.println(smsIndex);

          // Read message and check error situations
          if (!fona.readSMS(smsIndex, smsBuffer, 250, &smslen))
          {
            Serial.println("Error: Failed to read message");
            continue;
          }
          if (smslen == 0)
          {
            Serial.println("Error: Empty message");
            continue;
          }

          // Read sender number so that we can check if it was authorized
          fona.getSMSSender(smsIndex, senderBuffer, 250);

          // Compare against each authorized number
          int size = sizeof allowedNumbers / sizeof allowedNumbers[0];
          for( int ind = 0; ind < size; ind = ind + 1)
          {
            if(strstr(senderBuffer,allowedNumbers[ind]))
            {

              // Authorized -> Handle action
              Serial.println("Allowed number found");
//              delay(1000);
              if(strstr(smsBuffer, "ON"))
              {
                gsm_state = "ON";
                Serial.println("Received ON!!");
              }
              else if(strstr(smsBuffer, "OFF"))
              {
                gsm_state = "OFF";
              }
              else if (strstr(smsBuffer, "AUTO")){
                gsm_state = "AUTO";
              }
              // Already found authorized and did the action so we can bail out now
              break;
            }
          }
          // Message was handled or it was unauthorized -> delete
          fona.deleteSMS(smsIndex);

          // Flush buffers
          while (Serial.available())
          {
            Serial.read();
          }
          while (fona.available())
          {
            fona.read();
          }
        }
  }

  delay(2000);
  main_ldr_value = analogRead(main_ldr);  // READ MAIN LDR VALUE
  delay(500);
  if ( gsm_state == "ON" ){
    digitalWrite(str, HIGH);
    Serial.println("Street Lights On");
 
    if (check == "NEUTRAL"){
      //delay(1000);
    String msg = run_diagnosis();
    int size = sizeof allowedNumbers / sizeof allowedNumbers[0];
    Serial.println("Send text messages to ");
    for(int i = 0; i < size; i = i + 1)
    {
      char old_message[75];
      msg.toCharArray(old_message,75);
      char* message = old_message; 
      Serial.println(allowedNumbers[i]);
      fona.sendSMS(allowedNumbers[i], message);
    }
     Serial.print("MESSAGE: ");
     Serial.println(msg);
     delay(2000);
     msg = "";
     check = "OKAY";
     off_check = "NEUTRAL";
     }
  }

  if (gsm_state == "OFF"){
      digitalWrite(str, LOW);
      if (off_check == "NEUTRAL"){
        int size = sizeof allowedNumbers / sizeof allowedNumbers[0];
        Serial.println("Send text messages to ");
        for(int i = 0; i < size; i = i + 1)
        {
          Serial.println(allowedNumbers[i]);
          fona.sendSMS(allowedNumbers[i], "System is Off");
        } 
          off_check = "OKAY";
      }
      Serial.println(" Street Lights Off");
      check = "NEUTRAL"; 
}


 if ((gsm_state == "AUTO")&&(main_ldr_value < 200)){
    Serial.print("ldr value in auto");
    Serial.println(main_ldr_value);
    digitalWrite(str, HIGH);
    Serial.println("Street Lights On");
 
    if (check == "NEUTRAL"){
      //delay(1000);
    String msg = run_diagnosis();
    int size = sizeof allowedNumbers / sizeof allowedNumbers[0];
    Serial.println("Send text messages to ");
    for(int i = 0; i < size; i = i + 1)
    {
      char old_message[75];
      msg.toCharArray(old_message,75);
      char* message = old_message; 
      Serial.println(allowedNumbers[i]);
      fona.sendSMS(allowedNumbers[i], message);
    }
     Serial.print("MESSAGE: ");
     Serial.println(msg);
     delay(2000);
     msg = "";
     check = "OKAY";
     off_check = "NEUTRAL";
   }
}

  if ((gsm_state == "AUTO")&&(main_ldr_value > 700)){
    digitalWrite(str, LOW);
    if (off_check == "NEUTRAL"){
    int size = sizeof allowedNumbers / sizeof allowedNumbers[0];
    Serial.println("Send text messages to ");
    for(int i = 0; i < size; i = i + 1)
    {
      Serial.println(allowedNumbers[i]);
      fona.sendSMS(allowedNumbers[i], "System is Off");
    } 
      off_check = "OKAY";
  }
  Serial.println(" Street Lights Off");
  check = "NEUTRAL"; 
  }
}


String run_diagnosis(){
  delay(1000);
  _ldr1 = analogRead(ldr1);
  _ldr2 = analogRead(ldr2);
  _ldr3 = analogRead(ldr3);
  _ldr4 = analogRead(ldr4);
  _ldr5 = analogRead(ldr5);
  data = "";
  if (_ldr1 < 100){data += "1,";}
  if (_ldr2 < 100){data += "2,";}
  if (_ldr3 < 100){data += "3,";}
  if (_ldr4 < 100){data += "4,";}
  if (_ldr5 < 100){data += "5,";}
  if (data == ""){data += "NO SYSTEMS FAULTY";}
  return data;
}

