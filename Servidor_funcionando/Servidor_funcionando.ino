//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

//************************************************************************************************
// Prototipos de función
//************************************************************************************************
void check_UART(void);
void UART2_CONFIG(void);
void update_satus(void);
String send_json(void);
void toggle_led(void);

//************************************************************************************************
// Variables de uso general
//************************************************************************************************
//Variables para recibir datos
char Datos[8]; //se guarda en memoria
char dato[8]; //entra directo del uart
char int_disponible;
char dato_json[] = "\"parqueo1\":0,";

//Variables de los parqueos
char Datos_old[8];

//Variables de control
int i = 0;
bool leer = LOW;
int change = 0; //variable detecta cambios en los parqueos
char parqueo_disponible;
unsigned long previous_millis = 0;
unsigned long current_millis = 0;
unsigned long interval = 1000;

//Variable donde está la página web


//LED INTEGRADO
uint8_t LED1pin = 2;

//************************************************************************************************
// Variables y procesos del servidor
//************************************************************************************************

AsyncWebServer server(80);

const char* ssid = "CLARO1_1C865E";  // Enter your SSID here
const char* password = "682k1wtuFc";  //Enter your Password here

const char* PARAM_MESSAGE = "message";














//*********************************************************************************************************************
//    PROCESADOR HTML
//*********************************************************************************************************************
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<body>

<canvas id="myCanvas" width="1900" height="900" style="border:1px solid #d3d3d3;">
</canvas>

<script>
var c = document.getElementById("myCanvas");
var ctx = c.getContext("2d");





//FUNCION QUE PIDE DATOS AL ESP32
//La función setInterval ejecuta una rutina cada cierto tiempo
setInterval(function(){ 
  //Acá va la rutina
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "http://192.168.1.11/leer");

  xhr.responseType = 'json';
  
  xhr.onload = function() {
    var receive = xhr.response;
    console.log(receive);
    //Programa principal (dibujamos las cosas)
    //Primer rectangulo (parqueo 1)
    Pbox(x_rect,y1_rect,width_rect,height_rect,receive.parqueo1,P1,x_rect + 98);
    
    //Segundo rectangulo (parqueo 2)
    Pbox(x_rect,y2_rect,width_rect,height_rect,receive.parqueo2,P2,x_rect + 98);
    
    //Tercer rectangulo (parqueo 3)
    Pbox(x_rect,y3_rect,width_rect,height_rect,receive.parqueo3,P3,x_rect + 98);
    
    //Cuarto rectangulo (parqueo 4)
    Pbox(x_rect,y4_rect,width_rect,height_rect,receive.parqueo4,P4,x_rect + 98);

    //Dibujamos los lugares disponibles
    //Primero borramos el texto
    ctx.fillStyle = white1;
    ctx.fillRect(x_rect+320,y1_rect+20,400,50);
    //Luego escribimos el nuevo
    lugares(receive.disponible);
    
  };

  xhr.send();
  
  
 }, 250);
//FIN FUNCION QUE PIDE DATOS AL ESP32







//variables que manejan el color
var green1 = "#00F700";
var red1   = "#FF2000";
var white1 = "#FFFFFF";
ctx.lineWidth = "7";
ctx.strokeStyle = "black";


//variables que manejan la posición de los cuadros
var v_spacing = 150;
var x_rect = 650; //200 default
var y1_rect = 250;
var y2_rect = y1_rect + v_spacing;
var y3_rect = y2_rect + v_spacing;
var y4_rect = y3_rect + v_spacing;
var width_rect = 200;
var height_rect = 100;
//Variables de texto
ctx.font = "25px Helvetica";
var P1 = "Parqueo 1";
var P2 = "Parqueo 2";
var P3 = "Parqueo 3";
var P4 = "Parqueo 4";
var x_text  = x_rect + 43;
//var x_text  = x_rect + 43;
var y1_text = y1_rect+(height_rect/2)+5;
var y2_text = y1_text + v_spacing;
var y3_text = y2_text + v_spacing;
var y4_text = y3_text + v_spacing;


//Control del programa
var color1 = 1;
var color2 = 1;
var color3 = 1;
var color4 = 1;






//Función dibujar rectángulo
function Pbox(x, y, width, height, color, parqueo, x_text_1){

  if (color==0) {
  ctx.fillStyle = green1;
  } else {
  ctx.fillStyle = red1;
  }
  ctx.strokeRect(x,y,width,height);
  ctx.fillRect(x,y,width,height);
  //Dibujamos el texto
  ctx.fillStyle = "black";
  ctx.fillText(parqueo, x_text_1, y+(height/2)+5);

return 0;
};

//Función para dibujar lugares disponibles
function lugares(lugares_num){
  //variables de control
  //Primero borramos lo que sea que esté en el lugar donde ponemos el texto
  
  //Sistema de numeración de lugares
  var lugares_texto = "Lugares disponibles: ";
  var lugares = lugares_texto + lugares_num;
  //Dibujamos los lugares disponibles
  ctx.font = "bold 30px Cambria";
  ctx.fillStyle = "black";
  ctx.textAlign = "center";
  ctx.fillText(lugares, x_rect+500, 305);
  return 0;
};





//Programa principal (dibujamos las cosas)
//Primer rectangulo (parqueo 1)
Pbox(x_rect,y1_rect,width_rect,height_rect,color1,P1,x_rect + 43);

//Segundo rectangulo (parqueo 2)
Pbox(x_rect,y2_rect,width_rect,height_rect,color2,P2,x_rect + 43);

//Tercer rectangulo (parqueo 3)
Pbox(x_rect,y3_rect,width_rect,height_rect,color3,P3,x_rect + 43);

//Cuarto rectangulo (parqueo 4)
Pbox(x_rect,y4_rect,width_rect,height_rect,color4,P4,x_rect + 43);

//Dibujamos los lugares disponibles
lugares('4');




//Dibujamos el título de la página
ctx.font = "bold 80px Trebuchet MS";
ctx.fillStyle = "black";
ctx.textAlign = "center";
ctx.fillText("Parqueos Oakland Mall", 1900/2, 100);

//Dibujamos el autor y sus datos
//context.font="italic small-caps bold 12px arial";
ctx.font = "bold 30px Candara";
ctx.fillStyle = "black";
ctx.textAlign = "center";
ctx.fillText("Por: Juan Pablo Valenzuela (18057)", 1900/2, 150);

//Fuentes:
//Comic Sans MS
//Trebuchet MS
//Candara (recomendada)
//Tahoma (no recomendada)
//Calibri
//Cambria (recomendada)
//Optima






</script>

</body>
</html>
)rawliteral";
















//************************************************************************************************
// Handles
//************************************************************************************************
//************************************************************************************************
// Handle notFound
//************************************************************************************************
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


//************************************************************************************************
// Handle Root
//************************************************************************************************
void hanldeRoot(AsyncWebServerRequest *request){
      request->send(200, "text/html", index_html);
    }
//Se encarga de cargar la página al inicio

//************************************************************************************************
// Handle datos
//************************************************************************************************
void hanlde_datos(AsyncWebServerRequest *request){
      //Serial.println("Pidiendo datos");
      
      
      request->send(200, "text/plain", send_json());
    }














//************************************************************************************************
// Configuraciones
//************************************************************************************************
void setup() {
    //Pin integrado como indicador de online
    pinMode(LED1pin, OUTPUT);
    Serial.begin(115200);

    //CONFIGURACION UART2
    UART2_CONFIG();

    //CONFIGURACION ONLINE
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    // Check wi-fi is connected to wi-fi network
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected successfully");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());  //Show ESP32 IP on serial

    // HANDLE DE LA PÁGINA WEB
    server.on("/", hanldeRoot); 

    
    // HANDLE PARA ENVIAR DATOS MÁS ACTUALES
    server.on("/leer", hanlde_datos);

    server.onNotFound(notFound);

    server.begin();
}






//************************************************************************************************
// Código principal
//************************************************************************************************

void loop() {
  //Parpadeo de luces arbitrario
  toggle_led();
  //Revisamos el estado de los botones (parqueos)
  check_UART();
  update_satus();
  
}









//************************************************************************************************
// Revisamos si entró un dato por UART2
//Probar con
//************************************************************************************************
void check_UART(void){
  if (Serial2.available()){
    //detectamos cuando entraron 7 bytes
    //Leemos todo de un solo
    for (int j=0;j<7;j++)
      dato[j] = Serial2.read();    
    }
    
}

void UART2_CONFIG(void){
  Serial2.begin(115200, SERIAL_8N1);
  Serial2.print("UART2 funciona");
}


//*********************************************************************************************************************
//    ACTUALIZAMOS LOS DATOS
//*********************************************************************************************************************
void update_satus(void){    
  change = 0;
  if (dato[1]!=Datos_old[1]) {
    Datos_old[1]=dato[1];
    change++;
  }
  if (dato[2]!=Datos_old[2]) {
    Datos_old[2]=dato[2];
    change++;
  }
  if (dato[3]!=Datos_old[3]) {
    Datos_old[3]=dato[3];
    change++;
  }
  if (dato[4]!=Datos_old[4]) {
    Datos_old[4]=dato[4];
    change++;
  }
  if (dato[5]!=Datos_old[5]) {
    Datos_old[5]=dato[5];
    parqueo_disponible = dato[5];
    change++;
  }
  if (change>0){
    //Acá detectatmos si hubo algún cambio en los botones
    for (int j=0;j<7;j++){
      Serial.print(dato[j]);    
    }
    Serial.print('\n');
  }
}









String send_json(void){
String  dato_json  = "{\"parqueo1\":";
        dato_json += dato[1];
        dato_json += ",";
        dato_json += "\"parqueo2\":";
        dato_json += dato[2];
        dato_json += ",";
        dato_json += "\"parqueo3\":";
        dato_json += dato[3];
        dato_json += ",";
        dato_json += "\"parqueo4\":";
        dato_json += dato[4];
        dato_json += ",";
        dato_json += "\"disponible\":";
        dato_json += "\"";
        dato_json += dato[5];
        dato_json += "\"";
        dato_json += "}";
  return dato_json;
}





//Efectos de luces
void toggle_led(void){
  current_millis = millis();
  if ((current_millis - previous_millis)>=interval) {
    digitalWrite(LED1pin, HIGH);
    i++;
    previous_millis = current_millis;
    if (i==2){
      digitalWrite(LED1pin, LOW);
      i=0;
    }
  }
}
