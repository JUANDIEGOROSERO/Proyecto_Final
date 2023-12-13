#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

Servo myservo;  // crea el objeto servo



// GPIO de salida del servo
static const int servoPin = 13;

// GPIO de entrada de sensores
static const int Trigger_Comida = 2;   //Pin digital 2 para el Trigger del sensor
static const int Echo_Comida = 4;      //Pin digital 3 para el Echo del sensor

static const int Trigger_Perro = 18;   //Pin digital 2 para el Trigger del sensor
static const int Echo_Perro = 5;      //Pin digital 3 para el Echo del sensor

// Bandera de veces que se ha alimentado el perro
int Bandera_puerta;

// Porcentaje de comida
//int porcent;

// Reemplazar con tus credenciales de Wifi
const char* ssid     = "Camilo's Galaxy";
const char* password = "Camilo2004";

// Inicializa Bot Telegram
#define BOTtoken "6376433559:AAHX8KCyI5-ame4Tm2OlLHJnYDHnguO08Yg"  // Tu Bot Token (Obtener de Botfather)

#define CHAT_ID "1732582548"

WiFiClientSecure client;

UniversalTelegramBot bot(BOTtoken, client);


int Abrir_puerta(String chat_id){
  long t; //timepo que demora en llegar el eco
  long d; //distancia en centimetros
  

  int Apertura_puerta;
  
  myservo.write(-10);

  digitalWrite(Trigger_Perro, HIGH);
  delay(100);                       //Enviamos un pulso de 1s
  digitalWrite(Trigger_Perro, LOW);
  
  t = pulseIn(Echo_Perro , HIGH); //obtenemos el ancho del pulso
  d = t/59;                //escalamos el tiempo a una distancia en cm

  if (d <= 8){
    bot.sendMessage(chat_id, "Alimentando", "");
    Apertura_puerta = Bandera_puerta++ ;

    //return Bandera_puerta;

    myservo.write(45);
    delay(300);
    myservo.write(-10);

    delay(1000);
  }
  else{
    myservo.write(-10);
  }

  delay(100);
  return Apertura_puerta;

};



int Cantidad_Alimento(String chat_id){
  long t; //timepo que demora en llegar el eco
  long d; //distancia en centimetros

  digitalWrite(Trigger_Comida, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(Trigger_Comida, LOW);
  
  t = pulseIn(Echo_Comida, HIGH); //obtenemos el ancho del pulso
  d = t/59;                //escalamos el tiempo a una distancia en cm

  int d_vac = 15; //Distancia en la que el almacen esta vacio
  int cantidad = d_vac - d; // Distancia de comida
  
  int porcent = (cantidad * 100) / d_vac;
  return porcent;

  delay(100);
};

void handleNewMessages(int numNewMessages) {

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Usuario no autorizado", "");
      continue;
    }

    String text = bot.messages[i].text;
    
    String from_name = "Guest";
    if (text == "/start")
    {
      String welcome = "\n";
      welcome += "Esta Listo para revisar la informacion del alimentador y su mascota.\n\n";
      welcome += "/Cuanta_comida : Para saber cuanta comida queda en el contenedor.\n";
      welcome += "/Apertura_compuerta : Para saber cuantas veces se ha abierto la compuerta\n";
      bot.sendMessage(chat_id, welcome);
    }

    int Apertura_puerta = Abrir_puerta(chat_id);
    int porcent = Cantidad_Alimento(chat_id);

    if (text == "/Cuanta_comida"){
        String Mensaje_Comida = "La cantidad de comida de comida que hay en el almacen es de: " + String(porcent) + "%";
        bot.sendMessage(chat_id, Mensaje_Comida);
              
    }
    if ( text == "/Apertura_compuerta"){
      String Mensaje_Puerta = "Su perro ha abierto la compuerta "+ String(Apertura_puerta) + " veces.";
      bot.sendMessage(chat_id, Mensaje_Puerta);
    }

    if (porcent == 50){
      bot.sendMessage(chat_id, "La cantidad de alimento para tu mascota está al 50%");
    }

    if (porcent <= 25){
      bot.sendMessage(chat_id, "La cantidad de alimento para tu mascota es muy poca");
    }

    if (porcent <= 3){
      bot.sendMessage(chat_id, "La cantidad de alimento para tu mascota se acabó, recarga su almacen de comida por favor :D");
    }
  }
};

void setup() {
  Serial.begin(115200);

  myservo.attach(servoPin);  // vincula el servo en el servoPin

  // Conecta a red WiFi con SSID y password
  Serial.print("Conectado a "); 
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // Muestra IP local 
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot iniciado", "");

  pinMode(Trigger_Perro, OUTPUT);     //pin como salida
  pinMode(Echo_Perro, INPUT);         //pin como entrada
  digitalWrite(Trigger_Perro, LOW);   //Inicializamos el pin con 0

  pinMode(Trigger_Comida, OUTPUT);     //pin como salida
  pinMode(Echo_Comida, INPUT);         //pin como entrada
  digitalWrite(Trigger_Comida, LOW);   //Inicializamos el pin con 0
}

void loop() {
  while(1){
    Abrir_puerta(CHAT_ID);

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}
