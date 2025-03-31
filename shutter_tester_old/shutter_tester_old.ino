#define BUFF 16
uint16_t val[BUFF];
uint16_t value;
uint8_t counter = 0;
unsigned long old_mil = 0;
unsigned long mil = 0;
uint8_t zero = 0;

void setup() {
  pinMode(A0,INPUT);
  Serial.begin(115200);
  Serial.print("zero");
  Serial.print(' ');
  Serial.println("calibrated");
}

void loop() {
  
  int32_t sum = 0;
  for(int8_t i = 1; i < BUFF; i++){
    val[i-1] = val[i];
    sum+=val[i];
  }
  value = analogRead(A0);
  val[BUFF-1] = value;
  mil = millis();
  sum = sum >> 4;

  if(old_mil != mil){
    
    double calibrated = sum*0.8 + (value - sum)*0.2;
    Serial.print(zero);
    Serial.print(' ');
    Serial.print(value);
    Serial.print(' ');
    Serial.println(calibrated);

  }
  old_mil = mil;
 
}
