#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Rang Dong Lab_2G";
const char* pass = "ktdt123456";
const char* broker = "192.168.31.5"; //Mon IP : 192.168.31.195
const char* colorTopic = "/topic/louis/color";
const char* intensityTopic = "/topic/louis/intensity";
const char* morseTopic = "/topic/louis/morse";
const char* romainTopic = "/topic/romain/led";
const char* simonTopic = "/topic/simon/switch";
const char* romainSwitchTopic = "/topic/louis/romain/switch";
char val[10];
int last, sw = 0;
int intensity;
const char *alpha_morse[]={".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..","--","-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--.."};
const char *num_morse[]={"-----",".----","..---","...--","....-",".....","-....","--...","---..","----."};

WiFiClient espClient;
PubSubClient client(espClient);
long currentTime, lastTime;
int count = 0;
char messages[50];

void setupWifi(){
    delay(1000);

    Serial.print("\nConnecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);

    while(WiFi.status() != WL_CONNECTED){
        delay(1000);
        Serial.print("-");
    }

    Serial.print("\nConnecting to ");
    Serial.println(ssid);
}

void reconnect(){
    while(!client.connected()){
        Serial.print("\nConnecting to ");
        Serial.println(broker);
        if(client.connect("Test")){
            Serial.print("\nConnected to ");
            Serial.println(broker);
            client.subscribe(colorTopic);
            client.subscribe(intensityTopic);
            client.subscribe(morseTopic);
            client.subscribe(romainSwitchTopic);
        }
        else{
            Serial.println("\nTrying to connect again");
            delay(5000);
        }
    }
}

void rgb_led(char* payload){
    int r, g, b;
    if (sscanf(payload, "rgb(%d, %d, %d)", &r, &g, &b) == 3){
        Serial.print("\nR=");
        Serial.println(r);
        Serial.print("\nG=");
        Serial.println(g);
        Serial.print("\nB=");
        Serial.println(b);
    }
    else if (sscanf(payload, "#%02x%02x%02x", &r, &g ,&b) == 3){
        Serial.print("\nR=");
        Serial.println(r);  
        Serial.print("\nG=");
        Serial.println(g);
        Serial.print("\nB=");
        Serial.println(b);
    }
    else Serial.print("\nErreur lors de l'analyse");
    analogWrite(32, 255-r);
    analogWrite(22, 255-g);
    analogWrite(23, 255-b);
}

void on_off_switch(char* payload){
    if (strcmp((char*)payload, "on")){
        analogWrite(13, intensity);
    }
    else if (strcmp((char*)payload, "off")){
        analogWrite(13, 0);
    }
    else {
        Serial.print("\nError\npayload=");
        Serial.println(payload);
    }
}

void intensity_fun(char* payload){
    if (atoi(payload) >= 0 && atoi(payload) <= 255){
        Serial.print("\nIntensity:");
        Serial.println(payload);
        intensity = atoi(payload);
    }
    else {
        Serial.print("\nError\npayload=");
        Serial.println(payload);
    }
}

void long_led(int pin){
    analogWrite(pin,255);
    delay(500);
    analogWrite(pin,0);
    delay(100);
}

void short_led(int pin){
    analogWrite(pin,255);
    delay(100);
    analogWrite(pin,255);
    delay(100);
}

void morse(const char *letter){
    int len = sizeof(letter)/sizeof(char);
    for (int i=0; i<(len-1); i++){
        if (letter[i] == '.'){
            Serial.print("\n.");
            short_led(12);
        } 
        else if (letter[i] == '-'){
            Serial.print("\n-");
            long_led(12);
        }
        else{
            Serial.print("\nError\nmorse symbol=");
            Serial.println(letter[i]);
        }
    } 
}

void morse_code(char* payload, unsigned int length){
    for (int i=0; i<length; i++){
        if (payload[i] <= 122 && payload[i] >= 107){
            morse(alpha_morse[payload[i]-97]);
        }
        else if (payload[i] <= 90 && payload[i] >= 65){
            morse(alpha_morse[payload[i]-65]);
        }
        else if (payload[i] <= 57 && payload[i] >= 48){
            morse(num_morse[payload[i]-48]);
        }
        else{
            Serial.print("\nError\nletter=");
            Serial.println(payload[i]);
        }
    }
    delay(3000);
}

void callback(char* topic,
 byte* payload, unsigned int length){
    payload[length] = 0;
    Serial.print("\nReceived messages: ");
    Serial.println(topic);
    for(int i=0; i<length; i++){
        Serial.print((char)payload[i]);
    }
    Serial.println();
    if (strcmp(topic, colorTopic) == 0){
        rgb_led((char*)payload);
    }
    else if (strcmp(topic, romainSwitchTopic) == 0){
        on_off_switch((char*)payload);
    }
    else if (strcmp(topic, intensityTopic) == 0){
        intensity_fun((char*)payload);
    }
    // else if (strcmp(topic, morseTopic) == 0){
    //     morse_code((char*)payload, length);
    // }
}

void setup(){
    Serial.begin(115200);
    setupWifi();
    pinMode(2, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(32, OUTPUT);
    pinMode(36, INPUT);

    client.setServer(broker, 1883);
    client.setCallback(callback);
}

void loop(){
    analogWrite(2,0);
    delay(100);
    analogWrite(2,255);
    delay(100);

    if(!client.connected()){
        reconnect();
        analogWrite(23,255);
    }
    else analogWrite(23,0);
    client.loop();

    sw = digitalRead(36);
    if(sw != last){
        if (sw){
            Serial.print("\nSwitch off");
            strcpy(val, "off");
        }
        else{
            Serial.print("\nSwitch on");
            strcpy(val,"on");
        }
        client.publish(romainTopic, val);
        client.publish(simonTopic, val);
        last = sw;
    }
    delay(500);
}