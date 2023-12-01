// * COMUNICAÇÃO DE DADOS - JORIO LEMOS 2023_1 *

// Biblio Tela Nextion
#include "Nextion.h"

// Bibliotecas de conectividade
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <WiFiClientSecure.h>

// Bibliotecas do Sensor BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Bibliotecas auxiliares de publicação
#include <ArduinoJson.h>
#include <PubSubClient.h>

// Biblioteca NTP
#include <NTPClient.h>
#include <Arduino.h>

// Configurações de relógio on-line
WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000); // Cria um objeto "NTP" com as configurações.

// Led teste
const int led = 5;
#define LED_BUILTIN 2

/* Pressao referencia da estacao*/
#define SEALEVELPRESSURE_HPA (1014.0)

Adafruit_BME280 bme;
float temperature, humidity, pressure, altitude;

/*SSID & Password VIVOFIBRA-46B1 ufKBrTWp8p */
const char* ssid = "PH";
const char* password = "21216435";

//---- MQTT Broker
const char* mqtt_server      = "78302af2bede4fe6a6c4dda424e3a851.s1.eu.hivemq.cloud";
const char* mqtt_username    = "joriolemos";
const char* mqtt_password    = "Jo@0509_rio";
const int   mqtt_port        = 8883;

/**** Secure WiFi Connectivity Initialisation *****/
WiFiClientSecure espClient;

/**** MQTT Client Initialisation Using WiFi Connection *****/
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];


/****** root certificate *********/

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

void timeupdate() {
  if (ntp.update()) {
    Serial.print("DATA/HORA: ");
    Serial.println(ntp.getFormattedDate());

    Serial.print("HORARIO: ");
    Serial.println(ntp.getFormattedTime());
  } else {
    Serial.println("!Erro ao atualizar NTP!\n");
  }
}

void setup_wifi(){

  delay(10);
  Serial.println("\nConnecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
}

/************* Connect to MQTT Broker ***********/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe("led_state");   // subscribe the topics here

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/***** Call back Method for Receiving MQTT messages and Switching LED ****/

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);

  //--- check the incomming message
    if( strcmp(topic,"led_state") == 0){
     if (incommingMessage.equals("1")) digitalWrite(led, HIGH);   // Turn the LED on
     else digitalWrite(led, LOW);  // Turn the LED off
  }

}

/**** Method for Publishing MQTT Messages **********/
void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
      Serial.println("Message publised ["+String(topic)+"]: "+payload);
}

WebServer server(80);

/* Parte de envio de dados Nextion*/
NexPage p0 = NexPage(0, 0, "page0");
NexNumber p0_n0 = NexNumber(0, 2, "n0");
NexNumber p0_n1 = NexNumber(0, 3, "n1");
NexNumber p0_n2 = NexNumber(0, 4, "n2");
NexNumber p0_n3 = NexNumber(0, 5, "n3");
NexDSButton bt0 = NexDSButton(0, 15, "bt0");

// Declaração de variaveis
uint32_t next, myInt0, myInt1, myInt2, myInt3 = 0;
uint32_t dual_state;
uint32_t L_Temp, L_Humi, L_Pres, L_Alti;

// Daddos dos parametros para o Display local
void do_every_so_often() {
  myInt0 = bme.readTemperature();
  myInt1 = bme.readHumidity();
  myInt2 = bme.readPressure() / 100.0F;
  myInt3 = bme.readAltitude(SEALEVELPRESSURE_HPA);
  p0_n0.setValue(myInt0);
  p0_n1.setValue(myInt1);
  p0_n2.setValue(myInt2);
  p0_n3.setValue(myInt3);
}

void setup() {
  
  //
  nexInit();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(led, OUTPUT);
  next = millis();

  Serial.begin(115200);
  delay(100);

  bme.begin(0x77);
  setup_wifi();
  ntp.begin();

  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

}

// Dados dos sensores para publicação Webserver local
void handle_OnConnect() {

  temperature = bme.readTemperature();
  humidity    = bme.readHumidity();
  pressure    = bme.readPressure() / 100.0F;
  altitude    = bme.readAltitude(SEALEVELPRESSURE_HPA);

  server.send(200, "text/html", SendHTML(temperature, humidity, pressure, altitude));
}

// Serviço HTML local fora do ar
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

//######################################## Loop begin!

void loop() {

  timeupdate();

  String dataString = ntp.getFormattedDate();

  // Extrai o ano, mês e dia
  String ano = dataString.substring(0, 4);
  String mes = dataString.substring(5, 7);
  String dia = dataString.substring(8, 10);

  // Reorganiza no formato "DD/MM/AAAA"
  String dataFormatada = dia + "/" + mes + "/" + ano;

  String horario = ntp.getFormattedTime();

  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();

  // Definindo o tamanho do buffer para armazenar o JSON
  const size_t capacity = JSON_OBJECT_SIZE(8) + 200;

  DynamicJsonDocument doc(capacity);

  // Dados dos parametros para o monitor serial

  float L_Temp = bme.readTemperature();
  float L_Humi = bme.readHumidity();
  float L_Pres = bme.readPressure() / 100.0F;
  float L_Alti = bme.readAltitude(SEALEVELPRESSURE_HPA);

  client.publish("joriolemos/Temperatura", String(L_Temp).c_str());
  client.publish("joriolemos/Umidade", String(L_Humi).c_str());
  client.publish("joriolemos/Pressao", String(L_Pres).c_str());
  client.publish("joriolemos/Altitude", String(L_Alti).c_str());    

// Limitando os valores para duas casas decimais
  L_Temp = roundf(L_Temp * 100) / 100; // Limita para 2 casas decimais
  L_Humi = roundf(L_Humi * 100) / 100; // Limita para 2 casas decimais
  L_Pres = roundf(L_Pres * 100) / 100; // Limita para 2 casas decimais
  L_Alti = roundf(L_Alti * 100) / 100; // Limita para 2 casas decimais

// Montando o documento JSON
  doc["deviceId"]     = "ESP32 DEVIKT V1";
  doc["siteId"]       = "IFES";
  doc["Data"]         = dataFormatada;
  doc["Horario"]      = horario;
  doc["Temperatura"]  = float(L_Temp); // Convertendo para float
  doc["Umidade"]      = float(L_Humi); // Convertendo para float
  doc["Pressão"]      = float(L_Pres); // Convertendo para float
  doc["Altitude"]     = float(L_Alti); // Convertendo para float

  char mqtt_message[256];
  serializeJson(doc, mqtt_message);

  publishMessage("Esp32_data", mqtt_message, true);

  Serial.println("");
  Serial.println("Dados dos Sensor");
  Serial.println(L_Temp);
  Serial.println(L_Humi);
  Serial.println(L_Pres);
  Serial.println(L_Alti);
  Serial.println("");
  Serial.println("Webserver local");
  Serial.println(WiFi.localIP());
  Serial.println("");

  server.handleClient();
  do_every_so_often();
  bt0.getValue(&dual_state);
  if (dual_state > 0) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(5000);

}

// Publicação HTML com Webserver local
String SendHTML(float temperature, float humidity, float pressure, float altitude) {
  String ptr = "<!DOCTYPE html>";
  ptr += "<html>";
  ptr += "<head>";
  ptr += "<title>ESP32 Weather Station</title>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr += "<style>";
  ptr += "html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr += "body{margin: 0px;} ";
  ptr += "h1 {margin: 50px auto 30px;} ";
  ptr += ".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr += ".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr += ".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr += ".temperature .reading{color: #F29C1F;}";
  ptr += ".humidity .reading{color: #3B97D3;}";
  ptr += ".pressure .reading{color: #26B99A;}";
  ptr += ".altitude .reading{color: #955BA5;}";
  ptr += ".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
  ptr += ".data{padding: 10px;}";
  ptr += ".container{display: table;margin: 0 auto;}";
  ptr += ".icon{width:65px}";
  ptr += "</style>";
  ptr += "<meta http-equiv='refresh' content='2'>";
  ptr += "</head>";
  ptr += "<body>";
  ptr += "<h1>IFES-SERRA Weather Station</h1>";
  ptr += "<h3> Jorio Enterprises </h3>";
  ptr += "<div class='container'>";
  ptr += "<div class='data temperature'>";
  ptr += "<div class='side-by-side icon'>";
  ptr += "<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr += "C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr += "c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr += "c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr += "s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  ptr += "</div>";
  ptr += "<div class='side-by-side text'>Temperature</div>";
  ptr += "<div class='side-by-side reading'>";
  ptr += (int)temperature;
  ptr += "<span class='superscript'>&deg;C</span></div>";
  ptr += "</div>";
  ptr += "<div class='data humidity'>";
  ptr += "<div class='side-by-side icon'>";
  ptr += "<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr += "C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr += "c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr += "C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  ptr += "</div>";
  ptr += "<div class='side-by-side text'>Humidity</div>";
  ptr += "<div class='side-by-side reading'>";
  ptr += (int)humidity;
  ptr += "<span class='superscript'>%</span></div>";
  ptr += "</div>";
  ptr += "<div class='data pressure'>";
  ptr += "<div class='side-by-side icon'>";
  ptr += "<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424";
  ptr += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424";
  ptr += "c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25";
  ptr += "c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414";
  ptr += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804";
  ptr += "c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178";
  ptr += "C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814";
  ptr += "c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05";
  ptr += "C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#26B999 /></g></svg>";
  ptr += "</div>";
  ptr += "<div class='side-by-side text'>Pressure</div>";
  ptr += "<div class='side-by-side reading'>";
  ptr += (int)pressure;
  ptr += "<span class='superscript'>hPa</span></div>";
  ptr += "</div>";
  ptr += "<div class='data altitude'>";
  ptr += "<div class='side-by-side icon'>";
  ptr += "<svg enable-background='new 0 0 58.422 40.639'height=40.639px id=Layer_1 version=1.1 viewBox='0 0 58.422 40.639'width=58.422px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M58.203,37.754l0.007-0.004L42.09,9.935l-0.001,0.001c-0.356-0.543-0.969-0.902-1.667-0.902";
  ptr += "c-0.655,0-1.231,0.32-1.595,0.808l-0.011-0.007l-0.039,0.067c-0.021,0.03-0.035,0.063-0.054,0.094L22.78,37.692l0.008,0.004";
  ptr += "c-0.149,0.28-0.242,0.594-0.242,0.934c0,1.102,0.894,1.995,1.994,1.995v0.015h31.888c1.101,0,1.994-0.893,1.994-1.994";
  ptr += "C58.422,38.323,58.339,38.024,58.203,37.754z'fill=#955BA5 /><path d='M19.704,38.674l-0.013-0.004l13.544-23.522L25.13,1.156l-0.002,0.001C24.671,0.459,23.885,0,22.985,0";
  ptr += "c-0.84,0-1.582,0.41-2.051,1.038l-0.016-0.01L20.87,1.114c-0.025,0.039-0.046,0.082-0.068,0.124L0.299,36.851l0.013,0.004";
  ptr += "C0.117,37.215,0,37.62,0,38.059c0,1.412,1.147,2.565,2.565,2.565v0.015h16.989c-0.091-0.256-0.149-0.526-0.149-0.813";
  ptr += "C19.405,39.407,19.518,39.019,19.704,38.674z'fill=#955BA5 /></g></svg>";
  ptr += "</div>";
  ptr += "<div class='side-by-side text'>Altitude</div>";
  ptr += "<div class='side-by-side reading'>";
  ptr += (int)altitude;
  ptr += "<span class='superscript'>m</span></div>";
  ptr += "</div>";
  ptr += "</div>";
  ptr += "</body>";
  ptr += "</html>";
  return ptr;
}
