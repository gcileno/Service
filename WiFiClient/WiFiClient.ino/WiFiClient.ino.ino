#include <WiFi.h>

const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

const char* host = "192.168.1.100"; // IP que o Python imprime
const int port = 80;

// Secçao de declaração de métodos
void conectarWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
}

String httpGET(const char* rota) {
  WiFiClient client;

  if (!client.connect(host, port)) {
    return "Erro de conexão";
  }

  client.print(String("GET ") + rota + " HTTP/1.1\r\n");
  client.print(String("Host: ") + host + "\r\n");
  client.print("Connection: close\r\n\r\n");

  String resposta = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      resposta += client.readStringUntil('\n');
    }
  }

  client.stop();
  return resposta;
}


String httpPOST(const char* rota, String body) {
  WiFiClient client;

  if (!client.connect(host, port)) {
    return "Erro de conexão";
  }

  client.print(String("POST ") + rota + " HTTP/1.1\r\n");
  client.print(String("Host: ") + host + "\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Length: ");
  client.print(body.length());
  client.print("\r\n\r\n");
  client.print(body);  // body PURO (on, off, senha)

  String resposta = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      resposta += client.readStringUntil('\n');
    }
  }

  client.stop();
  return resposta;
}


void setup() {
  Serial.begin(115200);
  conectarWifi();

  Serial.println(httpGET("/status"));
}

void loop() {
  Serial.println(httpPOST("/led", "on"));
  delay(3000);

  Serial.println(httpPOST("/led", "off"));
  delay(3000);

  Serial.println(httpPOST("/autentication", "123456"));
  delay(5000);
}
