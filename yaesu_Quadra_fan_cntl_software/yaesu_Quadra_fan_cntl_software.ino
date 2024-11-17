
//*************************************************************************************************
//This is a simple temperature conrolled fan controller for Yaesu VL1000 an other pa´s.
//The program reading two I2C temperature sensors, like DS18B20.
//It must no set I2C adress in a register, read_sens setting this automaticly.
//Will you read data with UART, must set the debug = 1, uart is set to 9600 baud.
//When it have a read error of temperature sensors or over/under temperature, 
//is set error = 1 and overtemp output to high, fan´s are running full speed 
//The overtemp output can  connect to overtemp connector from VL-1000,
//this will set the PA in error mode.
//73´DJ1YR(René)
//*************************************************************************************************

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  ds_c(19);                    //DS18B20 Tuner Unit
OneWire  ds_ab(18);                   //DS18B20 RF Block

unsigned long previousMillis_temp = 0; 
const long interval_temp = 500;           //read intervall of temp sensors in ms 

const int fan_ab_speed = 2;               //on pin 32
const int fan_c_speed = 3;                //on pin 1
const int overtemp = 4;                   //on pin 4
const int reserve_0 = 8;                  //on pin 12
const int LED_1 = 20;                     //on pin 7
const int LED_2 = 21;                     //on pin 8
int LED_1_state = LOW;                    //debug led for "life pulse"
int LED_2_state = LOW;                    //debug led

const int fan_ab = 10;
int fan_ab_pwm;
int fan_ab_ptt_min = 20;                  //<---- minimal pwm for pa fan on vl-1000 when tx is enabled
const int fan_c = 9;
int fan_c_pwm;
int fan_c_ptt_min = 10;                   //<---- minimal pwm for atu fan on vl-1000 when tx is enabled

const int tx_in = 5;                      

bool tx_stat = 0;
bool ok = 0;
bool debug = 0;                           //<-------set this for debuging
bool error = 0;

//******* temperature sensor RF BLOCK ************
byte i_ab;
byte present_ab = 0;
byte type_s_ab;
byte data_ab[9];
byte addr_ab[8];
float temp_ab;

//******* temperature sensor Tuner ***************
byte i_c;
byte present_c = 0;
byte type_s_c;
byte data_c[9];
byte addr_c[8];
float temp_c;

//********** temperature limits **************
int temp_min_ab = 15;
int temp_max_ab = 45;
int temp_min_c = 15;
int temp_max_c = 45;

void setup() {
  Serial.begin(9600);
  
  pinMode(fan_ab_speed, INPUT);
  pinMode(fan_c_speed, INPUT);
  pinMode(overtemp, OUTPUT);
  pinMode(reserve_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(fan_ab, OUTPUT);
  pinMode(fan_c, OUTPUT);
  pinMode(tx_in,INPUT_PULLUP);
}

void loop() {
  
tx_stat = digitalRead(tx_in);
//*******PA_Start********** 
 
  
  if (debug == 0&&ok == 0) {
    fan_ab_pwm = 255;
    fan_c_pwm = 255;    
    analogWrite(fan_ab, fan_ab_pwm);
    analogWrite(fan_c, fan_c_pwm); 
    delay(5000);
    ok = 1;
    fan_ab_pwm = 0;
    fan_c_pwm = 0;    
    analogWrite(fan_ab, fan_ab_pwm);
    analogWrite(fan_c, fan_c_pwm); 
  }

//*************************
  
  if (ok == 1&&error == 0) { 
    digitalWrite(overtemp,LOW);  
    unsigned long currentMillis_temp = millis();                    //Temperatur nur jede n Sekunde auslesen, wenn debuging aktiviert ist, kann es zu längeren Intervallen kommen, da ein delay von 1000ms hinzukommt
    if (currentMillis_temp - previousMillis_temp >= interval_temp) {
    previousMillis_temp = currentMillis_temp;
    meassure_temp();
    }
    else
    analogWrite(fan_ab_pwm, 255); 
    analogWrite(fan_c_pwm, 255);
    digitalWrite(overtemp,HIGH);
   }

    fan_ab_pwm = map(temp_ab, temp_min_ab, temp_max_ab, 0, 255);
    if (fan_ab_pwm <= 25) {
    fan_ab_pwm = 0;
    }
    
    fan_c_pwm = map(temp_c, temp_min_c, temp_max_c, 0, 255);
    if (fan_c_pwm <= 10) {
    fan_c_pwm = 0;
    }

    if (tx_stat == 0){
        if(fan_ab_pwm <=25){
           fan_ab_pwm = fan_ab_ptt_min;
           }
           
        if(fan_c_pwm <= 10){
           fan_c_pwm = fan_c_ptt_min;
           }
     }
    analogWrite(fan_ab, fan_ab_pwm);
    analogWrite(fan_c, fan_c_pwm); 
   
  if(debug){
    debuging();
  }
}

void meassure_temp() {

  if ( !ds_ab.search(addr_ab)) {
    ds_ab.reset_search();
    return;}

  if ( !ds_c.search(addr_c)) {
     ds_c.reset_search();
     return;}  
  
  if (OneWire::crc8(addr_ab, 7) != addr_ab[7]) {
     error = 1;
     return;}
  if (OneWire::crc8(addr_c, 7) != addr_c[7]) {
     error = 0;
     return;}
  
  ds_ab.reset();
  ds_ab.select(addr_ab);
  ds_ab.write(0x44, 1);                         // start conversion, with parasite power on at the end

  ds_c.reset();
  ds_c.select(addr_c);
  ds_c.write(0x44, 1);                          // start conversion, with parasite power on at the end
  
  present_ab = ds_ab.reset();
  ds_ab.select(addr_ab);    
  ds_ab.write(0xBE);                            // Read Scratchpad

  present_c = ds_c.reset();
  ds_c.select(addr_c);    
  ds_c.write(0xBE);                             // Read Scratchpad

  for ( i_ab = 0; i_ab < 9; i_ab++) {           // we need 9 bytes
    data_ab[i_ab] = ds_ab.read();}

  for ( i_c = 0; i_c < 9; i_c++) {              // we need 9 bytes
    data_c[i_c] = ds_c.read();}  

  int16_t raw_ab = (data_ab[1] << 8) | data_ab[0];
  if (type_s_ab) {
    raw_ab = raw_ab << 3;                       // 9 bit resolution default
    if (data_ab[7] == 0x10) {
      raw_ab = (raw_ab & 0xFFF0) + 12 - data_ab[6];
      }
    }
    
    int16_t raw_c = (data_c[1] << 8) | data_c[0];
  if (type_s_c) {
    raw_c = raw_c << 3;                         // 9 bit resolution default
    if (data_c[7] == 0x10) {
    raw_c = (raw_c & 0xFFF0) + 12 - data_c[6];
    }
  } 
  
  temp_ab = (float)raw_ab / 16.0;
  temp_c = (float)raw_c / 16.0;

  if(temp_ab <= -1 || temp_ab >= 61){
    error = 1;
  }
  else{
    error = 0;
  }
  if(temp_c <= -1 || temp_c >= 60){
    error = 1;
  }
  else{
    error = 0;
  }

  
    if (LED_1_state == LOW) {
      LED_1_state = HIGH;}
      else {
      LED_1_state = LOW;}
      digitalWrite(LED_1, LED_1_state);  
}

void debuging(){ 
   if (ok == 0){
    ok = 1;
    }
  Serial.print("Temp_ab = ");
  Serial.print(temp_ab);
  Serial.print("°C_ab, ");
  Serial.print("PWM_ab = ");
  Serial.print(fan_ab_pwm);
  Serial.println();
  Serial.print("Temp_c = ");
  Serial.print(temp_c);
  Serial.print("°C_c, ");
  Serial.print("PWM_c = ");
  Serial.print(fan_c_pwm);
  Serial.println();
  Serial.println();
  Serial.print("TXin = ");
  Serial.println(tx_stat);
  Serial.println();
  Serial.println();
  Serial.print("Stat = ");
  Serial.println(ok);
  Serial.println();
  Serial.println();
  
  delay(1000);
}

  
