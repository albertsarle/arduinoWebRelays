#include <SPI.h>
#include <Ethernet.h>
#include <dht11.h>
#include <avr/interrupt.h>
#define MaxHeaderLength 25    //maximum length of http header required

/////////  INICI PART TEMPERATURA
dht11 DHT11;
int DHT11Pin = 5;

float humitat;
float temperatura;
float rosada;

float LM35_1;
int LM35_1_Pin = 0;
float LM35_2;
int LM35_2_Pin = 1;



/////////  INICI PART WEBSERVER

// la xarxa local de Casserres es aquesta: 
// nanostation2 (gateway) 192.168.1.1 amb nat al port 19710 cap al 80 del arduino
// linksys WRT54GL (access point)  192.168.1.2
// arduino (webserver) 192.168.1.3
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 
  192,168, 1, 3 };
byte gateway[] ={ 
  192, 168, 1, 1 };
EthernetServer server(80);
String HttpHeader = String(MaxHeaderLength); 


/////////  INICI PART RELAYS 
int switchPin1 = 2;   // switch button input
int switchPin2 = 3;   // switch button input
int relayPin1 = 7;   // IN1 connected to digital pin 7
int relayPin2 = 8;   // IN2 connected to digital pin 8


volatile int val1 = 0;    // el valor que recollim del pin d'entrada
volatile int val2 = 0;    // el valor que recollim del pin d'entrada
volatile int buttonState1 = 0;  // variable to hold the last button state
volatile int buttonState2 = 0;  // variable to hold the last button state

/// arduino debounce interrupts http://www.instructables.com/id/Arduino-Software-debouncing-in-interrupt-function/
long debouncing_time = 400; //Debouncing Time in Milliseconds
volatile unsigned long last_micros1;
volatile unsigned long last_micros2;

int lightStatus1 = 0;
int lightStatus2 = 0;  

void initWebServer()
{
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  HttpHeader="";
}

void webServerDaemon() 
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();


        //read MaxHeaderLength number of characters in the HTTP header
        //discard the rest until \n
        if (HttpHeader.length() < MaxHeaderLength)
        {
          //store characters to string
          HttpHeader = HttpHeader + c;
        }        

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {

          int api = HttpHeader.indexOf("/api/");

          String apiCallback;           
          if (api>0) {


            int switch1 = HttpHeader.indexOf("switch1");
            if (switch1>0) {

              int p1 = HttpHeader.indexOf("s=1");

              if (p1>=0) {
                digitalWrite(relayPin1, LOW);   
                lightStatus1 = 1;
                apiCallback = "callBack(1, 1);";                  
              } 
              else 
              {
                digitalWrite(relayPin1, HIGH);  
                lightStatus1 = 0;
                apiCallback = "callBack(1, 0);";

              }

            }
            int switch2 = HttpHeader.indexOf("switch2");
            if (switch2>0) {

              int p2 = HttpHeader.indexOf("s=1"); 
              if (p2>0) {
                digitalWrite(relayPin2, LOW);   
                lightStatus2 = 1;
                apiCallback = "callBack(2, 1);";
              } 
              else 
              {
                digitalWrite(relayPin2, HIGH);  
                lightStatus2 = 0;
                apiCallback = "callBack(2, 0);";                  
              } 
            }            
          }          
          
          String classLightStatus1;
          String classLightStatus2;


          if (lightStatus1 == 1){
            classLightStatus1 = "on";
          } 
          else {
            classLightStatus1 = "off";
          }          

          if (lightStatus2 == 1){
            classLightStatus2 = "on";
          } 
          else {
            classLightStatus2 = "off";
          }

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();  

          if (api>0) {
            client.println(apiCallback);
          }
          else {
            client.println("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>Connectant un arduino a guifi.net 1</title>");
            client.println("<meta name=\"author\" content=\"albertsarle@gmail.com\">");
            client.println("<script src=\"http://code.jquery.com/jquery-1.10.1.min.js\"></script>");
            client.println("<script src=\"http://dl.dropboxusercontent.com/u/1834023/arduino/temp.js\"></script>");
            client.println("<link href=\"http://dl.dropboxusercontent.com/u/1834023/arduino/temp.css\" rel=\"stylesheet\" type=\"text/css\" />");
            client.println("</head><body><div id=\"container\">");
            client.println("<div class=\"block temperature\">out ");

            char buffer[10];
            String temperaturaS = dtostrf(temperatura, 5, 1, buffer);            
            client.println(temperaturaS);

            client.println(" °C</div>");
            client.println("<div class=\"block temperature\">in ");

            String LM35_1S = dtostrf(LM35_1, 5, 1, buffer);            
            client.println(LM35_1S);

            client.println(" °C</div>");
            client.println("<div class=\"block\" id=\"humidity\">");

            String humitatS = dtostrf(humitat, 5, 1, buffer);            
            client.println(humitatS);

            client.println(" %</div>");
            client.println("<div class=\"block switch ");
            client.println(classLightStatus1);
            client.println("\" id=\"switch1\">heat is <span>");
            client.println(classLightStatus1);
            client.println("</span></div>");

            client.println("<div class=\"block switch ");
            client.println(classLightStatus2);
            client.println("\" id=\"switch2\">water is <span>");
            client.println(classLightStatus2);
            client.println("</span></div>");

            client.println("</div>");
            client.println("</body></html>");          
          }


          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    HttpHeader="";
    // close the connection:
    client.stop();
  }
}

/////////  FINAL PART WEBSERVER


void buttonRelaysInit() {
  pinMode(switchPin1, INPUT);
  pinMode(switchPin2, INPUT);  

  pinMode(relayPin1, OUTPUT);      // sets the digital pin as output
  pinMode(relayPin2, OUTPUT);      // sets the digital pin as output

  digitalWrite(relayPin1, HIGH);        // Prevents relays from starting up engaged
  digitalWrite(relayPin2, HIGH);        // Prevents relays from starting up engaged

  buttonState1 = digitalRead(switchPin1);   // read the initial state
  buttonState2 = digitalRead(switchPin2);   // read the initial state  
}

void processRelays()
{
  val1 = digitalRead(switchPin1);   // read input value and store it in val 
  val2 = digitalRead(switchPin2);   // read input value and store it in val   

  if (val1 != buttonState1) {          // the button state has changed!
    if (val1 == HIGH) {                // check if the button is pressed
      Serial.println("Button 1 just pressed");
      if (lightStatus1 == 1){
          digitalWrite(relayPin1, HIGH);   // energizes the relay and lights the LEDx
          lightStatus1 = 0;
           Serial.println("Apaguem el llum 1 ");
         
      } else {
          digitalWrite(relayPin1, LOW);   // energizes the relay and lights the LEDx
          lightStatus1 = 1;
           Serial.println("Engeguem el llum 1 ");
          
      }       
    } else {                         // the button is -not- pressed...
      Serial.println("Button 1 just released");
    }
  }
 
  if (val2 != buttonState2) {          // the button state has changed!
    if (val2 == HIGH) {                // check if the button is pressed
      Serial.println("Button 2 just pressed");
      if (lightStatus2 == 1){
          digitalWrite(relayPin2, HIGH);   // energizes the relay and lights the LEDx
          lightStatus2 = 0;
           Serial.println("Engeguem el llum 2 ");
      } else {
          digitalWrite(relayPin2, LOW);   // energizes the relay and lights the LEDx
          lightStatus2 = 1;
           Serial.println("Apaguem el llum 2 ");         
      }       
    } else {                         // the button is -not- pressed...
      Serial.println("Button 2 just released");
    }
  }
 
 
  //Serial.println(val1);
  buttonState1 = val1;                 // save the new state in our variable
  buttonState2 = val2;                 // save the new state in our variable 
}


void processRelay1()
{

  if (lightStatus1 == 0){
    digitalWrite(relayPin1, LOW);  
    lightStatus1 = 1;
  } 
  else {
    digitalWrite(relayPin1, HIGH);   
    lightStatus1 = 0;
  }        
}

void processRelay2()
{
  if (lightStatus2 == 0){
    digitalWrite(relayPin2, LOW);  
    lightStatus2 = 1;
  } 
  else {
    digitalWrite(relayPin2, HIGH);   
    lightStatus2 = 0;
  }        
}

/////////  FINAL PART RELAYS


void temperature()
{
  int chk = DHT11.read(DHT11Pin);

  humitat = (float)DHT11.humidity;
  temperatura = (float)DHT11.temperature;  
  rosada = DHT11.dewPoint();  

  LM35_1 = analogRead(LM35_1_Pin);
  LM35_1 = LM35_1 * 0.48828125;

  LM35_2 = analogRead(LM35_2_Pin);
  LM35_2 = LM35_2 * 0.48828125;

}

void debounceInterrupt1() {
  if((long)(micros() - last_micros1) >= debouncing_time * 1000) {
    processRelay1();
    last_micros1 = micros();
  }
}

void debounceInterrupt2() {
  if((long)(micros() - last_micros2) >= debouncing_time * 1000) {
    processRelay2();
    last_micros2 = micros();
  }
}

void setup() {                
  Serial.begin(9600);
  temperature();
  buttonRelaysInit();
  initWebServer();

  //attachInterrupt(0, debounceInterrupt1, FALLING);
  //attachInterrupt(1, debounceInterrupt2, FALLING);
  lightStatus1 =0;
  lightStatus2 =0;  
}

void loop() {
  processRelays();
  temperature();
  webServerDaemon(); 
  delay(100);       
}
