#include <Arduino.h>
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
#include <SolarPosition.h>
#include <NTPClient.h>
//#include "LoadingBar.h"
float timeZone = 2;
float MaxSunAngle = 2;
struct tm startTime;
struct tm endTime;
#define UDP_PORT 4210
WiFiUDP UDP;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
void HandleMyData();
ESP8266WebServer server(80);
String ssid = "";
String pass = "";
const size_t capacity = 512 * 8;
DynamicJsonDocument doc(capacity);
File SdCard;

SolarPosition RigaSun(56, 24);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
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
void Blank()
{
	for (int i = 0; i < 27; i++)
	{
		IPAddress pps;
		pps.fromString(doc["devices"][i]["IP"].as<String>());

		for (int j = 0; j < 64; j++)
		{
			UDP.beginPacket(pps, UDP_PORT);

			// for (int jj = 0; jj < 16; jj++)
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.write(uint8_t(0));
			UDP.endPacket();
		}
	}
}

String loads[5];
void loadingBarClear()
{
	for (int i = 0; i < 5; i++)
		loads[i] = "";
}
void loadingBar(int filled, String txt, bool scroll)
{
	int with = 100;
	int height = 10;
	u8g2.clearBuffer();
	u8g2.drawFrame(128 / 2 - with / 2, 64 - height, with, height);
	u8g2.drawBox(128 / 2 - with / 2, 64 - height, filled, height);
	u8g2.setFont(u8g2_font_6x13_tr);
	if (scroll)
	{
		loads[4] = loads[3];
		loads[3] = loads[2];
		loads[2] = loads[1];
		loads[1] = loads[0];
		loads[0] = txt;
	}

	for (int i = 0; i < 5; i++)
	{
		u8g2.setCursor(1, 11 * (i + 1) - 5);
		u8g2.print(loads[4 - i]);
	}
	u8g2.sendBuffer();
}

int GetFounds()
{
	int found = 0;
	for (int i = 0; i < 27; i++)
		if (doc["devices"][i]["found"] == true)
			found++;
	return (found);
}
bool CheckTime = true;
bool CheckSunAngle = true;
void setup()
{

	startTime.tm_hour = 7;
	startTime.tm_min = 30;
	endTime.tm_hour = 17;
	endTime.tm_min = 0;

	Serial.begin(115200);
	u8g2.begin();
	u8g2.setFlipMode(1);
	loadingBar(5, "Starting SD card", true);
	Serial.print("sd begin:");
	if (!SD.begin(D8))
	{
		loadingBar(0, "SD failed", true);
		delay(5000);
		ESP.restart();
	}
	else
		loadingBar(10, "SD initialized", true);





	SdCard = SD.open("config.json", FILE_READ);
	if (SdCard.available())
		loadingBar(20, "Got config", true);
	else
	{
		loadingBar(0, "Mising config.json", true);
		delay(5000);
		ESP.restart();
	}
	DynamicJsonDocument config(512);
	deserializeJson(config, SdCard.readString());
	timeZone = config["TimeZone"];
	startTime.tm_hour = getValue(config["StopInMorning"], ':', 0).toInt();
	startTime.tm_min = getValue(config["StopInMorning"], ':', 1).toInt();
	endTime.tm_hour = getValue(config["StartInEvening"], ':', 0).toInt();
	endTime.tm_min = getValue(config["StartInEvening"], ':', 1).toInt();
	CheckTime = config["CheckTime"];
	MaxSunAngle = config["MaxSunAngle"];
	CheckSunAngle = config["CheckSunAngle"];
	ssid = config["ssid"].as<String>();
	pass = config["pass"].as<String>();
	RigaSun = SolarPosition(config["Latitude"].as<float>(), config["Longitude"].as<float>());
	if (config["Flip"])
		u8g2.setFlipMode(1);
	else
		u8g2.setFlipMode(0);
	bool SpeedBoot = config["SpeedBoot"];
	SdCard.close();

	if (!SpeedBoot)
		delay(500);





	loadingBar(20, "Connecting to Wi-Fi", true);
	WiFi.begin(ssid, pass);
	WiFi.hostname("espMain");
	int i = 1;
	while (WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		delay(500);
		i++;
		if (i > 10)
			loadingBar(29 + (i % 2), "Connecting to Wi-Fi", false);
		else
			loadingBar(20 + i, "Connecting to Wi-Fi", false);
	}
	loadingBar(30, "Wi-Fi done", false);
	if (!SpeedBoot)
	delay(500);

	loadingBar(30, "Getting time", true);
	timeClient.begin();
	timeClient.setTimeOffset(3600 * timeZone);
	timeClient.update();
	time_t epochTime = timeClient.getEpochTime();
	struct tm *ptm = gmtime((time_t *)&epochTime);
	loadingBar(40, String(ptm->tm_mday) + "/" + String(ptm->tm_mon + 1) + "/" + String(ptm->tm_year + 1900), true);
	loadingBar(40, timeClient.getFormattedTime(), true);
	if (!SpeedBoot)
	delay(1500);

	SdCard = SD.open("modules.json", FILE_READ);
	if (SdCard.available())
		loadingBar(60, "Got modules", true);
	else
	{
		loadingBar(0, "Mising modules.json", true);
		delay(5000);
		ESP.restart();
	}

	if (!SpeedBoot)
		delay(500);

	loadingBar(70, "IP: " + WiFi.localIP().toString(), true);
	String pp = SdCard.readString();
	if (!SpeedBoot)
	delay(500);
	// pp.trim();
	Serial.println(pp);
	Serial.println("Got IP: " + WiFi.localIP().toString());
	// const char* ssid = pp;
	DeserializationError err = deserializeJson(doc, pp);
	Serial.println(err.c_str());
	Serial.println(doc["devices"].size());
	u8g2.clearDisplay();
	if (1)
		for (int i = 0; i < 27; i++)
		{
			WiFiClient client;
			HTTPClient http;
			String serverPath = "http://" + doc["devices"][i]["IP"].as<String>() + "/espinator/UThere";
			http.setTimeout(300);
			http.begin(client, serverPath.c_str());
			// int httpCode = http.GET();
			http.GET();
			doc["devices"][i]["found"] = false;
			if (http.getString() != "")
			{
				doc["devices"][i]["found"] = true;
			}
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
	SD.remove("modules.json");
	SdCard = SD.open("modules.json", FILE_WRITE);
	serializeJson(doc, SdCard);
	SdCard.close();

	server.on("/espinator/MyData", HandleMyData);
	server.begin();
	pinMode(0, INPUT_PULLUP);
	loadingBar(100, "Found: " + String(GetFounds()) + "/27", true);
	loadingBar(100, "Done!", true);
	if (!SpeedBoot)
	delay(1000);
}
bool Up = false;
bool Down = false;
bool UpRn = false;
bool DownRn = false;
bool buttonRn = false;
bool button = false;
bool buttonOld = false;
void handleInput()
{
	UpRn = false;
	DownRn = false;
	buttonRn = false;
	button = digitalRead(0);
	int a = analogRead(0);
	if (button != buttonOld && button == false)
		buttonRn = true;
	if (a > 750)
	{
		if (!Down)
			DownRn = true;
		Up = false;
		Down = true;
	}
	else if (a < 250)
	{

		if (!Up)
			UpRn = true;
		Up = true;
		Down = false;
	}
	else
	{
		Up = false;
		Down = false;
	}

	buttonOld = button;
	// u8g2.setFont(u8g2_font_6x13_tr); u8g2.drawStr(0,64,("Ram:"+String(system_get_free_heap_size())).c_str());
}
void ConnectedOnes()
{
	while (true)
	{
		u8g2.clearBuffer();
		server.handleClient();
		handleInput();
		if (buttonRn)
		{
			break;
		}
		for (int i = 0; i < 27; i++)
		{
			u8g2.setFont(u8g2_font_profont12_tr);
			// u8g2.set
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

void Info1()
{
	timeClient.update();
	while (true)
	{
		u8g2.clearBuffer();
		server.handleClient();
		handleInput();
		if (buttonRn)
		{
			break;
		}
		time_t epochTime = timeClient.getEpochTime();
		struct tm *ptm = gmtime((time_t *)&epochTime);
		u8g2.setFont(u8g2_font_6x13_tf);
		u8g2.setCursor(1, 11 * 1);
		u8g2.print("Time:" + timeClient.getFormattedTime());
		u8g2.setCursor(1, 11 * 2);
		u8g2.print("Date:" + String(ptm->tm_mday) + "/" + String(ptm->tm_mon + 1) + "/" + String(ptm->tm_year + 1900));
		u8g2.setCursor(1, 11 * 3);
		u8g2.print("IP: " + WiFi.localIP().toString());
		u8g2.setCursor(1, 11 * 4);
		u8g2.print("Found: " + String(GetFounds()) + "/27");
		u8g2.setCursor(1, 11 * 5);
		u8g2.print("Sun: " + String(RigaSun.getSolarElevation(epochTime - 3600 * 2)) + "deg");
		u8g2.sendBuffer();
	}
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}
void Info()
{
	Info1();
	timeClient.update();
	while (true)
	{
		u8g2.clearBuffer();
		server.handleClient();
		handleInput();
		if (buttonRn)
		{
			break;
		}
		//time_t epochTime = timeClient.getEpochTime();
		//struct tm *ptm = gmtime((time_t *)&epochTime);
		u8g2.setFont(u8g2_font_6x13_tf);
		u8g2.setCursor(1, 11 * 1);
		u8g2.print("Stop time:" + String(startTime.tm_hour)+":"+String(startTime.tm_min));
		u8g2.setCursor(1, 11 * 2);
		u8g2.print("Start time:" + String(endTime.tm_hour)+":"+String(endTime.tm_min));
		u8g2.setCursor(1, 11 * 3);
		u8g2.print("ssid: " + ssid);
		u8g2.setCursor(1, 11 * 4);
		if(timeZone<0)
			u8g2.print("TimeZone:" + String(timeZone));
		else
			u8g2.print("TimeZone:+" + String(timeZone));
		//u8g2.setCursor(1, 11 * 5);
		//u8g2.print("Sun: " + String(RigaSun.getSolarElevation(epochTime - 3600 * 2)) + "deg");
		u8g2.sendBuffer();
	}
	u8g2.clearBuffer();
	u8g2.sendBuffer();
}
// DynamicJsonDocument Frames(16384*2);

void Blinky(String name)
{
	u8g2.clearBuffer();
	uint8_t color[5];
	uint8_t pp[250 * 27 * sizeof(color)];
	handleInput();
	handleInput();
	handleInput();
	Serial.println("started");
	// SdCard = SD.open("random.dat");
	SdCard = SD.open(name);
	Serial.println("open:" + name);
	// const char* cstr = name;
	// std::string pps(cstr);
	int frames = getValue(name, '_', 0).toInt();
	Serial.println("Created");
	SdCard.read((uint8_t *)&pp, sizeof(pp));
	Serial.print("read pp:");
	Serial.println(SdCard.readString());
	int frame = 0;
	int frame2 = 0;
	bool runing = true;

	while (runing)
	{
		u8g2.clearBuffer();
		handleInput();
		if (frame > 255)
		{
			frame = 0;
			frame2++;
		}
		if (frame2 > 255)
			frame2 = 0;
		server.handleClient();

		if (buttonRn)
		{
			runing = false;
			break;
		}

		// this is really dum but it works
		time_t epochTime = timeClient.getEpochTime();
		// struct tm *ptm = gmtime((time_t *)&epochTime);
		tm time;
		time.tm_sec = second(epochTime);
		time.tm_min = minute(epochTime);
		time.tm_hour = hour(epochTime);
		time.tm_mday = day(epochTime);
		time.tm_mon = month(epochTime) - 1;
		time.tm_year = year(epochTime) - 1900;

		startTime.tm_mday = day(epochTime);
		startTime.tm_mon = month(epochTime) - 1;
		startTime.tm_year = year(epochTime) - 1900;
		startTime.tm_sec = 0;

		endTime.tm_mday = day(epochTime);
		endTime.tm_mon = month(epochTime) - 1;
		endTime.tm_year = year(epochTime) - 1900;
		endTime.tm_sec = 0;
		time_t time_epoch = mktime(&time);
		time_t endTime_epoch = mktime(&endTime);
		time_t startTime_epoch = mktime(&startTime);
		time_t TimeToEnd = endTime_epoch - time_epoch;
		time_t TimeToStart = startTime_epoch - time_epoch;
		bool dont = TimeToStart < 0 && TimeToEnd > 0;

		if ((RigaSun.getSolarElevation(epochTime - 3600 * timeZone) < MaxSunAngle || !CheckSunAngle) && (!dont || !CheckTime))
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
				if (frame > 255)
				{
					frame = 0;
					frame2++;
				}
				long startMilis = millis();
				server.handleClient();

				if (buttonRn)
				{
					runing = false;
					break;
				}
				for (int i = 0; i < 27; i++)
				{
					IPAddress pps;
					pps.fromString(doc["devices"][i]["IP"].as<String>());
					if (doc["devices"][i]["found"])
						for (int j = 0; j < 16; j++)
						{
							UDP.beginPacket(pps, UDP_PORT);

							// for (int jj = 0; jj < 16; jj++)
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
		else
		{
			u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
			u8g2.setCursor(1, 10);
			u8g2.print("Time to start:");
			u8g2.setCursor(1, 20);
			u8g2.print(String(TimeToEnd));
			u8g2.sendBuffer();
			// delay(20);
		}
		// delete ptm;
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
	return (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
	// return(true);
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
		if (buttonRn)
		{
			if (animSelect == 0)
				break;
			else
				Blinky(AnimList[animSelect]);
		}
		u8g2.sendBuffer();
	}
}

String menu[4] = {"Connected", "Anim", "Info", "Reboot"};
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
	if (buttonRn)
	{
		if (selection == 0)
			ConnectedOnes();
		if (selection == 1)
			Anims();
		if (selection == 2)
			Info();
		if (selection == 3)
			ESP.restart();
	}
	u8g2.sendBuffer();
	delay(50);
}
void HandleMyData()
{
	if (server.arg("Mac") != "" && server.arg("IP") != "" && server.arg("ID") != "")
	{
		Serial.println(atoi(server.arg("ID").c_str()));
		server.send(200, "application/json", "{\"found\":true\"}");
		doc["devices"][atoi(server.arg("ID").c_str())]["IP"] = server.arg("IP");
		doc["devices"][atoi(server.arg("ID").c_str())]["Mac"] = server.arg("Mac");
		doc["devices"][atoi(server.arg("ID").c_str())]["found"] = true;
		Serial.println(doc.as<String>());
		server.send(200, "application/json", "{ \"found\":false}");
		SD.remove("modules.json");
		SdCard = SD.open("modules.json", FILE_WRITE);
		serializeJson(doc, SdCard);
		SdCard.close();
	}
}