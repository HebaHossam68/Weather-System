#include "DHT.h"
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ESP32_MailClient.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;


AsyncWebServer server(80);
#define DHTTYPE DHT11 // DHT 11
uint8_t DHTPin = 4;
DHT dht(DHTPin, DHTTYPE);
float Temperature=0.0;
float Humidity;
String formattedDate;
String dayStamp;
String timeStamp;
String Pressure;
String Altitude;
float temperatureThreshold = 0.0;

const char *ssid = "Heba";
const char *password = "123456789";
#define SD_CS 5
String dataMessage;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// SMTP configuration
#define emailSenderAccount "iotproject554@gmail.com"
#define emailSenderPassword "ompe ehyc ibdt spei"
#define emailRecipient "hheba7308@gmail.com"
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "ESP32 Alert"
#define emailCompose "Mohamed Ali"

String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    return String(t);
  }
}

String readDHTHumidity() {//analog
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    return String(h);
  }
}

String readPressure() {//analog
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  float p = bmp.readPressure();
  if (isnan(p)) {
    return "--";
  }
  else {
    // Convert the float pressure value to an integer
    int pressureInt = static_cast<int>(p);
    return String(pressureInt);
  }
}

String readAltitude() {//digital
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = bmp.readAltitude(102000);
  if (isnan(h)) {
    return "--";
  }
  else {
    return String(h);
  }
}

SMTPData smtpData;
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPEHTML>

<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
      input[type=text], button {
      font-size: 1.5rem;
      margin: 10px;
      padding: 5px;

  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
      <i class="fas fa-tachometer-alt" style="color:#FFA500;"></i> 
      <span class="dht-labels">Pressure</span>
      <span id="pressure">%PRESSURE%</span>
      <sup class="units">Pa</sup>
  </p>
  <p>
    <i class="fas fa-tachometer-alt" style="color:#FFA500;"></i> 
    <span class="dht-labels">Altitude</span>
    <span id="altitude">%ALTITUDE%</span>
    <sup class="units">M</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <input type="text" id="inputValue" placeholder="Enter value...">
  <button onclick="sendValue()">Send Value</button>
   <script>
    function sendValue() {
      var inputValue = document.getElementById("inputValue").value;

      // Perform an XMLHttpRequest to send the value to the server
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          console.log("Value sent successfully:", inputValue);
        }
      };
      xhttp.open("GET", "/send-value?value=" + inputValue, true);
      xhttp.send();
    }
  </script>
</body>
</html>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200) {
document.getElementById("humidity").innerHTML = this.responseText;
}
};
xhttp.open("GET", "/humidity", true);
xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("altitude").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/altitude", true);
  xhttp.send();
}, 10000 ) ;
</script>

</html>)rawliteral";

String processor(const String& var) {
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  } else if(var == "HUMIDITY"){
    return readDHTHumidity();
  } else if(var == "PRESSURE"){
    return readPressure();
  } else if(var == "ALTITUDE"){
    return readAltitude();
  }
  return String();
}


void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}  


float Read_TempHum()
{
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();
  return Temperature;
}

void getTimeStamp()
{
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
}


void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void logSDCard()
{
  dataMessage = String(dayStamp) + "," + String(timeStamp) + "," +
                 String(Temperature) + "," + String(Humidity) + ","+String(Pressure)+","
                 +String(Altitude)+"\r\n";
  appendFile(SD, "/data.txt", dataMessage.c_str());
}

void sendCallback(SendStatus msg)
{

  if (msg.success())
  {
    Serial.println("----------------");
  }
}

void sendEmail()
{
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("ESP32", emailSenderAccount);
  smtpData.setPriority("High");
  smtpData.setSubject(emailSubject);

  // Get the current IP address
  String ipAddress = WiFi.localIP().toString();

  // Compose the email message with IP address and sensor readings
  String emailMessage = "<div style=\"color:#2f4468;\">"
                        "<h1>ESP TEST!</h1>"
                        "<p>- Temperature: " + readDHTTemperature() + "</p>"
                        "<p>- Humidity: " + readDHTHumidity() + "</p>"
                        "<p>- Pressure: " + readPressure() + " Pa</p>"
                        "<p>- Altitude: " + readAltitude() + " meters</p>"
                        "<p>- IP Address: " + ipAddress + "</p>"
                        "</div>";

  smtpData.setMessage(emailMessage, true);
  smtpData.addRecipient(emailRecipient);
  smtpData.setSendCallback(sendCallback);

  if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

  smtpData.empty();
}

void read_bmp()
{
  Pressure = readPressure();

    
  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Altitude=readAltitude();
    
  delay(500);
}
int x=33;
void setup()
{
  pinMode(x,INPUT);
  pinMode(2,OUTPUT);
  Serial.begin(115200);
  pinMode(DHTPin, INPUT);
  dht.begin();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  timeClient.begin();
  timeClient.setTimeOffset(19800);

  SD.begin(SD_CS);
  if (!SD.begin(SD_CS))
  {
    Serial.println("Card Mount Failed");
    return;
  }

  File file = SD.open("/data.txt");
  if (!file)
  {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "Date, Time, Temperature, Humidity,Pressure,Altitude \r\n");
  }
  else
  {
    Serial.println("File already exists");
  }
  file.close();

  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
	// while (1) {}
  }

  // SMTP setup
 
  // Start sending Email
  if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

  // Clear all data from Email object to free memory
  smtpData.empty();
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPressure().c_str());
  });

  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readAltitude().c_str());
  });
  server.on("/send-value", HTTP_GET, [](AsyncWebServerRequest *request){
  String value = request->arg("value");
  float newValue = value.toFloat();
  temperatureThreshold = newValue;

  // Print the updated threshold value
  Serial.println("Updated temperatureThreshold: " + String(temperatureThreshold));

  // Do something with the received value
});


  // Start server
  server.begin();

}

void loop()
{
    Temperature = Read_TempHum();
    read_bmp();
    getTimeStamp();
    logSDCard();
  if (Temperature >= temperatureThreshold)
  {
    sendEmail();
  }
  //LDR
   if (digitalRead(x)==HIGH)
  {
    digitalWrite(2,HIGH);
  }
  else
  {
    digitalWrite(2,LOW);
  }
   
  delay(10000); // Wait for 5 seconds before writing the next data
}
