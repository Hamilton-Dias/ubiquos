#include <ESP8266WiFi.h>
#include "coap_server.h"

//Cabecalho Callback do servidor CoAP
void callback_light(coapPacket &packet, IPAddress ip, int port, int obs);

//Cria instancia de servidor CoAP
coapServer coap;

//Parametros para conexao WiFi
const char* ssid = "Almerinda Repetidor";
const char* password = "hamilton";

bool LEDSTATE;

//URLs Endpoints do servidor CoAP
void callback_light(coapPacket *packet, IPAddress ip, int port, int obs) {
  if (port){
    //Serial.println("Light");
    
    char p[packet->payloadlen + 1];
    memcpy(p, packet->payload, packet->payloadlen);
    p[packet->payloadlen] = NULL;

    String message(p);
    Serial.println(p);

    //Teste de loops usando funcao POST
    if (message.equals("0")){
      digitalWrite(16,LOW);
      Serial.println("Estou no IF");
    }
    else if (message.equals("1")){
      digitalWrite(16,HIGH);
      Serial.println("Estou no ELSE");
    }

    char *light = (digitalRead(16) > 0)? ((char *) "1") :((char *) "0");

    if (obs == 1)
    coap.sendResponse(light);
    else
    coap.sendResponse(ip, port, light);
  }
}

void callback_lightled(coapPacket *packet, IPAddress ip, int port, int obs) {
  if (port){
    Serial.println("LightLed");

    char p[packet->payloadlen + 1];
    memcpy(p, packet->payload, packet->payloadlen);
    p[packet->payloadlen] = NULL;

    String message(p);

    if (message.equals("1"))
      LEDSTATE = false;
    else if (message.equals("0"))
      LEDSTATE = true;

    //Alterando o estado do LED usando metodo POST
    if (LEDSTATE) {
      digitalWrite(2, HIGH);
      if(obs==1)
      coap.sendResponse("1");
      else
      coap.sendResponse(ip, port, "1");
    } else {
      digitalWrite(2, LOW);
      if (obs==1)
      coap.sendResponse("0");
      else
      coap.sendResponse(ip, port, "0");
    }
  }
}

//Funcao utilizada como teste para metodo de descoberta CoAP. Uma simples copia da funcao lightled.
void callback_vazao(coapPacket *packet, IPAddress ip, int port, int obs){
  if (port){
    Serial.println("Vazao");

    char p[packet->payloadlen + 1];
    memcpy(p, packet->payload, packet->payloadlen);
    p[packet->payloadlen] = NULL;

    String message(p);


    //Aqui tem bastante coisa, mas o que só importa é o coap.sendResponse()
    //pois é ela que envia a resposta para o cliente, acionando a callback do cliente!!
    
    if (message.equals("0"))
      Serial.println("if loop");
    else if (message.equals("1"))
      Serial.println("else loop");

    if (LEDSTATE) {
      digitalWrite(3, HIGH);
      if (obs==1)
      coap.sendResponse("1");
      else
      coap.sendResponse(ip, port, "1");
    } else {
      digitalWrite(3, LOW);
      if (obs==1)
      coap.sendResponse("0");
      else
      coap.sendResponse(ip, port, "0");
    }
  }
}

void setup() {
  yield();
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  //Conectando a rede WiFi
  Serial.print("");
  Serial.print("Conectando a rede ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //yield();
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado");
  
  //Mostra o IP do servidor CoAP no monitor serial
  Serial.println(WiFi.localIP());

  //Aqui são testes do Patrick
  //SetUp de pinos e estados
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  
  LEDSTATE = false;
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);  

  //URLs Endpoints. Se criar uma nova funcao, adicione aqui
  coap.server(callback_light, "light");
  coap.server(callback_lightled, "lightled");
  coap.server(callback_vazao, "vazao");

  //Inicia o servidor CoAP na porta 9000
  coap.start(9000);
}

void loop() {
  //SEMPRE CHAMAR LOOP
  coap.loop();
  delay(0);
}
