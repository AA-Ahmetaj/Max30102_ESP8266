
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <DFRobot_MAX30102.h>
#include <DHT.h>
#include <Servo.h>
Servo myservo;


#define DHT_PIN D4     // Replace 2 with the pin number connected to the DHT sensor
#define DHT_TYPE DHT11   // Replace DHT11 with the type of DHT sensor you are using



DFRobot_MAX30102 particleSensor;
DHT dht(DHT_PIN, DHT_TYPE);
/*
Macro definition options in sensor configuration
sampleAverage: SAMPLEAVG_1 SAMPLEAVG_2 SAMPLEAVG_4
               SAMPLEAVG_8 SAMPLEAVG_16 SAMPLEAVG_32
ledMode:       MODE_REDONLY  MODE_RED_IR  MODE_MULTILED
sampleRate:    PULSEWIDTH_69 PULSEWIDTH_118 PULSEWIDTH_215 PULSEWIDTH_411
pulseWidth:    SAMPLERATE_50 SAMPLERATE_100 SAMPLERATE_200 SAMPLERATE_400
               SAMPLERATE_800 SAMPLERATE_1000 SAMPLERATE_1600 SAMPLERATE_3200
adcRange:      ADCRANGE_2048 ADCRANGE_4096 ADCRANGE_8192 ADCRANGE_16384
*/
uint32_t tsLastReport = 0; //convert data

const char* ssid = ""; //--> Your wifi name or SSID.
const char* password = ""; //--> Your wifi password.


//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

//String GAS_ID = "165dp995JFkmX611p2fdSkd2CxatHGXCnmUCSyVMdrzU"; //--> spreadsheet deployment ID
String GAS_ID = "";
void setup()
{
  //Init serial
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");
  myservo.attach(D5);  // attach the servo to pin D5


    
  pinMode(LED_BUILTIN,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(LED_BUILTIN, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(LED_BUILTIN, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
  WiFi.setAutoReconnect(true); //this line and the following one make the connection persistent after each upload, IMPORTANT!!
  WiFi.persistent(true); 
  while (!particleSensor.begin()) {
    Serial.println("MAX30102 was not found");
    delay(1000);
  }

  /*!
   *@brief Use macro definition to configure sensor 
   *@param ledBrightness LED brightness, default value: 0x1F（6.4mA), Range: 0~255（0=Off, 255=50mA）
   *@param sampleAverage Average multiple samples then draw once, reduce data throughput, default 4 samples average
   *@param ledMode LED mode, default to use red light and IR at the same time 
   *@param sampleRate Sampling rate, default 400 samples every second 
   *@param pulseWidth Pulse width: the longer the pulse width, the wider the detection range. Default to be Max range
   *@param adcRange ADC Measurement Range, default 4096 (nA), 15.63(pA) per LSB
   */
  particleSensor.sensorConfiguration(/*ledBrightness=*/50, /*sampleAverage=*/SAMPLEAVG_4, \
                        /*ledMode=*/MODE_MULTILED, /*sampleRate=*/SAMPLERATE_100, \
                        /*pulseWidth=*/PULSEWIDTH_411, /*adcRange=*/ADCRANGE_16384);
}

int32_t SPO2; //SPO2
int8_t SPO2Valid; //Flag to display if SPO2 calculation is valid
int32_t heartRate; //Heart-rate
int8_t heartRateValid; //Flag to display if heart-rate calculation is valid 

void loop()
{
  Serial.println(F("Wait about four seconds"));
  particleSensor.heartrateAndOxygenSaturation(/**SPO2=*/&SPO2, /**SPO2Valid=*/&SPO2Valid, /**heartRate=*/&heartRate, /**heartRateValid=*/&heartRateValid);
  //Print result 
  Serial.print(F("heartRate="));
  Serial.print(heartRate, DEC);
  Serial.print(F(", heartRateValid="));
  Serial.print(heartRateValid, DEC);
  Serial.print(F("; SPO2="));
  Serial.print(SPO2, DEC);
  Serial.print(F(", SPO2Valid="));
  Serial.println(SPO2Valid, DEC);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  -  Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
  if(SPO2<=100 & SPO2>=40){
    if(heartRate<=150 & heartRate>=20){
    sendData(temperature, humidity, SPO2, heartRate);
  }
  }
  if (humidity>= 60 || temperature >=50){

    Serial.println("Motor ON!");
    for (int pos = 0; pos <= 180; pos += 1) { // sweep from 0 to 180 degrees
    myservo.write(pos);   // set the servo position
    delay(15);            // wait for the servo to reach the position
  }
    for (int pos = 180; pos >= 0; pos -= 1) { // sweep from 180 to 0 degrees
      myservo.write(pos);   // set the servo position
     delay(15);            // wait for the servo to reach the position
    }
    }
 
  //if (humidity<= 60){
    //myservo.write(0);
    //} 
}
void sendData( float temperature, float humidity, int32_t SPO2, int32_t heartRate) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  // Connecting to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  // Processing and sending data
  String string_temperature =  String(temperature);
  String string_humidity =  String(humidity); 
  String string_spo2 = String(SPO2);
  String string_bpm = String(heartRate);
  // URL Connection
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + 
  "&oxygen=" + string_spo2 + "&bpm=" + string_bpm;
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" +
      "Connection: close\r\n\r\n");
  Serial.println("request sent");
  // Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }

  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
} 
