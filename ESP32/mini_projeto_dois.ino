const int touchPin = 4;
const int ledLight = RX;
void setup() {
  // put your setup code here, to run once:
  pinMode(ledLight, OUTPUT);
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  int touch = touchRead(touchPin);
  digitalWrite(ledLight, LOW);

  if (touch <= 500){
    digitalWrite(ledLight, HIGH);
  }
  Serial.println(touch);
  delay(1000);
}
