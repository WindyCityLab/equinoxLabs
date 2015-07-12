void setup() {
  Serial.begin(115200);
  
}

void loop() {
  Serial.print(analogRead(A0));
  Serial.print("-");
  Serial.println(analogRead(A1));
  delay(100);
}
