/*************************************************************************************************
  Departamento de Electrónica
  Electrónica Digital II
  Profesor: Kurt Kellner
  Sección: 31
  Autor: Juan Pablo Valenzuela
  Descripción: Este es el código que monitorea el sistema de parqueos de Oakland Mall,
  Recibe los datos del estado por medio del uart y los envía al servidor.
**************************************************************************************************/
//************************************************************************************************
// Librerías
//************************************************************************************************
#include <WiFi.h>
#include <WebServer.h>
//************************************************************************************************
// Prototipos de función
//************************************************************************************************
void check_UART(void);
void UART2_CONFIG(void);
void update_satus(void);

//************************************************************************************************
// Variables globales
//************************************************************************************************
// SSID & Password
const char* ssid = "CLARO1_1C865E";  // Enter your SSID here
const char* password = "682k1wtuFc";  //Enter your Password here
//Prueba de strings
String text_string_test;

//Variables para recibir datos
char Datos[8]; //se guarda en memoria
char dato[8]; //entra directo del uart

//Variables de los parqueos
char Datos_old[8];

//Variables de control
int i = 0;
bool leer = LOW;
int change = 0; //variable detecta cambios en los parqueos

WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)


uint8_t LED1pin = 2;
bool LED1status = LOW;

//************************************************************************************************
// Configuración
//************************************************************************************************
void setup() {
  Serial.begin(115200);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  //Configuración uart2
  UART2_CONFIG();
  pinMode(LED1pin, OUTPUT);


  // Connect to your wi-fi modem
  WiFi.begin(ssid, password);

  // Check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32 IP on serial

  server.on("/", handle_OnConnect); // Directamente desde e.g. 192.168.0.8
  //Interrupciones de los diferentes botones
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);
  //Pruebas de string
  /*
  text_string_test = "var lugares_num = \""+String(3)+"\";";
  Serial.println(text_string_test);
  */

  
}














//************************************************************************************************
// loop principal
//************************************************************************************************
void loop() {
  server.handleClient();
  //Detectamos datos en el Buffer
  check_UART();
  update_satus();
  
    
}

















//************************************************************************************************
// Handler de Inicio página
//************************************************************************************************
void handle_OnConnect() {
  LED1status = LOW;
  Serial.println("GPIO2 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status));
}
//************************************************************************************************
// Handler de led1on
//************************************************************************************************
void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO2 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status));
}
//************************************************************************************************
// Handler de led1off
//************************************************************************************************
void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO2 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status));
}
//************************************************************************************************
// Procesador de HTML
//************************************************************************************************
String SendHTML(uint8_t led1stat) {
  String ptr = "<!DOCTYPE html>\n";
  ptr += "<html>\n";
  ptr += "<body>\n";
  ptr += "<canvas id=\"myCanvas\" width=\"1900\" height=\"900\" style=\"border:1px solid #d3d3d3;\">\n";
  ptr += "</canvas>\n";
  ptr += "<script>\n";
  ptr += "var c = document.getElementById(\"myCanvas\");\n";
  ptr += "var ctx = c.getContext(\"2d\");\n";
  //Variables que manejan el color
  ptr += "var green1 = \"#00F700\";\n";
  ptr += "var red1   = \"#FF2000\";\n";
  ptr += "ctx.lineWidth = \"7\";\n";
  ptr += "ctx.strokeStyle = \"black\";\n";
  //variables que manejan la posición de los cuadros
  ptr += "var v_spacing = 150;\n";
  ptr += "var x_rect = 650;\n"; //200 default
  ptr += "var y1_rect = 250;\n";
  ptr += "var y2_rect = y1_rect + v_spacing;\n";
  ptr += "var y3_rect = y2_rect + v_spacing;\n";
  ptr += "var y4_rect = y3_rect + v_spacing;\n";
  ptr += "var width_rect = 200;\n";
  ptr += "var height_rect = 100;\n";
  //Variables de texto
  ptr += "ctx.font = \"25px Helvetica\";\n";
  ptr += "var P1 = \"Parqueo 1\";\n";
  ptr += "var P2 = \"Parqueo 2\";\n";
  ptr += "var P3 = \"Parqueo 3\";\n";
  ptr += "var P4 = \"Parqueo 4\";\n";
  ptr += "var x_text  = x_rect + 43;\n";
  ptr += "var y1_text = y1_rect+(height_rect/2)+5;\n";
  ptr += "var y2_text = y1_text + v_spacing;\n";
  ptr += "var y3_text = y2_text + v_spacing;\n";
  ptr += "var y4_text = y3_text + v_spacing;\n";

  //***************************************************************************************************************
  //Sistema de numeración de lugares
  ptr += "var lugares_texto = \"Lugares disponibles: \";\n";
  ptr += "var lugares_num = \"3\";\n"; ////////////////////////////////////////////////////////////Modificar esta línea para poder actualizar los lugares disponibles
  //insertar string del numero de lugares disponibles
  ptr += "var lugares = lugares_texto + lugares_num;\n";
  //***************************************************************************************************************











  //Programa principal (dibujamos las cosas)
  //***************************************************************************************************************
  //Primer rectangulo (parqueo 1)
  ptr += "ctx.fillStyle = green1;\n";  //**************************************************************color de relleno
  ptr += "ctx.strokeRect(x_rect,y1_rect,width_rect,height_rect);\n";
  ptr += "ctx.fillRect(x_rect,y1_rect,width_rect,height_rect);\n";
  //Dibujamos el texto   
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.fillText(P1, x_text, y1_text);\n";

  //***************************************************************************************************************
  //Segundo rectangulo (parqueo 2)
  ptr += "ctx.fillStyle = green1;\n"; //**************************************************************color de relleno
  ptr += "ctx.strokeRect(x_rect,y2_rect,width_rect,height_rect);\n";
  ptr += "ctx.fillRect(x_rect,y2_rect,width_rect,height_rect);\n";
  //Dibujamos el texto
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.fillText(P2, x_text, y2_text);\n";

  //***************************************************************************************************************
  //Tercer rectangulo (parqueo 3)
  ptr += "ctx.fillStyle = green1;\n";  //**************************************************************color de relleno
  ptr += "ctx.strokeRect(x_rect,y3_rect,width_rect,height_rect);\n";
  ptr += "ctx.fillRect(x_rect,y3_rect,width_rect,height_rect);\n";
  //Dibujamos el texto
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.fillText(P3, x_text, y3_text);\n";

  //***************************************************************************************************************
  //Cuarto rectangulo (parqueo 4)
  ptr += "ctx.fillStyle = red1;\n";  //**************************************************************color de relleno
  ptr += "ctx.strokeRect(x_rect,y4_rect,width_rect,height_rect);\n";
  ptr += "ctx.fillRect(x_rect,y4_rect,width_rect,height_rect);\n";
  //Dibujamos el texto
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.fillText(P4, x_text, y4_text);\n";















  //***************************************************************************************************************
  //Dibujamos el título de la página
  ptr += "ctx.font = \"bold 80px Trebuchet MS\";\n";
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.textAlign = \"center\";\n";
  ptr += "ctx.fillText(\"Parqueos Oakland Mall\", 1900/2, 100);\n";
  
  //Dibujamos el autor y sus datos
  //context.font="italic small-caps bold 12px arial";
  ptr += "ctx.font = \"bold 30px Candara\";\n";
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.textAlign = \"center\";\n";
  ptr += "ctx.fillText(\"Por: Juan Pablo Valenzuela (18057)\", 1900/2, 150);\n";
  
  //Fuentes:
  //Comic Sans MS
  //Trebuchet MS
  //Candara (recomendada)
  //Tahoma (no recomendada)
  //Calibri
  //Cambria (recomendada)
  //Optima
  
  
  //Dibujamos los lugares disponibles
  ptr += "ctx.font = \"bold 30px Cambria\";\n";
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.textAlign = \"center\";\n";
  ptr += "ctx.fillText(lugares, x_rect+500, 305);\n";
  
  
  ptr += "</script>\n";
  
  ptr += "</body>\n";
  ptr += "</html>\n";
  
  
  
  return ptr;
}
//************************************************************************************************
// Handler de not found
//************************************************************************************************
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

//************************************************************************************************
// Revisamos si entró un dato por UART0 
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
    change = (dato[1]==Datos_old[1]) + (dato[2]==Datos_old[2]) + (dato[3]==Datos_old[3]) + (dato[4]==Datos_old[4]) + (dato[5]==Datos_old[5]);
    if (dato[1]!=Datos_old[1]) {
      Datos_old[1]=dato[1];
      Serial.print("Cambio dato 1\n");
    }
    if (dato[2]!=Datos_old[2]) {
      Datos_old[2]=dato[2];
      Serial.print("Cambio dato 2\n");
    }
    if (dato[3]!=Datos_old[3]) {
      Datos_old[3]=dato[3];
      Serial.print("Cambio dato 3\n");
    }
    if (dato[4]!=Datos_old[4]) {
      Datos_old[4]=dato[4];
      Serial.print("Cambio dato 4\n");
    }
    if (dato[5]!=Datos_old[5]) {
      Datos_old[5]=dato[5];
      Serial.print("Cambio dato 5\n");
    }
  
  }
