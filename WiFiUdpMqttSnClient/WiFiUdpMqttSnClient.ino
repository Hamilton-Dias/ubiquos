/*
* The MIT License (MIT)
*
* Copyright (C) 2018 Gabriel Nikol
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "WiFiUdpSocket.h"
#include "MqttSnClient.h"

//Credenciais da Wifi Utilizadas
const char* ssid     = "Almerinda Repetidor";
const char* password = "hamilton";

//Contadores para as mensagens
int mensagem_enviada = 0;
int mensagem_recebida = 0;


//Tem que mudar o buffer_length para alterar o tamanho do payload
//depois no código, só será preenchido com zeros
#define buffer_length 10
char buffer[buffer_length + 1];

//Endereço e porta do Gateway (Olhar o endereço e porta do PAHO GATEWAY)
IPAddress gatewayIPAddress(192, 168, 0, 106);
uint16_t localUdpPort = 10000;

//Cria o objeto e sockets de wifi com comunicação UDP
WiFiUDP udp;
WiFiUdpSocket wiFiUdpSocket(udp, localUdpPort);
MqttSnClient<WiFiUdpSocket> mqttSnClient(wiFiUdpSocket);

//Cliente ID (pode ser qualquer um)
const char* clientId = "hamilton1";

//Nome do tópico para o cliente se inscrever e receber as mensagens
char* subscribeTopicName = "ESP8266/123";

//Nome do tópico para publicar mensagens
char* publishTopicName = "ESP8266/123";

//Quality of Service, deixe zero, assim nós não esperamos receber nenhum ACK
int8_t qos = 0;

//O callback é executado toda vez que o cliente recebe uma mensagem do broker
void mqttsn_callback(char *topic, uint8_t *payload, uint16_t length, bool retain) {
  //Isso garante que só será executado quando não for um ACK
  //pois ACKs tem o "topic" igual a NULL
  if (topic != NULL){
    //Funções que printam as mensagens recebidas
    //Foi as principais funções para calculo de vazão e latência

    
    //Serial.print("Mensagem recebida: ");
    //Serial.print(mensagem_recebida);
    //Serial.print("Tempo Recebido msg: ");
    //Serial.println(millis());

    mensagem_recebida = mensagem_recebida + 1;
  }
}

void setup() {

  //Conexao com a Wifi
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Em média, o esp demora 4420 milissegundos para se conectar à wifi
  Serial.print("Tempo para conectar Wifi: ");
  Serial.println(millis());

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());

  //Serial.print("Starting MqttSnClient - ");
  mqttSnClient.setCallback(mqttsn_callback);
  if  (!mqttSnClient.begin()) {
    //Serial.print("Could not initialize MQTT-SN Client ");
    while (true) {
      //Serial.println(".");
      delay(1000);
    }
  }
  //Serial.println(" ready!");

  //Tempo para iniciar o cliente é sempre muuuuito baixo
  
  //Serial.print("Tempo para iniciar o cliente: ");
  //Serial.println(millis());
}

//Essa função é padrão, pra converter o IP e passar pra biblioteca, não precisa mexer nela
void convertIPAddressAndPortToDeviceAddress(IPAddress& source, uint16_t port, device_address& target) {
  // IPAdress 0 - 3 bytes
  target.bytes[0] = source[0];
  target.bytes[1] = source[1];
  target.bytes[2] = source[2];
  target.bytes[3] = source[3];
  // Port 4 - 5 bytes
  target.bytes[4] = port >> 8;
  target.bytes[5] = (uint8_t) port ;
}


void loop() {
  //Tenta a conexão com o Gateway
  if (!mqttSnClient.is_mqttsn_connected()) {
#if defined(gatewayHostAddress)
    IPAddress gatewayIPAddress;
    if (!WiFi.hostByName(gatewayHostAddress, gatewayIPAddress, 20000)) {
      Serial.println("Could not lookup MQTT-SN Gateway.");
      return;
    }
#endif
    device_address gateway_device_address;
    convertIPAddressAndPortToDeviceAddress(gatewayIPAddress, localUdpPort, gateway_device_address);
    //Serial.print("MQTT-SN Gateway device_address: ");
    printDeviceAddress(&gateway_device_address);


    if (!mqttSnClient.connect(&gateway_device_address, clientId, 180) ) {
      Serial.println("Could not connect MQTT-SN Client.");
      delay(1000);
      return;
    }
    //Serial.println("MQTT-SN Client connected.");
    //Serial.print("Tempo para conectar ao Broker: ");
    //Serial.println(millis());
    //mqttSnClient.set_mqttsn_connected();


    //É preciso se inscrever no tópico que será enviado mensagens para poder entrar na função de callback
    if (!mqttSnClient.subscribe(subscribeTopicName, qos)){
      Serial.println("Cant subscribe");
    }
    //Serial.println("Subscribed");
    //Serial.print("Tempo para se inscrever: ");
    //Serial.println(millis());
  }

  //Funções que printam as mensagens recebidas
  //Foi as principais funções para calculo de vazão e latência
  
  //Serial.print("Mensagem enviada: ");
  //Serial.print(mensagem_enviada);
  //Serial.print("Tempo para enviar msg: ");
  //Serial.println(millis());


  //Aqui é um teste de perda de pacotes, onde será enviado mensagens duante 10min, depois, printa o resultado
  //Se nao der 10min
  if (millis() < 600000){
      memset(buffer, 0x0, buffer_length);
      mqttSnClient.publish(buffer, publishTopicName , qos);

      mensagem_enviada = mensagem_enviada + 1;
  }
  else{
    Serial.print("Foi enviado ");
    Serial.print(mensagem_enviada);
    Serial.print(" mensagens, e foi recebido ");
    Serial.print(mensagem_recebida);
    Serial.println(" mensagens");
    delay(500000);
  }

  //TEM QUE SEMPRE TER O MQTTSNCLIENTE.LOOP!!!!!!!!! IMPORTANTE
  mqttSnClient.loop();

  //Se não der esse delay de 20ms o cliente é desconectado do Broker.
  //Tem que pesquisar o motivo (será que é proteção anti-spam???)
  delay(20);
}
