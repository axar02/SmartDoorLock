#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <FirebaseESP8266.h>

#define trigPin D0
#define echoPin D1
#define SERVO D2
#define SS_PIN D8
#define RST_PIN D4
#define FIREBASE_HOST "https://mipro-c6af8-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "Wr40s9LDQN9Kzg78wSQuzLcsBb6Ij2baq8jVy4kE"

const char *ssid     = "ridho";
const char *password = "ridho123";

long duration;
int distance,jam,menit,detik;
MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;
Servo servo1;
String kunci= "tempat sedang tutup";
String buka= "tempat sedang buka";
String path= "Distance";

FirebaseData firebaseData;
FirebaseData doorlock;
FirebaseData lock;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", 25200);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  SPI.begin();
  rfid.PCD_Init();
  
  servo.attach(SERVO);
  servo1.attach(D3);

 timeClient.begin();
}

void loop() {
timeClient.update();
Serial.print("Waktu : ");
Serial.print(timeClient.getHours());
Serial.print(":");
Serial.print(timeClient.getMinutes());
Serial.print(":");
Serial.println(timeClient.getSeconds());
delay(1000);
jam = timeClient.getHours();
menit = timeClient.getMinutes();
detik = timeClient.getSeconds();


digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
distance = duration * 0.034 / 2;
Serial.print("Distance: ");
Serial.print(distance);
Serial.println(" cm");
Firebase.setInt(firebaseData, path ,distance);
delay(500);

if ( timeClient.getHours() >=6 && timeClient.getHours() <= 20 && distance <= 80  ) {
servo1.write(0);
Serial.println("pintu terbuka");
Serial.println();
delay(3000);
}
else {
servo1.write(180);
}

if ( timeClient.getHours() >=20 && timeClient.getHours() <= 24 ) {
Firebase.setString(lock,"status", kunci);
servo.write(0);
Serial.println("terkunci");
Serial.println();
delay(1000);
}
else if ( timeClient.getHours() >=0 && timeClient.getHours() <= 6 ) {
Firebase.setString(lock,"status", kunci);
servo.write(0);
Serial.println("terkunci");
Serial.println();
delay(1000);
}
else {
Firebase.setString(lock,"status", buka);
Serial.println("tidak terkunci");
Serial.println();
servo.write(180);
}

if ( ! rfid.PICC_IsNewCardPresent()) { //memeriksa kartu
 return;
 }
if ( ! rfid.PICC_ReadCardSerial()) { //membaca kartu
// rfid.PICC_HaltA();  // Menahan kartu agar tidak terus membaca
 return;
 }
String id = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
     id.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
     id.concat(String(rfid.uid.uidByte[i], HEX));
     }
Serial.print("RFID UID: ");
Serial.println(id);
if (id.substring(1) == "d3 73 e7 a7" )
 {
 Firebase.setInt(doorlock, "petugas", id.substring(1) == "d3 73 e7 a7");
 Serial.println("SILAHKAN!!!!");
 Serial.println();
 servo.write(180);
 delay(1000);
 servo1.write(0);
 delay(3000);
 }
 else {
 Firebase.setInt(doorlock, "petugas", id.substring(1) == "d3 73 e7 a7");
 Serial.println("ANDA SIAPA!!!");
 Serial.println();
 servo.write(0);
 servo1.write(180);
 }

}
