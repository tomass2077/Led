#include <Arduino.h>

/*
 Name:		SchoolLedMain.ino
 Created:	1/14/2022 12:20:31 PM
 Author:	Tomass
*/
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <SD.h>
#include <WString.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include <cstring>
#define UDP_PORT 4210
WiFiUDP UDP;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
void HandleMyData();
ESP8266WebServer server(80);
String ssid = "";
String pass = "";

const size_t capacit = 512 * 8;
DynamicJsonDocument doc(capacit);
File SdCard;
void Blank()
{
	for (int i = 0; i < 27; i++)
	{
		IPAddress pps;
		pps.fromString(doc["devices"][i]["IP"].as<String>());

		for (int j = 0; j < 64; j++)
		{
			UDP.beginPacket(pps, UDP_PORT);

			//for (int jj = 0; jj < 16; jj++)
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.endPacket();
		}
	}
}
void setup()
{
	u8g2.begin();
	Serial.begin(115200);
	WiFi.begin(ssid, pass);
	WiFi.hostname("espMain");
	while (WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		delay(500);
	}

	//const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
	SD.begin(D8);
	SdCard = SD.open("modules.json");
	String pp = SdCard.readString();
	//pp.trim();
	Serial.println(pp);
	Serial.println("Got IP: " + WiFi.localIP().toString());
	//const char* ssid = pp;
	DeserializationError err = deserializeJson(doc, pp);
	Serial.println(err.c_str());
	Serial.println(doc["devices"].size());
	if (1)
		for (int i = 0; i < 27; i++)
		{
			WiFiClient client;
			HTTPClient http;
			String serverPath = "http://" + doc["devices"][i]["IP"].as<String>() + "/espinator/UThere";
			http.setTimeout(300);
			http.begin(client, serverPath.c_str());
			int httpCode = http.GET();
			doc["devices"][i]["found"] = false;
			if (http.getString() != "")
				doc["devices"][i]["found"] = true;
			else
				doc["devices"][i]["found"] = false;

			u8g2.setFont(u8g2_font_profont12_tr);

			if (!doc["devices"][i]["found"])
				u8g2.setDrawColor(1);
			else
				u8g2.setDrawColor(1);
			int offset = 25;
			if (i < 3)
				u8g2.setCursor(5 + offset * 0, 9 + 9 * (i - 0));
			else if (i < 6)
				u8g2.setCursor(5 + offset * 1, 9 + 9 * (i - 3));
			else if (i < 9)
				u8g2.setCursor(5 + offset * 2, 9 + 9 * (i - 6));
			else if (i < 12)
				u8g2.setCursor(5 + offset * 3, 9 + 9 * (i - 9));
			else if (i < 15)
				u8g2.setCursor(5 + offset * 0, 15 + 9 * (i - 9));
			else if (i < 18)
				u8g2.setCursor(5 + offset * 1, 15 + 9 * (i - 12));
			else if (i < 21)
				u8g2.setCursor(5 + offset * 2, 15 + 9 * (i - 15));
			else if (i < 24)
				u8g2.setCursor(5 + offset * 3, 15 + 9 * (i - 18));
			else if (i < 27)
				u8g2.setCursor(5 + offset * 4, 15 + 9 * (i - 21));
			if (doc["devices"][i]["found"])
			{
				if (i < 10)
					u8g2.print(".0" + (String)i + ".");
				else
					u8g2.print("." + (String)i + ".");
			}
			else
			{
				if (i < 10)
					u8g2.print("?0" + (String)i + "?");
				else
					u8g2.print("?" + (String)i + "?");
			}
			u8g2.sendBuffer();
		}
	Blank();
	SdCard.close();
	server.on("/espinator/MyData", HandleMyData);
	server.begin();
	pinMode(0, INPUT_PULLUP);
}
bool Up = false;
bool Down = false;
bool UpRn = false;
bool DownRn = false;
bool butonRn = false;
bool buton = false;
bool butonOld = false;
void handleInput()
{
	UpRn = false;
	DownRn = false;
	butonRn = false;
	buton = digitalRead(0);
	int a = analogRead(0);
	if (buton != butonOld && buton == false)
		butonRn = true;
	if (a > 750)
	{
		if (!Up)
			UpRn = true;
		Up = true;
		Down = false;
	}
	else if (a < 250)
	{
		if (!Down)
			DownRn = true;
		Up = false;
		Down = true;
	}
	else
	{
		Up = false;
		Down = false;
	}

	butonOld = buton;
}
void ConnectedOnes()
{
	while (true)
	{
		u8g2.clearBuffer();
		server.handleClient();
		handleInput();
		if (butonRn)
		{
			break;
		}
		for (int i = 0; i < 27; i++)
		{
			u8g2.setFont(u8g2_font_profont12_tr);
			//u8g2.set
			if (!doc["devices"][i]["found"])
				u8g2.setDrawColor(1);
			else
				u8g2.setDrawColor(1);
			int offset = 25;
			if (i < 3)
				u8g2.setCursor(5 + offset * 0, 9 + 9 * (i - 0));
			else if (i < 6)
				u8g2.setCursor(5 + offset * 1, 9 + 9 * (i - 3));
			else if (i < 9)
				u8g2.setCursor(5 + offset * 2, 9 + 9 * (i - 6));
			else if (i < 12)
				u8g2.setCursor(5 + offset * 3, 9 + 9 * (i - 9));
			else if (i < 15)
				u8g2.setCursor(5 + offset * 0, 15 + 9 * (i - 9));
			else if (i < 18)
				u8g2.setCursor(5 + offset * 1, 15 + 9 * (i - 12));
			else if (i < 21)
				u8g2.setCursor(5 + offset * 2, 15 + 9 * (i - 15));
			else if (i < 24)
				u8g2.setCursor(5 + offset * 3, 15 + 9 * (i - 18));
			else if (i < 27)
				u8g2.setCursor(5 + offset * 4, 15 + 9 * (i - 21));
			if (bool(doc["devices"][i]["found"]))
			{
				if (i < 10)
					u8g2.print(".0" + (String)i + ".");
				else
					u8g2.print("." + (String)i + ".");
			}
			else
			{
				if (i < 10)
					u8g2.print("?0" + (String)i + "?");
				else
					u8g2.print("?" + (String)i + "?");
			}
		}
		u8g2.setDrawColor(1);

		u8g2.sendBuffer();
	}
	u8g2.setDrawColor(1);
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}
//DynamicJsonDocument Framies(16384*2);
String getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = {0, -1};
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++)
	{
		if (data.charAt(i) == separator || i == maxIndex)
		{
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void Blinky(String name)
{
	u8g2.clearBuffer();
	uint8_t color[5];
	uint8_t pp[250 * 27 * sizeof(color)];
	handleInput();
	handleInput();
	handleInput();
	Serial.println("started");
	//SdCard = SD.open("random.dat");
	SdCard = SD.open(name);
	Serial.println("open:" + name);
	//const char* cstr = name;
	//std::string pps(cstr);
	int frames = getValue(name, '_', 0).toInt();
	Serial.println("Created");
	SdCard.read((uint8_t *)&pp, sizeof(pp));
	Serial.print("read pp:");
	Serial.println(SdCard.readString());
	float biges = 0;
	int frame = 0;
	int frame2 = 0;
	bool runing = true;
	while (runing)
		for (int f = 0; f < frames; f++)
		{
			u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
			for (int y = 0; y < 9; y++)
				for (int x = 0; x < 3; x++)
				{
					int led = y * 3 + x;
					u8g2.setCursor(y * 13 + 10, x * 13 + 20);
					int fuR = pp[f * 27 * sizeof(color) + led * sizeof(color) + 0] * 2;
					int fuG = pp[f * 27 * sizeof(color) + led * sizeof(color) + 1] * 2;
					int fuB = pp[f * 27 * sizeof(color) + led * sizeof(color) + 2] * 2;
					int coli = (fuR + fuG + fuB) / 3;
					char myASCII[] = " .:-=+*#%@";
					char ap = myASCII[int(float(coli) / 255 * (sizeof(myASCII) / sizeof(*myASCII)))];
					if (coli > 230)
						ap = '@';
					u8g2.print(ap);
				}
			u8g2.sendBuffer();
			handleInput();
			biges += 0.2;
			if (frame > 255)
			{
				frame = 0;
				frame2++;
			}
			uint8_t a = (sin(biges) + 1) / 2 * 255;
			long startMilis = millis();
			server.handleClient();

			if (butonRn)
			{
				runing = false;
				break;
			}
			for (int i = 0; i < 27; i++)
			{
				IPAddress pps;
				pps.fromString(doc["devices"][i]["IP"].as<String>());
				if(doc["devices"][i]["found"])
				for (int j = 0; j < 16; j++)
				{
					UDP.beginPacket(pps, UDP_PORT);

					//for (int jj = 0; jj < 16; jj++)
					UDP.write(uint8_t(pp[f * 27 * sizeof(color) + i * sizeof(color) + 0] * 2));
					UDP.write(uint8_t(pp[f * 27 * sizeof(color) + i * sizeof(color) + 1] * 2));
					UDP.write(uint8_t(pp[f * 27 * sizeof(color) + i * sizeof(color) + 2] * 2));
					UDP.write(f);
					UDP.write(frame2);
					UDP.endPacket();
				}

				delay(1);
			}

			if ((millis() - startMilis) < 100)
			{
				delay(100 - (millis() - startMilis));
			}
		}

	Blank();
	Serial.println("Send done");
	SdCard.close();
	Serial.println("close");
}
int animSelect = 0;
bool isAnim(const std::string &str)
{
	std::string suffix = ".dat";
	bool result = false;
	return (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
	//return(true);
}
void Anims()
{
	File root = SD.open("/");
	int AnimCount = 0;
	int FileCount = 0;

	while (true)
	{
		File entry = root.openNextFile();
		if (!entry)
		{
			root.rewindDirectory();
			break;
		}
		else if (isAnim(entry.fullName()))
		{
			AnimCount++;
		}
		entry.close();
	}
	while (true)
	{
		File entry = root.openNextFile();
		if (!entry)
		{
			root.rewindDirectory();
			break;
		}
		else
		{
			FileCount++;
		}
		entry.close();
	}

	String *AnimList = new String[AnimCount + 1];
	int animAAAAA = 1;
	for (int i = 0; i < FileCount; i++)
	{
		File entry = root.openNextFile();
		if (isAnim(entry.fullName()))
		{
			AnimList[animAAAAA] = entry.name();
			entry.close();
			Serial.println(AnimList[animAAAAA]);
			animAAAAA++;
		}
	}
	AnimList[0] = "Exit";
	AnimCount++;
	while (true)
	{
		u8g2.clearBuffer();
		server.handleClient();
		handleInput();
		if (UpRn)
			animSelect++;
		if (DownRn)
			animSelect--;

		if (animSelect < 0)
			animSelect = AnimCount - 1;
		if (animSelect > AnimCount - 1)
			animSelect = 0;
		u8g2.setFont(u8g2_font_t0_11b_tf);

		for (int i = 0; i < AnimCount; i++)
		{
			int why = i - animSelect;
			if (why >= 0 && why < 4)
			{
				u8g2.setCursor(10, 12 + why * 12);
				if (animSelect == i)
					u8g2.print("| " + AnimList[i]);
				else
					u8g2.print(AnimList[i]);
			}
		}
		if (butonRn)
		{
			if (animSelect == 0)
				break;
			else
				Blinky(AnimList[animSelect]);
		}
		u8g2.sendBuffer();
	}
}

String menu[4] = {"Connected", "Anim", "Other2", "Other3"};
int selection = 0;
void loop()
{
	u8g2.clearBuffer();

	server.handleClient();
	handleInput();
	if (UpRn)
		selection++;
	if (DownRn)
		selection--;

	if (selection < 0)
		selection = 3;
	if (selection > 3)
		selection = 0;
	u8g2.setFont(u8g2_font_t0_11b_tf);

	for (int i = 0; i < 4; i++)
	{

		u8g2.setCursor(10, 12 + i * 12);
		if (selection == i)
			u8g2.print("| " + menu[i]);
		else
			u8g2.print(menu[i]);
	}
	if (butonRn)
	{
		if (selection == 0)
			ConnectedOnes();
		if (selection == 1)
			Anims();
	}
	u8g2.sendBuffer();
	delay(50);
}
void HandleMyData()
{
	if (server.arg("Mac") != "" && server.arg("IP") != "" && server.arg("ID") != "")
	{
		SdCard = SD.open("modules.json", 2);
		Serial.println(atoi(server.arg("ID").c_str()));
		//for (int i = 0; i < doc["devices"].size(); i++)
		//{
			//if (int(doc["devices"][i]["id"]) == server.arg("ID").c_str())
			//{
				server.send(200, "application/json", "{\"found\":true\"}");

				doc["devices"][atoi(server.arg("ID").c_str())]["IP"] = server.arg("IP");
				doc["devices"][atoi(server.arg("ID").c_str())]["Mac"] = server.arg("Mac");
				doc["devices"][atoi(server.arg("ID").c_str())]["found"] = true;
				//break;
			//}
		//}
		Serial.println(doc.as<String>());
		server.send(200, "application/json", "{ \"found\":false}");
		serializeJson(doc, SdCard);
		SdCard.close();
	}
}
