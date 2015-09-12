#include <SoftwareSerial.h>
#include "pitches.h"
SoftwareSerial RFID(2, 3); // RX and TX
#include "Adafruit_FONA.h"

#define FONA_RX 11
#define FONA_TX 12
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

// notes in the melody:
int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
// use first sketch in http://wp.me/p3LK05-3Gk to get your tag numbers

int data1 = 0;
int ok = -1;
int yes = 14;
int no = 13;
 
// use first sketch in http://wp.me/p3LK05-3Gk to get your tag numbers
int tag1[14] = {2,51,66,48,48,56,55,50,50,55,68,69,51,3};
int tag2[14] = {2,52,48,48,48,56,54,67,54,54,66,54,66,3};
int newtag[14] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // used for read comparisons

void setup()
{
  RFID.begin(9600);    // start serial to RFID reader
  Serial.begin(9600);  // start serial to PC 
  pinMode(yes, OUTPUT); // for status LEDs
  pinMode(no, OUTPUT);
  Serial.println(F("#*** INICIANDO EL FONA ****#"));
  fonaSerial->begin(9600);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Error No puedo encontrar el FONA"));
    while(1);
  }
  Serial.println(F("El FONA esta Pura Vida!"));
}

void melodia() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 9; thisNote++) {
    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(9, melody[thisNote], noteDuration);
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(9);
  }
}

void melodia_error() {
  for (int thisNote = 0; thisNote < 1; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(9, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(9);
  }
}
 

void flushSerial() {
    while (Serial.available()) 
    Serial.read();
}

void sms_ok(){
  while (!Serial);
  Serial.begin(9600);
  Serial.println(F("Iniciando Fona...Gracias por esperar :-))"));
  fonaSerial->begin(9600);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Error no logro encontrar el FONA :-("));
    while(1);
  }
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  char sendto[9] = "83053940";
  char message[141] = "Christian, Mary acaba de llegar a Casa :-)";
  flushSerial();
  Serial.println(sendto);
  if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Error enviando Mensaje!"));
  } else {
    Serial.println(F("Notificacion enviada!...Muchas Gracias!"));
      }
}

void sms_error(){
  while (!Serial);
  Serial.begin(9600);
  Serial.println(F("Iniciando Fona...Gracias por esperar :-))"));
  fonaSerial->begin(9600);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Error no logro encontrar el FONA :-("));
    while(1);
  }
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  char sendto[9] = "83053940";
  char message[141] = "Christian, Ten cuidado! Alguien no autorizado entro a la casa :-/";
  flushSerial();
  Serial.println(sendto);
  if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Error enviando Mensaje!"));
  } else {
    Serial.println(F("Notificacion enviada!...Muchas Gracias!"));
      }
}


boolean comparetag(int aa[14], int bb[14])
{
  boolean ff = false;
  int fg = 0;
  for (int cc = 0 ; cc < 14 ; cc++)
  {
    if (aa[cc] == bb[cc])
    {
      fg++;
    }
  }
  if (fg == 14)
  {
    ff = true;
  }
  return ff;
}
 
void checkmytags() // compares each tag against the tag just read
{
  ok = 0; // this variable helps decision-making,
  // if it is 1 we have a match, zero is a read but no match,
  // -1 is no read attempt made
  if (comparetag(newtag, tag1) == true)
  {
    ok++;
  }
  if (comparetag(newtag, tag2) == true)
  {
    ok++;
  }
}
 
void readTags()
{
  RFID.listen();
  ok = -1;
  if (RFID.available() > 0) 
  {
    // read tag numbers
    delay(100); // needed to allow time for the data to come in from the serial buffer.
    for (int z = 0 ; z < 14 ; z++) // read the rest of the tag
    {
      data1 = RFID.read();
      newtag[z] = data1;
    }
    RFID.flush(); // stops multiple reads
    // do the tags match up?
    checkmytags();
  }
  // now do something based on tag type
  if (ok > 0) // if we had a match
  {
    melodia();
    Serial.println("Acceso Autorizado");
    digitalWrite(yes, HIGH);
    delay(1000);
    digitalWrite(yes, LOW);
    sms_ok();
    ok = -1;
  }
  else if (ok == 0) // if we didn't have a match
  {
    melodia_error();
    Serial.println("Acceso Denegado");
    digitalWrite(no, HIGH);
    delay(1000);
    digitalWrite(no, LOW);
    sms_error();
 
    ok = -1;
  }
}
 
void loop()
{
  readTags();
}


