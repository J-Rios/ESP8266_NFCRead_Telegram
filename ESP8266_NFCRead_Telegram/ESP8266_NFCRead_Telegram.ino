#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "MFRC522.h"

/***********************************************************************************************************/

// Parametros de conexion al Punto de Acceso
char ssid[] = "xxxxxxxxxxxxxxxxxxxxxx"; // A establecer por el usuario
char pass[] = "yyyyyyyy"; // A establecer por el usuario

// Token del BOT de Telegram (crear bot con botfather)
#define BOTtoken "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" // A establecer por el usuario (consultar mediante botFather)

#define RST_PIN 5 // RST-PIN for RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN  4  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2 

/***********************************************************************************************************/

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

/***********************************************************************************************************/

void setup()
{
    Serial.begin(9600);    // Initialize serial communications
    SPI.begin();           // Init SPI bus
    mfrc522.PCD_Init();    // Init MFRC522
    wifi_begin();
    telegram_begin();
    telegram_println("Ready to Read NFC Tags");
}

void loop()
{ 
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        delay(50);
        return;
    }
    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        delay(50);
        return;
    }
    // Show some details of the PICC (that is: the tag/card)
    dump_uid(mfrc522.uid.uidByte, mfrc522.uid.size);
}

// Helper routine to dump a byte array as hex values to Telegram
void dump_uid(byte *buffer, byte bufferSize)
{
    String data = "Card UID: ";
    
    for(byte i = 0; i < bufferSize; i++)
    {
        data = data + (buffer[i] < 0x10 ? " 0" : " ");
        data = data + String(buffer[i], HEX);
    }
    
    telegram_println(data);
}

void telegram_begin() // Necesario para determinar el chat id del bot
{
    long Bot_lasttime;
    int Bot_mtbs = 1000; // Tiempo entre lecturas de mensajes 1s
    bool start = false;

    while(start == false)
    {
        // El tiempo para consultar si hay mensajes nuevos ha transcurrido
        if (millis() > Bot_lasttime + Bot_mtbs)
        {
            int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
            if(numNewMessages)
            {
                start = true;
                Serial.println("Telegram begin success");
            }
            Bot_lasttime = millis();
        }
    }
}

void telegram_println(String msg)
{
    if(msg != "")
        bot.sendSimpleMessage(bot.messages[0].chat_id, msg, "");
}

String telegram_read()
{
    String msg = "";

    if(bot.getUpdates(bot.last_message_received + 1))
        msg = bot.messages[0].text;

    return msg;
}

void wifi_begin()
{
    // Reinicia la interfaz de conexion en caso de que estubiera conectado a una red
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(1000);
    
    // Conexion a la red
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    
    // Esperar la conexión
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Conectado a la red
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
}
