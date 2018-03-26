int oep3 = 5;
int led = 13;


void setup() {
  pinMode(led, OUTPUT);
  pinMode(oep3, OUTPUT);
  pinMode (27, OUTPUT);
  digitalWrite (27, HIGH);



  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);


  

  Serial3.setRX(7);
  Serial3.setTX(8);
  
  digitalWrite(oep3, HIGH);
  Serial3.begin(153600);
}

void loop() {
  while (Serial.available() > 0) {
    Serial3.write(Serial.read());
    digitalWrite(led, HIGH);
  }
  digitalWrite(led, LOW);
}
