#include <ESP8266WiFi.h>
#include "coap_client.h"

//Cria instancia de cliente coap
coapClient coap;

//Credenciais da Wifi Utilizadas
const char* ssid = "Almerinda Repetidor";
const char* password = "hamilton";

//Contadores para as mensagens
int mensagem_enviada = 0;
int mensagem_recebida = 0;

//IP e porta padrao do servidor CoAP. Esse IP deve ser obtido executando seu servidor CoAP e observado o endereco IP mostrado no monitor serial.
IPAddress ip(192,168,0,104);
int port = 9000;

//Cabecalho da resposta callback do cliente
void callback_response(coapPacket &packet, IPAddress ip, int port);

//Resposta callback
void callback_response(coapPacket &packet, IPAddress ip, int port){

    //Serial.print("Mensagem recebida: ");
    //Serial.print(mensagem_recebida);
    //Serial.print("Tempo Recebido msg: ");
    //Serial.println(millis());

    mensagem_recebida = mensagem_recebida + 1;

}

void setup(){
    Serial.begin(9600);

    WiFi.begin(ssid, password);
    
    //Conectando a Wifi
    Serial.println("");
    Serial.print("Conectando na rede ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        //yield();
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado");
    
    //Mostra o IP do cliente no monitor serial
    Serial.println(WiFi.localIP());

    //Instancia a resposta do cliente
    coap.response(callback_response);

    //Inicia o cliente CoAP na porta 5683
    coap.start(9000);
}


//Variaveis para o teste de vazao
char *payload = "0";

void loop() {

  //sempre chamar o loop! não esquecer
  coap.loop();
    
  //colocando o delay de 20 para deixar justo com o MQTT-SN
  delay(20);

  //Igual MQTT-SN
  //Serial.print("Mensagem enviada: ");
  //Serial.print(mensagem_enviada);
  //Serial.print("Tempo para enviar msg: ");
  //Serial.println(millis());

  //Se nao der 1min
  if (millis() < 600000){
      coap.post(ip, port, "light", payload, strlen(payload));
      
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
    

}
