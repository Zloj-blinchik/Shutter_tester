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
volatile uint16_t sum = 0;
volatile uint16_t curr_calib_value = 0;
volatile uint16_t prev_calib_value = 0;
volatile uint16_t curr_top = 0;
volatile uint8_t flag = 0; //флаг активности импульса
volatile uint8_t flag_output = 0; //флаг готовности результата 
volatile uint16_t trashold; //граница активного уровня импульса
volatile uint16_t counter = 0;

volatile uint8_t flag_tcv = 0;
volatile uint8_t flag_adcc = 0;

void setup()
{

  cli();                  // отключить глобальные прерывания
    
    // инициализация Timer1

    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = CTC_VAL;        // установка регистра совпадения
    TCCR1B |= (1 << WGM12); // включение в CTC режим
    TCCR1B |= (1 << CS10);  // Установить CS10 бит так, чтобы таймер работал при тактовой частоте:

    TIMSK1 |= (1 << OCIE1A);  // включение прерываний по совпадению
    
  //Инициализация ADC A0
  pinMode(A0, INPUT);
  
  ADCSRA = 0;             // Сбрасываем регистр ADCSRA
  ADCSRB = 0;             // Сбрасываем регистр ADCSRB
  ADMUX |= (1 << REFS0);  // Задаем ИОН
  
  ADMUX |= (1 << ADLAR);  // Меняем порядок записи бит, чтобы можно было читать только 8 бит регистра ADCH
                          // Таким образом отсекаются 2 последних "шумных" бита, результат 8-битный (0...255)
                          
  ADMUX |= (0 & 0x07);    // Выбираем пин A0 для преобразования
  
  // Устанавливаем предделитель - 8 (ADPS[2:0]=011)
  ADCSRA &= ~(1 << ADPS2);
  ADCSRA |= ((1 << ADPS1) | (1 << ADPS0));

  ADCSRA |= (1 << ADATE); // Включаем автоматическое преобразование
  ADCSRA |= (1 << ADIE);  // Разрешаем прерывания по завершении преобразования
  ADCSRA |= (1 << ADEN);  // Включаем АЦП
  ADCSRA |= (1 << ADSC);  // Запускаем преобразование
  
    
    sei();                  // включить глобальные прерывания

    Serial.begin(115200);

    Serial.println("Initialisation done.");
}

void loop(){
  Serial.print(value);
  Serial.print(' ');
  Serial.print(ADCH);
  Serial.println(' ');
  if(flag_output == 1){
    Serial.println(counter);
    flag_output = 0;
  }

}

ISR(TIMER1_COMPA_vect)
{
  flag_tcv = 1;
     curr_calib_value = sum >> 2;
     
     if(flag == 0){
       if(curr_calib_value > 4*prev_calib_value){
          flag = 1;
          counter = 1;
          trashold = TRASHOLD_PERCENT * curr_calib_value; 
       }
     }

     if(flag == 1){
       if(curr_calib_value < trashold){
          flag = 0;
          flag_output = 1;
       }else{
          counter++;
       }
     }
     
     prev_calib_value = curr_calib_value;

     
}

ISR(ADC_vect){
  value = ADCH;  // Считываем только значимые 8 бит - значение из диапазона 0...255
  //cli();
  for(int8_t i = 1; i < BUFF; i++){
    val[i-1] = val[i];
    sum+=val[i];
  }

  val[BUFF-1] = value;
  sum+= value;
  //sei();
}
