#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <FS.h>
#include <Arduino.h>

#include <Servo.h>
// Definicion de pines de la nodeMCU
const int trigPin = 0;
const int echoPin = 4;
// Contadores angulares
int i = 14;
int j = 165;
//mensaje enviado por ws
String mess;
//variables del sensor de ultrasonido
long duration, distance;
// crean objeto servo para controlar el servomotor 
Servo myServo;
//puertos
ESP8266WebServer server(80);//El servidor en el puerto 80
WebSocketsServer webSocket = WebSocketsServer(81);// canal de comunicacion ws puerto 81


//Funcion para enviar los archivos en los SPIFFS al servidor
void handleRoot() {
  sendFile("index.htm", "text/html");
}

//Funcion para subir los archivos a los SPIFFS
void sendFile(String name, String type) {
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);

  // this opens the file in read-mode
  File f = SPIFFS.open("/" + name, "r");

  if (!f) {
    Serial.println("File open failed" + name );
    return;
  }
  server.streamFile(f, type);
  f.close();
}

void setup()
{



  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  myServo.attach(5); // Defines on which pin is the servo motor attached
  Serial.begin(9600);
  SPIFFS.begin();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ECOLOCALIZADOR","123456789");
  Serial.print("Accede a la pagina usando la ip: ");
  Serial.println(WiFi.softAPIP());
  server.onNotFound([]() {
  Serial.println("HTTP > Not Found");
    server.send(404, "text/plain", "FileNotFound");
  });
  webSocket.begin();			//Inicia el WebSocket
  server.begin();			//inicia el servidor
  server.serveStatic("/", SPIFFS, "/", "no-cache");// inicia servidor guardado
  server.on("/", handleRoot);

  Serial.println("Servidor iniciado");
}
float SentirDistancia() {


  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  //  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;

  if (distance >= 450 || distance <= 0) {
    return 450;
  }
  else {
    return distance;
  }

}
void loop()
{ i++;

  server.handleClient(); //Maneja los clientes
  webSocket.loop();  //Maneja los Sockets

  if ( i >= 15 && i <= 165) {
    myServo.write(i);
    mess = (String)SentirDistancia() + ":" + (String)i;
    Serial.println(mess);
    webSocket.sendTXT(0, mess);

  }

  if ( i > 165 && i <= 315) {
    myServo.write(j--);
    mess = (String)SentirDistancia() + ":" + (String)j;
    Serial.println(mess);
    webSocket.sendTXT(0, mess);
  }
  if (i == 316) {
    i = 14;
    j = 165;
  }

  delay(100);

}




