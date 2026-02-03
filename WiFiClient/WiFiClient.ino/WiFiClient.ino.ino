#include <WiFi.h>

#define A 13
#define B 18
#define C 19
#define D 27
#define E 14
#define F 17
#define G 16

//Dados do usuario
String NOME;
String EMAIL;
String USER;
const char PASSWORD;

bool AUTENTICADO = false;

//dados da rede
const char* ssid = "SEU_WIFI";
const char* passwordWifi = "SUA_SENHA";

const char* host = "192.168.1.100"; // IP que o Python imprime
const int port = 80;

int button = 21;  
int ledRed = 4;
int buzzer = 11;
int lightSensor = A0;

// Secçao de declaração de métodos
void conectarWifi() {
  WiFi.begin(ssid, passwordWifi);
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
};

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

void capturarDados(String resp) {
  resp.trim();

  // Caso FAIL
  if (resp == "FAIL") {
    AUTENTICADO = false;
    NOME = "";
    EMAIL = "";
    USER = "";
    return;
  }

  // Deve começar com OK|
  if (!resp.startsWith("OK|")) {
    AUTENTICADO = false;
    return;
  }

  int p1 = resp.indexOf('|');        // após OK
  int p2 = resp.indexOf('|', p1 + 1);
  int p3 = resp.indexOf('|', p2 + 1);

  // Garante formato OK|nome|email|user
  if (p1 < 0 || p2 < 0 || p3 < 0) {
    AUTENTICADO = false;
    return;
  }

  NOME  = resp.substring(p1 + 1, p2);
  EMAIL = resp.substring(p2 + 1, p3);
  USER  = resp.substring(p3 + 1);

  AUTENTICADO = true;
}
// ======= Display 7 segmentos (cátodo comum) =======
bool sete_segmentos[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

void escreverNumero(int numero) {
  numero = constrain(numero, 0, 9);
  bool* display = sete_segmentos[numero];
  digitalWrite(A, display[0]);
  digitalWrite(B, display[1]);
  digitalWrite(C, display[2]);
  digitalWrite(D, display[3]);
  digitalWrite(E, display[4]);
  digitalWrite(F, display[5]);
  digitalWrite(G, display[6]);
}

float readLightVoltage() {
  int v = analogRead(lightSensor);
  return v * (5.0 / 1023.0);
}

// ======= Estados =======
enum Estado {
  ST_DESARMADO, 
  ST_ARMANDO,          
  ST_ARMADO,
  ST_DISPARO_INICIO,
  ST_ESPERANDO_SENHA,
  ST_ALARME_FINAL
};

void entrarEstado(Estado novo);

// ======= Configurações =======
const float LIGHT_THRESHOLD_V = 1.5;        
const unsigned long ENTRY_WINDOW_MS = 10000; 
const unsigned long ARM_DELAY_MS = 10000;   
const int MAX_TRIES = 2;

// ======= Variáveis =======
Estado estado = ST_DESARMADO;

const char* SENHA_FIXA = "1234";
String bufferEntrada = "";

int tentativas = 0;
unsigned long inicioJanela = 0;
int lastShown = -1;

// botão (INPUT_PULLUP => pressionado = LOW)
int buttonState = HIGH;
int lastButtonState = HIGH;

// armando
unsigned long armStartMs = 0;

// alarme final: pisca LED
unsigned long lastBlink = 0;
bool ledOn = false;

void limparSerial() {
  while (Serial.available() > 0) Serial.read();
}

void resetEntrada() {
  bufferEntrada = "";
}

bool coletar4DigitosDoSerial() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c >= '0' && c <= '9') {
      bufferEntrada += c;
      Serial.print("*");
      if (bufferEntrada.length() >= 4) return true;
    }
  }
  return false;
}

void bipDuploBloqueante() {
  tone(buzzer, 1000, 150);
  delay(200);
  tone(buzzer, 1000, 150);
  delay(200);
}

void entrarEstado(Estado novo) {
  estado = novo;

  if (estado == ST_DESARMADO) {
    tentativas = 0;
    resetEntrada();
    escreverNumero(0);
    noTone(buzzer);
    digitalWrite(ledRed, LOW);
    limparSerial();

    Serial.println("\nAlarme DESARMADO.");
    Serial.println("Aperte o botao para ARMAR.");
  }

  if (estado == ST_ARMANDO) {
    tentativas = 0;
    resetEntrada();
    noTone(buzzer);
    escreverNumero(0);
    limparSerial();

    armStartMs = millis();
    lastShown = -1;

    Serial.println("\nArmando... Aguarde 10 segundos.");
  }

  if (estado == ST_ARMADO) {
    tentativas = 0;
    resetEntrada();
    escreverNumero(0);
    noTone(buzzer);
    digitalWrite(ledRed, LOW);
    limparSerial();

    Serial.println("\nAlarme ARMADO.");
  }

  if (estado == ST_DISPARO_INICIO) {
    Serial.println("\n*** DISPARO! ***");
    Serial.println("Digite a senha em 10 segundos!");
    Serial.print("Senha: ");

    bipDuploBloqueante();

    tentativas = 0;
    resetEntrada();
    inicioJanela = millis();
    lastShown = -1;

    entrarEstado(ST_ESPERANDO_SENHA);
  }

  if (estado == ST_ALARME_FINAL) {
    Serial.println("\nSenha incorreta/tempo expirou 2x. ALARME FINAL!");
    Serial.println("Digite a senha para desarmar.");
    Serial.print("Senha: ");

    tone(buzzer, 1200); // sirene contínua
    lastBlink = 0;
    ledOn = false;
    digitalWrite(ledRed, LOW);
    escreverNumero(0);
    resetEntrada();
    limparSerial();
  }
}

void setup() {
    Serial.begin(115200);
    conectarWifi();

    Serial.println(httpGET("/status"));

    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    pinMode(C, OUTPUT);
    pinMode(D, OUTPUT);
    pinMode(E, OUTPUT);
    pinMode(F, OUTPUT);
    pinMode(G, OUTPUT);

    pinMode(button, INPUT); // pressionado = LOW
    pinMode(ledRed, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(lightSensor, INPUT);

    buttonState = digitalRead(button);
    lastButtonState = buttonState;

    escreverNumero(0);
    digitalWrite(ledRed, LOW);

    Serial.println("Sistema iniciado.");
    entrarEstado(ST_DESARMADO);
}


void loop() {
    buttonState = digitalRead(button);
    bool pressedEdge = (buttonState == LOW && lastButtonState == HIGH);
    lastButtonState = buttonState;

  switch (estado) {

    case ST_DESARMADO: {
      if (pressedEdge) {
        entrarEstado(ST_ARMANDO);
      }
      break;
    }

    case ST_ARMANDO: {
      unsigned long now = millis();
      unsigned long elapsed = now - armStartMs;

      if ((now / 250) % 2 == 0) digitalWrite(ledRed, HIGH);
      else digitalWrite(ledRed, LOW);

      int remaining = 10 - (int)(elapsed / 1000);
      if (remaining < 0) remaining = 0;
      int showDigit = (remaining == 10) ? 9 : remaining;

      if (showDigit != lastShown) {
        lastShown = showDigit;
        escreverNumero(showDigit);
      }

      // ignora LDR aqui por regra

      if (elapsed >= ARM_DELAY_MS) {
        digitalWrite(ledRed, LOW);
        entrarEstado(ST_ARMADO);
      }
      break;
    }

    case ST_ARMADO: {
      float v = readLightVoltage();

      if (v > LIGHT_THRESHOLD_V) {
        entrarEstado(ST_DISPARO_INICIO);
      }
      break;
    }

    case ST_ESPERANDO_SENHA: {
      unsigned long now = millis();
      unsigned long elapsed = now - inicioJanela;

      int remaining = 10 - (int)(elapsed / 1000);
      if (remaining < 0) remaining = 0;
      int showDigit = (remaining == 10) ? 9 : remaining;

      if (showDigit != lastShown) {
        lastShown = showDigit;
        escreverNumero(showDigit);
      }

      if ((now / 250) % 2 == 0) digitalWrite(ledRed, HIGH);
      else digitalWrite(ledRed, LOW);

      if (coletar4DigitosDoSerial()) {
        Serial.println();
        String tentativa = bufferEntrada.substring(0, 4);
        
        //TODO INCLUIR UMA ESPERA AQUI
        String resp = httpPOST("/autentication", tentativa);

        capturarDados(resp);
        //TODO alterar para esperar a resposta da requisição
        if (AUTENTICADO) {
          Serial.println("Senha correta!");
          entrarEstado(ST_DESARMADO);
          
        } else {
          Serial.println("Senha incorreta!");
          tentativas++;

          resetEntrada();
          limparSerial();

          if (tentativas >= MAX_TRIES) {
            entrarEstado(ST_ALARME_FINAL);
          } else {
            Serial.println("Tente novamente. Voce tem mais 10 segundos.");
            Serial.print("Senha: ");
            inicioJanela = millis();
            lastShown = -1;
          }
        }
      }

      if (elapsed >= ENTRY_WINDOW_MS) {
        tentativas++;
        resetEntrada();
        limparSerial();

        Serial.println("\nTempo expirou!");

        if (tentativas >= MAX_TRIES) {
          entrarEstado(ST_ALARME_FINAL);
        } else {
          Serial.println("Nova tentativa. Digite a senha em 10 segundos.");
          Serial.print("Senha: ");
          inicioJanela = millis();
          lastShown = -1;
        }
      }

      break;
    }

    case ST_ALARME_FINAL: {
      unsigned long now = millis();

      if (now - lastBlink >= 300) {
        lastBlink = now;
        ledOn = !ledOn;
        digitalWrite(ledRed, ledOn ? HIGH : LOW);
      }

      if (coletar4DigitosDoSerial()) {
        Serial.println();
        String tentativa = bufferEntrada.substring(0, 4);
        resetEntrada();
        limparSerial();

        if (tentativa == String(SENHA_FIXA)) {
          Serial.println("Senha correta. Desarmando alarme final.");
          noTone(buzzer);
          entrarEstado(ST_DESARMADO);
        } else {
          Serial.println("Senha incorreta. Alarme continua.");
          Serial.print("Senha: ");
        }
      }

      break;
    }

    case ST_DISPARO_INICIO:
    default:
      // ST_DISPARO_INICIO sempre redireciona dentro de entrarEstado()
      break;
  }
}