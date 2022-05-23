#include <ArduinoJson.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "GGWP"
#define WIFI_PASSWORD "modaldong"

/* 2. Define the API Key */
#define API_KEY "AIzaSyBtXJByvYRcevpiPO0IbQQvxsYXRAAUVVo"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "smarthome-e89c8"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "bana@ta.com"
#define USER_PASSWORD "123456"

//definisikan pin sensor 

#define sensorPIR 21 
#define sensorApi 23
#define sensorGas 36
#define sensorMC 19

//definis pin kipas buzzer dan lampu
#define kipas 15
#define lampuKamar 33
#define lampuRT 14
#define lampuDapur 26
#define lampuTeras 27
#define buzzer 17

//objek parasing dataJSON firestore
StaticJsonDocument<384> doc;

//deklarasi variabel collection database Firestore
String PathDapur = "dapur/dapur";
String PathKamar = "kamartidur/kamartidur";
String PathRT = "ruangtamu/ruangan";
String PathTeras = "teras/terasControl";

//deklarasi field database
String fieldpirRT = "pir";
String fieldD = "lampuDapur";
String fieldK = "lampuKamar";
String fieldRT = "lampu";
String fieldDKipas = "kipasDapur";
String fieldT = "lampuTeras";
String tadaima = "iamhome";

//define object patch data firestore
FirebaseJson content;

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//definisi penampungan variabel baca sensor
bool iamHome= true;
bool valPIR = true;
bool lampuK =true;
bool lampuD =true;
bool lampuRTvar =true;
bool lampuT =true;
bool kipasD = false;
int valgas = 200;
bool valApi = true;
bool valmc = true;
int state = 0;
int pintu =0;
bool pirstate = false;
bool gasstate = false;
int cekapi=0;
int count=0;
int check=0;
int openPIR=0;
int openAPI=0;
unsigned long dataMillis1 = 0;
unsigned long dataMillis2 = 0;
unsigned long dataMillis3 = 0;
unsigned long dataMillis4 = 0;
unsigned long dataMillis5 = 0;
unsigned long dataMillis6 = 0;

//variabel deteksi gas
float sensor_volt; // variabel nilai tegangan
  float RS_air; // variabel resistansi sensor
  float sensorrr;
  float ratio;// fariabel untuk R0
  float sensorValue;//variabel analog reading
  bool calibrate = false;
 // float a=574.25 ;
  //float b= -2.222;
  double m = -0.473;
  double b = 1.413;
  float R0;
void setup()
{

    Serial.begin(115200);

    //inisialisasi mode pin sensor,switch dan relay
    pinMode(kipas,OUTPUT);
    pinMode(lampuKamar,OUTPUT);
    pinMode(lampuRT,OUTPUT);
    pinMode(lampuDapur,OUTPUT);
    pinMode(lampuTeras,OUTPUT);
    pinMode(buzzer,OUTPUT);
    pinMode(sensorPIR,INPUT);
    pinMode(sensorApi,INPUT);
    pinMode(sensorGas,INPUT);
    pinMode(sensorMC,INPUT_PULLUP);


    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    
    Firebase.reconnectWiFi(true);
}

void loop()
{
  count++;
  
    valApi = digitalRead(sensorApi);
    valmc = digitalRead(sensorMC);

    
    if (Firebase.ready())
    {
        
        content.clear();

           keamananRumah();
 
        
    }

    if (Firebase.ready())
    {
       
        content.clear();
        switching();
      
     }
     if (Firebase.ready())
    {
        dataMillis5 = millis();
        content.clear();
        deteksiAPI();
      
     }
     

     if (Firebase.ready())
    {
        
        content.clear();
        deteksiMC();
     }

     if (Firebase.ready())
    {
        
        content.clear();
        deteksiPIR();
        
      
     }

     

     if (Firebase.ready())
    {
        dataMillis6 = millis();
        content.clear();
        gasdetector();
      
     }
  
}





//fungsi sistem keamanan rumah
void keamananRumah(){

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathRT.c_str(),"iamhome"))
            {
              
              DeserializationError error = deserializeJson(doc, fbdo.payload());
              if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return;
                }
              iamHome = doc["fields"]["iamhome"]["booleanValue"]; 
              Serial.print("POSISI : ");
              if(iamHome){
                
                Serial.println("SAYA SEDANG DIRUMAH");
                }
                else{
                  Serial.println("SAYA SEDANG DILUAR");
                  }
        
              }
        else
            Serial.println(fbdo.errorReason());
  
   if(!iamHome){
    content.set("fields/state/stringValue", " Saya Keluar");
    pintu =1;
     
    }
    else{
      
      content.set("fields/state/stringValue", " Saya pulang");
      pintu =0;
      }

      if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathRT.c_str(), content.raw(), "state")){}
            
        else
            Serial.println(fbdo.errorReason());
 
  }

  
//update/kirim nilai sensor di dapur
  void gasdetector(){
  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathDapur.c_str(),"kipasDapur"))
        {

              DeserializationError errorr = deserializeJson(doc, fbdo.payload());
            
            if (errorr) {
              Serial.print("deserializeJson() failed: ");
              Serial.println(errorr.c_str());
              return;
            }
            
            kipasD = doc["fields"]["kipasDapur"]["booleanValue"];
            
              
          }
          else{
            Serial.println(fbdo.errorReason());}


    
valgas = deteksiGAS();
  Serial.print("Sensor GAS PPM : ");Serial.print(valgas);Serial.print(" || ");
  
    if (!kipasD){

      if(valgas >=2000){
                     
         digitalWrite(kipas,HIGH);
         Serial.println("Kadar gas lebih dari 2000 PPM ,Kipas ON");
         gasstate=true;

         }
      else{
         digitalWrite(kipas,LOW);
         Serial.println("Kadar gas aman, Kipas OFF");
          }
      
      }

      else{

        Serial.println("KIPAS ON");
        digitalWrite(kipas,HIGH);
        }
    
      
        
        
        

  
  content.set("fields/gas/stringValue", String(valgas).c_str());
  content.set("fields/kipas/booleanValue", gasstate);
  if (Firebase.Firestore.patchDocument(&fbdo,FIREBASE_PROJECT_ID, "",PathDapur.c_str(),content.raw(),"kipas,gas")){
    
    }
            
  else{
    Serial.println(fbdo.errorReason());
    }
            
  
  
  
  }


  void deteksiPIR(){
  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathTeras.c_str(), "lampuTeras"))
            {
              
              DeserializationError errorr4 = deserializeJson(doc, fbdo.payload());
              if (errorr4) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(errorr4.c_str());
                return;
                }
              lampuT = doc["fields"]["lampuTeras"]["booleanValue"];

              }
            
              else{
                Serial.println(fbdo.errorReason());}

    
   Serial.print("KONDSI PIR : ");
          valPIR = digitalRead(sensorPIR);

          
          pirstate=lampuT;
            Serial.print("PIR ACTIVED || ");
          if(valPIR==HIGH && check==0)
          {

            digitalWrite(lampuTeras,HIGH);
            check=1;
            Serial.println("UPS....PIR DETECTING SOMETHING");
          }
          
          else if (valPIR==HIGH &&check==1)
          {
              digitalWrite(lampuTeras,HIGH);
              
              Serial.println("PIR STILL DETECTING ");
          }

          else if (valPIR==LOW && check==1)
          {

            digitalWrite(lampuTeras,LOW);
             check=0;
            Serial.println("DONE PIR LOW ||");

            
          }
          

          else{
              
           
            Serial.print("NO DETECTING ||");
            check=0;

            
            }

            
          if(check==0){
            
            if(lampuT==true){
              
              
              Serial.println("LAMP ON");
              digitalWrite(lampuTeras,HIGH);
              
              }
              else{
                Serial.println("PIR LOW");
                digitalWrite(lampuTeras,LOW);
                
                }
            
            }

            else{
              check=1;
              
              }
         
        
        content.set("fields/pir/booleanValue", valPIR);
        content.set("fields/lampuTeras/booleanValue", pirstate);

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathTeras.c_str(), content.raw(), "pir,lampuTeras")){}
            
        else{Serial.println(fbdo.errorReason());}
              

  }     

    
    
  void deteksiMC(){
     
    Serial.print("KONDISI PINTU : ");
    if(valmc == HIGH && state==0 && pintu==0){
    
    Serial.println("ADA YANG BUKA PINTU");
    state =1;
      }
    

    else if(valmc == HIGH  && pintu==1){
      digitalWrite(buzzer,HIGH);
      Serial.println("WARNING ADA PENYUSUP  BUZZER ON");
      
      }
  
      else{

        if (openAPI==1){
          Serial.println("BUZZER SEDANG DIGUNAKAN");
          state =0;
          }
          else{
            Serial.println("PINTU TERTUTUP");
        digitalWrite(buzzer,LOW);
        
        state =0;
            }
        
        }

      content.set("fields/mc/booleanValue", valmc);
        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathRT.c_str(), content.raw(), "mc")){}
            
        else
            Serial.println(fbdo.errorReason());
  
      
    }
  void deteksiAPI(){

  Serial.print("KONDISI API : ");
      
   if(valApi==HIGH ){
    digitalWrite(buzzer,HIGH);
    Serial.println("TERDETEKSI API BUZZER ON");
    openAPI=1;
    
      }
    else if(valApi==LOW ){
      digitalWrite(buzzer,LOW);
      Serial.println(" API PADAM BUZZER OFF");
      openAPI=0;
      
  }

  
  else{
    
      Serial.println("TIDAK TERDETEKSI API ");
      
    }

    
        content.set("fields/fire/booleanValue", valApi);
        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathRT.c_str(), content.raw(), "fire")){}
            
        else
            Serial.println(fbdo.errorReason());
    
    }
  int deteksiGAS(){
    
     sensorValue = analogRead(sensorGas);
  if(sensorValue<=0){sensorValue = 1;}
   

  if(!calibrate){
    calibrate = true;
    
    for (int i =0;i<50;i++)
  {
    sensorValue = sensorValue+analogRead(sensorGas);
    
    }

    sensorValue = sensorValue/50;
    sensor_volt = sensorValue*(3.3/4095.0);//convert average to voltage
    RS_air = (3.3/sensor_volt-1)*10.0;//calculate RS in fresh air
    R0= RS_air/9.8;
    }
  
  
  sensor_volt = sensorValue*(3.3/4095.0);//convert average to voltage
 // RS_air = ((3.3-sensor_volt)/sensor_volt);
  //RS_air = ((3.3*10.0)/sensor_volt)-10.0;
 float RS= (3.3/sensor_volt-1)*10.0;//calculate RS in fresh air
  ratio = RS/R0;
  if(ratio<0){ratio=0;}
  //double ppm = a*pow(ratio,b);
  double ppm_log = (log10(ratio)-b)/m ;
  double ppm = pow(10,ppm_log);
  if(ppm>=10000){
    ppm=10000;
    
    }


return ppm;
    
    }



//switching system
  void switching(){
Serial.print("Relay 1 : ");
              //switching Lampu Ruang Tamu
              if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathRT.c_str(), "lampu"))
            {
              
              DeserializationError error3 = deserializeJson(doc, fbdo.payload());
              if (error3) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error3.c_str());
                return;
                }
              lampuRTvar = doc["fields"]["lampu"]["booleanValue"];
              
                  if(lampuRTvar){
                    digitalWrite(lampuRT,HIGH);
                    Serial.println("LAMPU RUANG TAMU ON"); 
                  }
                  else{
                    digitalWrite(lampuRT,LOW);
                    Serial.println("LAMPU RUANG TAMU OFF");
                  }  
              }
            
              else{
                Serial.println(fbdo.errorReason());}
                  


          

    
        
            
          Serial.print("Relay 3: ");
          //switching lampu Kamar
          if(Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathKamar.c_str(), "lampuKamar"))
            {
              
              DeserializationError error1 = deserializeJson(doc, fbdo.payload());
              if (error1) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error1.c_str());
                return;
                }
              lampuK = doc["fields"]["lampuKamar"]["booleanValue"];
                  if(lampuK){
                    digitalWrite(lampuKamar,HIGH);
                    Serial.println("LAMPU KAMAR ON"); 
                  }
                  else{
                    digitalWrite(lampuKamar,LOW);
                    Serial.println("LAMPU KAMAR OFF");
                  }  
              }
              else
              {
                  Serial.println(fbdo.errorReason());
              }

Serial.print("Relay 4 : ");
      //switching Dapur
        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", PathDapur.c_str(),"lampuDapur"))
        {

              DeserializationError error2 = deserializeJson(doc, fbdo.payload());
            
            if (error2) {
              Serial.print("deserializeJson() failed: ");
              Serial.println(error2.c_str());
              return;
            }
            lampuD = doc["fields"]["lampuDapur"]["booleanValue"];
            
            if(lampuD){
    
              digitalWrite(lampuDapur,HIGH);
              Serial.println("LAMPU DAPUR ON");
            
            }
            else{
              digitalWrite(lampuDapur,LOW);
              Serial.println("LAMPU DAPUR OFF");
            } 

            
              
          }
          else{
            Serial.println(fbdo.errorReason());}

                
                 
  }
