#include <LiquidCrystal.h>
#include <avr/interrupt.h>
#include <SoftwareSerial.h>

SoftwareSerial comm (6, 7); // rx,tx
bool led_state = false;
int b = 0;
#define LM35_PIN  0
#define MOTION_PIN PD2
#define LED_PIN 1
#define F_CPU 16000000UL

bool motionReading = 0;
const int rs = 12, en = 11, d4 = 10, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
volatile uint32_t milliseconds = 0;
const long interval = 2000;
float temperature = 0.0;
float smokeReading = 0.0;


void setup() {

  DDRD &= ~(1 << MOTION_PIN );
  PORTD |= (1 << 2 ); 
  DDRB |= (1 << LED_PIN);
  lcd.begin(16, 2);
  Serial.begin(115200);

  INIT_ADC();
  EIMSK = (1 << INT0); // to generate INT0 (PD2)
  EICRA |= (1 << ISC00);//Any logical change on INT0 generates the interrupt request

  TCCR1A = 0;//Normal mode
  TCCR1B = (1 << WGM12) | (1 << CS12) ; //clk / 256 , ctc mode
  //OCR1AL = 0x24; // 1 s -> T = 1/(16M/256)= 16us , counts = 1sec/16us = 62500
  OCR1A = 0xF424;
  TIMSK1 = (1 << OCIE1A);
  sei();
  comm.begin(115200);
  wifi_init();
  Serial.println("System Ready..");

}


void loop() {

  if (milliseconds >= 2000) {
    temperature = Analog_Read(0);
    temperature = (temperature * 5000 / 1024.0 ) / 10;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Tmp:");
    lcd.print(temperature);

    lcd.setCursor(10, 0);
    lcd.print("Mo: ");
    lcd.print(motionReading);
    smokeReading = Analog_Read(1);
    lcd.setCursor(0, 1);
    lcd.print("Gas: ");
    lcd.print(smokeReading);
    milliseconds = 0;
    //motionReading= 0; 
    Serial.println("hi sensor");
  }
  b = 0;
  // Serial.println("Refresh Page");
  while (b < 1000)
  {
    b++;
    while (comm.available())
    {
      String s = comm.readString();
      Serial.println(s);
      if (s.indexOf("+IPD,") != -1 || s.indexOf("+HPD,") != -1)
      {
        if (s.indexOf("LEDON") != -1)
        {
          PORTB |= (1 << 1);
          led_state = 1;
        }
        else if (s.indexOf("LEDOFF") != -1 || s.indexOf("LEDOGF") != -1)
        {
          PORTB &=  ~(1 << 1);
          led_state = 0;
        }
        Serial.println("Starting");
        int id = 0;
        if (s.indexOf("+IPD,") != -1)
          id = s.charAt(s.indexOf("+IPD,") + 5) - '0';
        else
          id = s.charAt(s.indexOf("+HPD,") + 5) - '0';

        sendToServer(String(id));
        Serial.println("Finished");
        int x = milliseconds;
        while (milliseconds - x <= 1000);
        break;
      }
    }
  }


}
void INIT_ADC()
{
  ADMUX |= (1 << REFS0);
   
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);

}

int Analog_Read(uint8_t Sensor_PIN)
{
  ADMUX |= Sensor_PIN ;
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADIF) == 0);

  return ADC;
}

ISR(TIMER1_COMPA_vect) {
  milliseconds += 1000;
}

ISR(INT0_vect) {
   motionReading=  (PIND & (1 << MOTION_PIN));
}



void establishConnection(String command, int timeOut) //Define the process for sending AT commands to module
{
  int q = 0;
  int y = milliseconds;
  while (q < 5)
  {

    Serial.println(command);
    comm.println(command);
    while (comm.available())
    {
      if (comm.find("OK"))
        q = 8;
    }
    while (milliseconds - y <= timeOut);
    y = milliseconds;
    if (q == 8)
      break;
    q++;
  }
  if (q == 8)
    Serial.println("OK");
  else
    Serial.println("Error");
}

void wifi_init() //send AT commands to module
{
  establishConnection("AT", 1000);
  establishConnection("AT+RST", 1000);
  establishConnection("AT+CWMODE=3", 1000);
  
  Serial.println("Connecting Wifi....");
  establishConnection("AT+CWJAP=\"ZeyadAtef\",\"1234567890\"", 5000); //provide your WiFi username and password here
  Serial.println("Wifi Connected");
  establishConnection("AT+CIPMUX=1", 1000);
  establishConnection("AT+CIPSERVER=1,80", 1000);
}

void sendData(String x , String server1) //send data to module
{
  int p = 0;
  int c = 0;
  unsigned int l = server1.length();
  int y = milliseconds;

  int flag = 0;
  while (c < 5)
  {
    if (flag == 0)
    {
      comm.print("AT+CIPSEND=");
      comm.print(x);
      comm.print(",");
      comm.println(l);
      Serial.print("AT+CIPSEND=");
      Serial.print(x);
      Serial.print(",");
      Serial.println(l);
    }
    flag = 1;
    if (milliseconds - y >= 2000)
    {
      y = milliseconds;
      c++;
      flag = 0;
      comm.print(server1);
      while (comm.available())
      {
        if (comm.find("OK"))
        {
          p = 11;
          break;
        }
      }
    }

    if (p == 11)
      break;
  }
}

void sendToServer(String x)//send data to webpage
{
  sendData(x, SendHTML(led_state));
  int y = milliseconds;
  while (milliseconds - y <= 300);
  comm.print("AT+CIPCLOSE=");
  comm.println(x);
  Serial.println("AT+CIPCLOSE=" + x);
}


String SendHTML(bool state) {
  String ptr = "";
  ptr += "<!DOCTYPE html><html><head><title>LED Control</title></head><body style =\"display: flex; justify-content: center; align-items: center;flex-direction: column;\"><h1>Server</h1>";

 ptr += (state) ? "<h3>LED Status: ON</h3><a href=\"http://192.168.4.1/LEDOFF\"><button style=\"border-radius: 15px; width: 130px; height: 50px; font-size: 20px;\" ><b>OFF</b></button></a>" :
           "<h3>LED Status: OFF</h3><a href=\"http://192.168.4.1/LEDON\"><button style=\"border-radius: 15px; width: 130px; height: 50px; font-size: 20px;\" ><b>ON</b></button></a>";

  
  if (smokeReading > 210 )
    ptr += "<p>Warning! high Gas</p>" ;
  if(temperature > 70.0)
    ptr += "<p>Warning! high Temp</p>" ;
    
  

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
