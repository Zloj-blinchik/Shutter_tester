/*
(target time) = (timer resolution) * (# timer counts + 1)

(# timer counts + 1) = (target time) / (timer resolution)
(# timer counts + 1) = (1/50 000) / (1/1 000 000)
(# timer counts + 1) = 10
(# timer counts) = 20 - 1 = 19
*/
#define CTC_VAL 19

#define TRASHOLD_PERCENT 0.8

#include <avr/io.h>
#include <avr/interrupt.h>

#define BUFF 4
volatile uint8_t val[BUFF];
volatile uint8_t value;
volatile uint16_t curr_calib_value = 0;
volatile uint16_t prev_calib_value = 0;
volatile uint16_t curr_top = 0;
volatile uint8_t flag = 0; //флаг активности импульса
volatile uint8_t flag_output = 0; //флаг готовности результата 
volatile uint16_t trashold; //граница активного уровня импульса
volatile uint16_t counter_start = 0;

volatile uint8_t flag_tcv = 0;
volatile uint8_t flag_adcc = 0;

volatile uint16_t output;

void setup()
{

  cli();                  // отключить глобальные прерывания
    
    // инициализация Timer1

    TCCR1A = 0;
    TCCR1B = 0;

    TCCR1B |= (1 << CS10);  // Установить CS10 бит так, чтобы таймер работал при тактовой частоте:

   
  //Инициализация ADC A0
  pinMode(A0, INPUT);
  
  ADCSRA = 0;             // Сбрасываем регистр ADCSRA
  ADCSRB = 0;             // Сбрасываем регистр ADCSRB
  ADMUX |= (1 << REFS0);  // Задаем ИОН
  
  ADMUX |= (1 << ADLAR);  // Меняем порядок записи бит, чтобы можно было читать только 8 бит регистра ADCH
                          // Таким образом отсекаются 2 последних "шумных" бита, результат 8-битный (0...255)
                          
  ADMUX |= (0 & 0x07);    // Выбираем пин A0 для преобразования
  
  // Устанавливаем предделитель - 8 (ADPS[2:0]=011)
  ADCSRA |= (1 << ADPS2);
  ADCSRA &= ~((1 << ADPS1) | (1 << ADPS0));

  ADCSRA |= (1 << ADATE); // Включаем автоматическое преобразование
  ADCSRA |= (1 << ADIE);  // Разрешаем прерывания по завершении преобразования
  ADCSRA |= (1 << ADEN);  // Включаем АЦП
  ADCSRA |= (1 << ADSC);  // Запускаем преобразование
  
    
    

    Serial.begin(115200);

    Serial.println("Initialisation done.");

    sei();                  // включить глобальные прерывания */
}

void loop(){
  while(1){
    //Serial.print(TCNT1);
    //Serial.print(' ');
    Serial.print(ADCH);
    Serial.println(' ');
    /*if(flag_output == 1){
      Serial.println(output);
      flag_output = 0;
    }*/
  }
}



ISR(ADC_vect){
  
  cli();
  
  value = ADCH;  // Считываем только значимые 8 бит - значение из диапазона 0...255
  int16_t sum = 0;
  int8_t i;
  for(i = 1; i < BUFF; i++){
    sum+=val[i];
    val[i-1] = val[i];
  }
  val[BUFF-1] = value;
  sum+=value;
  
  curr_calib_value = sum >> 2;

  if(flag == 0){
     if(curr_calib_value > (prev_calib_value<<2)){
        flag = 1;
        counter_start = TCNT1;
        trashold = (curr_calib_value>>1)+(curr_calib_value>>2); 
     }
  }

  if(flag == 1){
     if(curr_calib_value < trashold){
        flag = 0;
        flag_output = 1;
        if(TCNT1<counter_start){
          (0xFFFF - counter_start)+TCNT1;
        }else{
          output = TCNT1 - counter_start;
        }
     }
    
  }

  prev_calib_value = curr_calib_value;
  
  sei();
  
}
