//Este tiene todo integrado
const char* ssid = "Tec-IoT";
//Yo debo estar con el internet del Tec
const char* password = "spotless.magnetic.bridge";
// URL del servidor (cambia localhost por la IP de tu servidor)
const char* serverName = "https://iotweb-production.up.railway.app/insert_data";


#include "Arduino.h" 
//Temperatura
#include "DHT.h"
//----------------------
//Bocina
#include "DFRobotDFPlayerMini.h"
//-----------------------
//Sistema de archivos
#include "LittleFS.h"
//-----------------------
//Acelerometro
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
Adafruit_MPU6050 mpu;


//---------------------------------


//Conexion a internet
#include <WiFi.h>
#include <HTTPClient.h>
//--------------------------------------------------------------------------------------------------


//Ajustes de calibracion del MPU6050.

float off_x = -1.8;
float off_y = 2.5;
float off_z = -9.94;
//Valor de aceleracion limite.
const int lim_accel = 7;

float promedio_uno_accel;
float valor_temperatura;

//Pin para el reset.
const int reset = 4;
//Numero de minutos usados para 
const int num_minutos = 2;
//Funcion para leer el ultimo estado del medicamento, registrado en la memoria.
char leer(){
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return 'm';
  }
  
  File file = LittleFS.open("/registro.txt", "r");
  if(!file){
    Serial.println("Failed to open file for reading");
    return 'f';
  }
  
  Serial.println("Initial file Content:");
  char retorno;
  while(file.available()){
    //Serial.write(file.read());
    retorno = file.read();
    //Serial.print("retornando: ");
    //Serial.write(retorno);
    //Serial.println("");
  }
  file.close();

  return retorno;
}

  
  void sobreescribir(char estado_nuevo){
    File fileMod = LittleFS.open("/registro.txt","w");
    if (!fileMod){
    Serial.println("Sobreescritura fallida");
    return;
    }
    //char value;
    //Serial.print("El valor actual de inicio es: ");
    //Serial.println(inicio);
    
    fileMod.print(estado_nuevo);
    fileMod.close();

  }



#ifdef ESP32
#define FPSerial Serial1  // For ESP32, use hardware serial port 1
#else
#include <SoftwareSerial.h> // Include SoftwareSerial library for non-ESP32 boards
SoftwareSerial FPSerial(16, 17); // Define SoftwareSerial on pins 16 (RX) and 17 (TX)
#endif
DFRobotDFPlayerMini myDFPlayer;
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

//Variable muy importante para llevar el flujo del programa.
int iteracion;
//



int reiniciar = 0;
const int pinRojo = 23;
const int pinVerde = 19;
const int pinAzul = 18;
const int DHTPin = 5;     // what digital pin we're connected to
//Los limites que defino son incluyentes, es decir que si la temperatura es igual a la de lim_inf, todavia esta bien.
//Debido a la precision del sensor, en la iteracion final se recomienda dejar un alto margen en la temperatura.
const float lim_inf = 25.7;
const float lim_sup = 29;
DHT dht(DHTPin, DHTTYPE);

void ledRGB(int rojo, int verde, int azul){
  analogWrite(pinRojo,rojo);
  analogWrite(pinVerde,verde);
  analogWrite(pinAzul,azul);
  //Naranja: ledRGB(140,150,0);
  

}

char estado = leer();
std::vector<float> accel_segundos;
std::vector<float> accel;
std::vector<float> accel_minutos;
std::vector <float> temperatura;
std::vector <float> temperatura_minutos;


void setup() {

  // Conectar a WiFi
 WiFi.begin(ssid, password);
 //Ciclo en el que se intenta conectar.
 while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println("Connecting to WiFi...");
 }
 Serial.println("Connected to WiFi");



  pinMode(reset,INPUT);
  
  //std::vector<float> accel;
  //sobreescribir('1');
  pinMode(OUTPUT,pinRojo);
  pinMode(OUTPUT,pinVerde);
  pinMode(OUTPUT,pinAzul);
  
  //167 84 20
  #ifdef ESP32
  FPSerial.begin(9600, SERIAL_8N1, 16, 17); // Start serial communication for ESP32 with 9600 baud rate, 8 data bits, no parity, and 1 stop bit
  #else
  FPSerial.begin(9600); // Start serial communication for other boards with 9600 baud rate
  #endif

  Serial.begin(115200);
  Serial.println("DHTxx test!");
  Serial.println(F("DFRobot DFPlayer Mini Demo")); // Print a demo start message
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)")); // Print initialization message
  
  if (!myDFPlayer.begin(FPSerial)) { // Initialize the DFPlayer Mini with the defined serial interface
    Serial.println(F("Unable to begin:")); // If initialization fails, print an error message
    Serial.println(F("1.Please recheck the connection!")); // Suggest rechecking the connection
    Serial.println(F("2.Please insert the SD card!")); // Suggest checking for an inserted SD card
    while(true); // Stay in an infinite loop if initialization fails
  }
  Serial.println(F("DFPlayer Mini online.")); // Print a success message if initialization succeeds
  
  myDFPlayer.volume(20);  // Set the DFPlayer Mini volume (max is 30)
  //Mensaje de inicio
  myDFPlayer.play(4);
  //Sensor de temperatura
  dht.begin();
  //Tiempo para que el mensaje de voz pueda ser reproducido adecuadamente.
  delay(4000);

  //Intentamos iniciar el acelerometro
  if ( !mpu.begin() ){
    Serial.println("Failed to find MPU6050 chip");
  }
  //Inicializamos adecuadamente.
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  //Puede configurarse con 16, 8, 4, 2, obteniendo mayor rango o precision.
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  //No se si se ocupa pero lo pongo por si a caso.
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);
  //Creo que es la frecuencia, debe aumentarse si se esperan sacudidas muy rapidas. Puede ser 260,184,94,44,21,10,5

}


void loop() {
  // Ciclo para sacar el promedio de temperatura cada 5 minutos.
  
    


    //Reading temperature or humidity takes about 250 milliseconds!
    //Demasiado espacio para que puedan hacerlo, literalmente 10 segundos.
    //En caso de que aun sirva el medicamento (estado == '1'), se procede a tomar mediciones.
    if (estado == '1'){
      for(int k = 0; k < num_minutos;k++){
        temperatura.clear();
        if(estado == '0'){
          break;
        }
        // for encargado de que se ejecute 6 veces para formar el promedio de un minuto.
        for(int i = 0; i < 6; i ++){
          accel_segundos.clear();
          if(estado=='0'){
            break;
          }
          
          for(int j = 0; j < 5;j++){
            //Mediciones de la vibracion para el promedio de la misma. Notese que estamos considerando que la vibracion sea igual a la aceleracion total.
            sensors_event_t a, g, temp;
            mpu.getEvent(&a, &g, &temp);
            float x_accel = a.acceleration.x+off_x;
            float y_accel = a.acceleration.y+off_y;
            float z_accel = a.acceleration.z+off_z;
            float accel_total = sqrt((pow(x_accel,2)+pow(y_accel,2)+pow(z_accel,2)));
            Serial.print("Aceleracion muestra: ");
            Serial.println(accel_total);
            //Vector de aceleracion 10 seg.
            accel_segundos.push_back(accel_total);
          }
          promedio_uno_accel = 0;
          
          for(int  a =0; a < accel_segundos.size();a++){
            promedio_uno_accel += accel_segundos[a];
          }

          if(promedio_uno_accel != 0){
            promedio_uno_accel/= accel_segundos.size();
            if(promedio_uno_accel > lim_accel){
              estado = '0';
              break;
            }
          }
          else{
            promedio_uno_accel = 0;
          }
          Serial.print("Aceleracion promediada: ");
          Serial.println(promedio_uno_accel);
          if (WiFi.status() == WL_CONNECTED) {
            
            HTTPClient http;
          // Configurar la URL y el header
            http.begin(serverName);
            http.addHeader("Content-Type", "application/json");
          // Crear el JSON con el valor del sensor
            String nombre_sensor = "MPU6050";
            String json = "{\"nombre_sensor\": \""+ nombre_sensor + "\", \"valor_sensor\": " + String(promedio_uno_accel) + "}";
          // Hacer la solicitud POST
            int httpResponseCode = http.POST(json);
            if (httpResponseCode > 0) {
              String response = http.getString();
              Serial.println(httpResponseCode);
              Serial.println(response);
          } else {
              Serial.print("Error code: ");
              Serial.println(httpResponseCode);
          }
            http.end();
          }



          float t = dht.readTemperature();
          

      if (isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        //return -1;
      }

      
      
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" *C \n");

      if(t < lim_inf ){
      ledRGB(0,0,255);
      myDFPlayer.play(2);
      }
      
      else if(t>lim_sup){
        ledRGB(140,150,0);
        myDFPlayer.play(1);
        //estado = '0';
        
      }
      else{
        ledRGB(0,255,0);
      }
        
        
        //Insertamos el primer promedio, el cual fue calculado al promediar cinco lecturas.
        temperatura.push_back(t);
        Serial.print("Longitud del vector de temperatura de promedio 1 minuto: ");
        Serial.println(temperatura.size());
        delay(8000);



      }

      //Ya sabemos que es de 6, es por si a caso.
      //Esto es para sacar el promedio del minuto.
      
      float suma = 0;
      for (int i = 0; i<temperatura.size();i++){
        suma+= temperatura[i];

      }
      Serial.print("Promedio de temperatura del minuto actual: ");
      Serial.println( (suma/temperatura.size())   );
      if (WiFi.status() == WL_CONNECTED) {
            
            HTTPClient http;
          // Configurar la URL y el header
            http.begin(serverName);
            http.addHeader("Content-Type", "application/json");
            valor_temperatura = (suma/temperatura.size());
            
          // Crear el JSON con el valor del sensor
            String nombre_sensor = "DHT22";
            String json = "{\"nombre_sensor\": \""+ nombre_sensor + "\", \"valor_sensor\": " + String(valor_temperatura) + "}";
          // Hacer la solicitud POST
            
        
            int httpResponseCode = http.POST(json);

            if (httpResponseCode > 0) {
              String response = http.getString();
              Serial.println(httpResponseCode);
              Serial.println(response);
          } else {
              Serial.print("Error code: ");
              Serial.println(httpResponseCode);
          }
            http.end();
          }



      temperatura_minutos.push_back( (suma/temperatura.size()) );
      Serial.print("Longitud del vector de temperatura por 5 minutos: ");
      Serial.println(temperatura_minutos.size());
      iteracion+=1;
      if(iteracion >num_minutos){
      std:: vector<float> aux = temperatura_minutos;
      temperatura_minutos.clear();
      for(int in = 1; in < aux.size();in++){
        temperatura_minutos.push_back(aux[in]);
      }
      Serial.print("Nueva longitud de vector temperatura_minutos: ");
      Serial.println(temperatura_minutos.size());
      //aux.clear();
      break;

      }



      }
    }//Aqui termina el if que estas usando.


  float promedio_minutos = 0;
  for(int in = 0; in < temperatura_minutos.size() ;in++){
    promedio_minutos+=temperatura_minutos[in];
  }

  promedio_minutos /= temperatura_minutos.size();
  Serial.print("Promedio en base al numero de minutos seleccionado: ");
  Serial.println(promedio_minutos);
  //Este condicional cambia el estado en base al promedio de los 5 minutos consecutivos.
  if(  (promedio_minutos > lim_sup) || (promedio_minutos < lim_inf) || (promedio_uno_accel > lim_accel) ) {
    estado = '0';
    sobreescribir(estado);

  }



    if(estado == '0'){

      ledRGB(255,0,0);
      myDFPlayer.play(3);
      //3 es igual a echado a perder.
      Serial.println("El medicamento se ha echado a perder!");
      delay(10000);
      
      
      reiniciar = digitalRead(reset);
      Serial.println(reiniciar);
      if( reiniciar == 1 ){
        temperatura_minutos.clear();
        sobreescribir('1');
        estado = '1';
        iteracion = 0;
        Serial.print("Nuevo valor de estado: ");
        Serial.println(estado);
      }

    }
    
   //////Aqui es donde terminaba el for
  //Condicional para saber a que cambiar el estado
  
  
  

  
  
  
  
}