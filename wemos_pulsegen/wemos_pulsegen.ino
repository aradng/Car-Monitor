void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(D0, OUTPUT);
  pinMode(D5, OUTPUT);
  digitalWrite(D0, HIGH);
  digitalWrite(D5, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(A0);
  val*= 4;
  analogWrite(D2, 512);
  analogWriteFreq(val);
  Serial.println(val);
  delay(300);
}
