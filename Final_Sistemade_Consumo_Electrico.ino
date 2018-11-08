// Incluir Emon Library
#include "EmonLib.h"
// Incluir SPI y Ethernet Library
#include <SPI.h>
#include <Ethernet.h>

// Declaración de la direcciones MAC e IP. También del puerto 80
byte mac[]={0xDE,0xAD,0xBE,0xEF,0xFE,0xED}; //MAC
IPAddress ip(172,16,48,109); //IP
EthernetServer servidor(80);

//Declaracion de Variables
double sensorValue=0;
double sensorValue1=0;
int crosscount=0;
int climbhill=0;
double VmaxD=0;
double VeffD;
double Veff;

// Crear una instancia EnergyMonitor
EnergyMonitor energyMonitor;

void setup()
{
  Serial.begin(9600);

  //Inicializamos el servidor
  Ethernet.begin(mac, ip); 
  servidor.begin();  

  // Iniciamos la clase indicando
  // Número de pin: donde tenemos conectado el SCT-013
  // Valor de calibración: valor obtenido de la calibración teórica
  energyMonitor.current(8, 7.4);
}

void loop()
{
  // Funcionamiento Sensor Voltaje
  // Funcionamiento media onda 1
  sensorValue1=sensorValue;
  delay(50);
  sensorValue = analogRead(A9);
    if (sensorValue>sensorValue1 && sensorValue>511){
      climbhill=1;
      VmaxD=sensorValue;
  }
  // Funcionamiento media onda 2
    if (sensorValue<sensorValue1 && climbhill==1){
      climbhill=0;
      VmaxD=sensorValue1;
      VeffD=VmaxD/sqrt(2);
      Veff=(((VeffD-420.76)/-90.24)*-210.2)+155.2;
      VmaxD=0;
}
  // Funcionamiento Sensor de Amperaje
  // Obtenemos el valor de la corriente eficaz
  // Pasamos el número de muestras que queremos tomar
  double Irms = energyMonitor.calcIrms(1484);

  // Calculamos la potencia aparente
  double potencia =  Irms * Veff;

  // Mostramos la información por el monitor serie
  Serial.print("Potencia = ");
  Serial.print(potencia);
  Serial.print("    Irms = ");
  Serial.println(Irms);
  Serial.print("    Veff = ");
  Serial.println(Veff);


// listen for incoming clients
  EthernetClient cliente = servidor.available();
  if (cliente) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (cliente.connected()) {
      if (cliente.available()) {
        char c = cliente.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-Type: text/html");
          cliente.println("Connection: close");  // the connection will be closed after completion of the response
          cliente.println("Refresh: 2");  // refresh the page automatically every 5 sec
          cliente.println();
          cliente.println("<!DOCTYPE HTML>");
          cliente.print("<html lang='es'>");
          cliente.println("<html>");
          cliente.println("<head>");
          cliente.println("<title>CONTROL DE CONSUMO ELECTRICO</title>");
          cliente.println("<style type='text/css'> body { color: red; background-image: url('http://frostcode.es/descargas/back%20servidor.jpg'); } </style>");//EL CSS
          cliente.println("<meta charset='utf-8'>");
          cliente.println("</head>");
          
          cliente.println("<body>");


          cliente.print("<center>");   
          cliente.print("<h1>Bienvenido/a al Sistema de Monitoreo de Consumo Electrico</h1>");
          cliente.print("<br><hr><br>Aquí tiene el Consumo electrico que se esta presentando en este momento en su Casa/Laboratorio/Lugar de Trabajo<br><br>");
          cliente.print("</center>"); 

          cliente.print("<center>"); 
          cliente.print("Potencia = ");
          cliente.print(potencia);
          cliente.print("  W");
          cliente.print("    Irms = ");
          cliente.println(Irms);
          cliente.print("  A");
          cliente.print("    Veff = ");
          cliente.println(Veff);
          cliente.print("  V");
           cliente.print("</center>");
          
          cliente.println("</body>");
          cliente.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    cliente
    .stop();
    Serial.println("client disconnected");
  }
}
