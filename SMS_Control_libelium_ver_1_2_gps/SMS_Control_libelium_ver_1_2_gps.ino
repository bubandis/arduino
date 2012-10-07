//message format:
//password#command*time
//  3425#start*30

#include <MsTimer2.h>
//#include <SimpleTimer.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#define maxLength 256

String inData = String(maxLength);
char data[maxLength];
String number = String(12);
char c_number[12];
//char phone_number[]="+79222620280";
int x;
int onModulePin = 2;
int led = 13;
int IdxCmd;
boolean logg = true;

TinyGPS gps;

//SimpleTimer timer;

SoftwareSerial nss(9, 8);
SoftwareSerial debug = SoftwareSerial(11, 10);

//*****************************************************************************************
void switchModule(){
  digitalWrite(onModulePin,HIGH);
  delay(2000);
  digitalWrite(onModulePin,LOW);
}

//**************************************************************
void HeaterStart() {
  digitalWrite(led, HIGH);
}

//**************************************************************
void HeaterStop() {
  digitalWrite(led, LOW);
  MsTimer2::stop();
  if (logg) {
    debug.println("Heater stop.");
  }
}

//**************************************************************
void getSerialChars() {
  for (x=0;x <= 255;x++){
    data[x]='\0';                        
  }
  x=0;
  while (Serial.available() > 0) {
    data[x] = Serial.read();
    x++;
//    if(data[x-1]==0x0D&&data[x-2]=='"'){
//      x=0;
//    }
  }
  inData = data;
  if (logg && inData.length() > 0) {
    debug.println(inData);
  }
}

//**************************************************************
void GetGPSData(TinyGPS &gps) {
  bool newdata = false;
  unsigned long start = millis();
  unsigned long age, date, time, chars = 0;
  unsigned long sat, hdopl;
  float flat, flon;
  float alt, cou, spd;
  char c_lat[10], c_lon[10], c_cou[6], c_alt[6], c_spd[6];
  
  while (millis() - start < 1000)
  {
    if (feedgps())
      newdata = true;
  }  
  if (logg) {
    debug.print("sat: ");
  }
  sat = gps.satellites();
  if (logg) {
    debug.print(sat);
  }
  feedgps();
  if (logg) {
    debug.print(" hdop: ");
  }
  hdopl = gps.hdop();
  if (logg) {
    debug.print(hdopl);
  }
  feedgps();
  gps.f_get_position(&flat, &flon, &age);
  if (logg) {
    debug.print(" lat: ");
    debug.print(flat, 6);
  }
  feedgps();
  if (logg) {
    debug.print(" lon: ");
    debug.print(flon, 6);
  }
  feedgps();
  if (logg) {
    debug.print(" alt: ");
  }
  alt = gps.f_altitude();
  if (logg) {
    debug.print(alt);
  }
  feedgps();
  if (logg) {
    debug.print(" cou: ");
  }
  cou = gps.f_course();
  if (logg) {
    debug.print(cou);
  }
  feedgps();
  if (logg) {
    debug.print(" spd: ");
  }
  spd = gps.f_speed_kmph();
  if (logg) {
    debug.print(spd);
  }
  feedgps();
  if (logg) {
    debug.println();
  }
  
  dtostrf(flat, 2, 6, c_lat);
  dtostrf(flon, 2, 6, c_lon);
  dtostrf(cou, 2, 6, c_cou);
  dtostrf(spd, 2, 6, c_spd);
  dtostrf(alt, 2, 6, c_alt);
  
  Serial.print("AT+CMGS=\"");
  Serial.print(number);
  Serial.println("\"");

  if (logg) {
    debug.print("number: ");
    debug.println(number);
  }
   
  while(Serial.read()!='>');
  
  if (logg) {
    debug.println("read > ");
  }
  
  Serial.print("http://maps.google.com/maps?ll=");
  
  if (logg) {
    debug.println("set the text ");
  }
  Serial.print(String(c_lat));
  Serial.print(String(c_lon));
  Serial.print("&q=");
  Serial.print(String(c_lat));
  Serial.print(",");
  Serial.print(String(c_lon));
  Serial.println("");

  Serial.print("lat: ");
  Serial.print(String(c_lat));
  Serial.print(" lon: ");
  Serial.print(String(c_lon));
  Serial.print(" cou: ");
  Serial.print(String(c_cou));
  Serial.print(" spd: ");
  Serial.print(String(c_spd));
  Serial.print(" alt: ");
  Serial.print(String(c_alt));
  Serial.println("");//

  delay(500);
  Serial.write(0x1A);       //sends ++
  if (logg) {
    debug.println("ending sms ");
  }
  Serial.write(0x0D);
  Serial.write(0x0A);
}

//**************************************************************
void setup() {
  // GSM
  Serial.begin(9600);
  delay(2000);
    
  // Debug port
  debug.begin(9600);
  delay(200);
  
  // GPS
  if (logg) {
    debug.println("Setting up GPS...");
  }
  nss.begin(4800);
  if (logg) {
    debug.println("Done setting up GPS!");
  }  
  delay(200);

  pinMode(led, OUTPUT);
  pinMode(onModulePin, OUTPUT);

  HeaterStop();
  if (logg) {
    debug.println("Starting GSM module...");
  }
  switchModule();
  for (int i=0;i < 5;i++){
      delay(6000);
  }
  Serial.println("AT+CLIP=1");
  delay(1000);
  Serial.println("AT+CMGF=1");
  delay(1000);
  Serial.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
  if (logg) {
    debug.println("Started");
  }  
}

//**************************************************************
void loop() {
  getSerialChars();
  //Serial.flush();
  // SMS
  IdxCmd = inData.indexOf("+CMT:");
  if (IdxCmd != -1) {
    number = inData.substring(IdxCmd + 7, IdxCmd + 19);
    number.toCharArray(c_number,13);
    if (logg) {
      debug.print("phone number sms: ");
      debug.println(number);
    }
    if (inData.indexOf("3425#") != -1) {
      IdxCmd = inData.indexOf("#");
      int IdxTim = inData.indexOf("*");
      String command = String(10);
      String TimeStr = String(2);
      command = inData.substring(IdxCmd + 1, IdxTim);
      if (logg) {
        debug.print("Cmd: ");
        debug.println(command);
      }
      TimeStr = inData.substring(IdxTim + 1, inData.length());
      char this_char[TimeStr.length() + 1];
      TimeStr.toCharArray(this_char, sizeof(this_char));
      unsigned long Time = atoi(this_char);
      Time = Time * 1000 * 60;
      if (command == "start") {
        HeaterStart();
        MsTimer2::set(Time, HeaterStop);
        MsTimer2::start();
        if (logg) {
          debug.print("start on ");
          debug.println(Time);
        }
      }
      if (command.indexOf("stop") != -1) {
        HeaterStop();
      }
      if (command == "getpos") {
        GetGPSData(gps);
      }
      deleteAllMsgs();
    }
  }
  // Incoming ring
  int IdxIncNum = inData.indexOf("+CLIP:");
//  debug.print("IdxIncNum");
//  debug.println(IdxIncNum);
  if (IdxIncNum != -1) {
    number = inData.substring(IdxIncNum + 8, IdxIncNum + 20);
    number.toCharArray(c_number,13);
    if (logg) {
      debug.print("phone number ring: ");
      debug.println(number);
    }
    if (number == "+79222620280") {
      HeaterStart();
      unsigned long TimeStartRing = 1200000;
      MsTimer2::set(TimeStartRing, HeaterStop); // 1200000 - 20 min.
      MsTimer2::start();
      //timer.setTimeout(1200000, HeaterStop);
      if (logg) {
        debug.print("start on ring: ");
        debug.print(TimeStartRing);
        debug.println(" msec.");
      }
      delay(2000);
      Serial.println("ATH");              // disconnect the call
    }
  }
  //timer.run();
  delay(1000);
}

//**************************************************************
void deleteAllMsgs() {
  delay(1000);
  if (logg) {
    debug.println("deleting sms");
  }
  Serial.println("AT+CMGD=1,4");
  delay(1000);
}

//**************************************************************
static bool feedgps()
{
  while (nss.available())
  {
    if (gps.encode(nss.read()))
      return true;
  }
  return false;
}
