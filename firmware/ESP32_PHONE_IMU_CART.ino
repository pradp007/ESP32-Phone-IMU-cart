#include <WiFi.h>
#include <WebServer.h>

// ================= WIFI =================
const char* ssid = "ESP_CART";
const char* password = "12345678";
WebServer server(80);

// ================= MOTOR =================
#define AIN1 26
#define AIN2 27
#define PWMA 25
#define BIN1 12
#define BIN2 13
#define PWMB 14

// ================= ULTRASONIC =================
#define TRIG 5
#define ECHO 18

// ================= BUZZER =================
#define BUZZER 32

// ================= GLOBALS =================

bool engineSoundDone = false;

float pitch=0, roll=0;
bool carEnabled=false;

int distance=999;

// ===== SPEED SMOOTHING =====
int currentSpeed = 0;
int targetSpeed = 0;

// ===== TIMERS =====
unsigned long lastSensor=0;
unsigned long lastBeep=0;
unsigned long lastRamp=0;

// ===== SOUND FLAGS =====
bool playEngineSound = false;
bool enginePlaying = false;
bool closeAlertActive = false;

// ================= ENGINE SOUND =================
void beep(int freq, int duration_ms) {
  int delayTime = 1000000 / freq / 2;
  int cycles = freq * duration_ms / 1000;

  for (int i = 0; i < cycles; i++) {
    digitalWrite(BUZZER, HIGH);
    delayMicroseconds(delayTime);
    digitalWrite(BUZZER, LOW);
    delayMicroseconds(delayTime);
  }
}

void engineStartSound() {
  for (int f = 1200; f <= 2800; f += 40) {
    beep(f, 12);
  }
  for (int i = 0; i < 15; i++) {
    beep(2600, 15);
    delay(10);
  }
  beep(3200, 100);
  delay(40);
  beep(3600, 120);
}

// ================= WEB UI (UNCHANGED) =================
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>

<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>ESP32 SMART DRIVE</title>

<style>
body{
  background:#0f0f14;
  font-family:Arial;
  color:white;
  text-align:center;
  margin:0;
}

h2{margin:12px 0;font-size:20px;letter-spacing:1px;}

.panel{
  background:#1b1b25;
  padding:20px;
  border-radius:20px;
  width:92%;
  max-width:400px;
  margin:auto;
  box-shadow:0 0 25px rgba(0,0,0,0.6);
}

#startStop{
  width:100%;
  padding:18px;
  font-size:18px;
  border:none;
  border-radius:14px;
  background:#00c853;
  color:white;
  font-weight:bold;
  margin-bottom:15px;
}

.controls{display:flex;gap:10px;margin-bottom:15px;}

button{
  flex:1;
  padding:16px;
  font-size:16px;
  border:none;
  border-radius:12px;
  font-weight:bold;
}

button:active{transform:scale(0.93);opacity:0.8;}

#brake{background:#ff0000;color:white;}
#horn{background:#ffd600;color:white;}

.data{font-size:16px;margin:10px 0;}

.distance{
  font-size:32px;
  font-weight:bold;
  color:#00e5ff;
  margin:10px 0;
}

.status{
  font-size:18px;
  font-weight:bold;
}
</style>
</head>

<body>

<h2>ESP32 SMART DRIVE</h2>

<div class="panel">

<button id="startStop" onclick="toggleCar()">START</button>

<div class="controls">
  <button id="brake" onclick="brake()">BRAKE</button>
  <button id="horn" onclick="horn()">HORN</button>
</div>

<div class="data">
Pitch: <span id="pitch">0</span><br>
Roll: <span id="roll">0</span>
</div>

<div class="distance" id="distance">-- cm</div>

<div class="status" id="distStatus">SAFE</div>

</div>

<script>

let running=false;

function toggleCar(){
  if(!running){
    fetch("/start");
    document.getElementById("startStop").innerText="STOP";
    document.getElementById("startStop").style.background="#d50000";
  }else{
    fetch("/stop");
    document.getElementById("startStop").innerText="START";
    document.getElementById("startStop").style.background="#00c853";
  }
  running=!running;
}

function brake(){ fetch("/brake"); }
function horn(){ fetch("/horn"); }

if(window.DeviceOrientationEvent){
  window.addEventListener('deviceorientation',function(event){
    let pitch=event.beta||0;
    let roll=event.gamma||0;

    document.getElementById("pitch").innerText=pitch.toFixed(1);
    document.getElementById("roll").innerText=roll.toFixed(1);

    fetch(`/imu?pitch=${pitch}&roll=${roll}`);
  });
}

setInterval(()=>{
  fetch("/distance")
  .then(res=>res.json())
  .then(data=>{
    document.getElementById("distance").innerText = data.distance + " cm";

    let status="SAFE";
    let color="#00e676";

    if(data.distance<5){
      status="STOP!";
      color="#ff1744";
    }
    else if(data.distance<20){
      status="WARNING";
      color="#ff9100";
    }

    document.getElementById("distStatus").innerText=status;
    document.getElementById("distStatus").style.color=color;
  });
},200);

</script>
</body>
</html>
)rawliteral";

// ================= SETUP =================
void setup(){

  pinMode(AIN1,OUTPUT); pinMode(AIN2,OUTPUT);
  pinMode(BIN1,OUTPUT); pinMode(BIN2,OUTPUT);
  pinMode(PWMA,OUTPUT); pinMode(PWMB,OUTPUT);

  pinMode(TRIG,OUTPUT);
  pinMode(ECHO,INPUT);
  pinMode(BUZZER,OUTPUT);

  WiFi.softAP(ssid,password);

  server.on("/", [](){ server.send_P(200,"text/html",webpage); });

  server.on("/imu", [](){
    if(!carEnabled) return server.send(200,"text/plain","OFF");
    pitch=server.arg("pitch").toFloat();
    roll=server.arg("roll").toFloat();
    updateMotors();
    server.send(200,"text/plain","OK");
  });

  server.on("/distance", [](){
    server.send(200,"application/json","{\"distance\":"+String(distance)+"}");
  });

  // ===== In /start handler =====
server.on("/start", [](){
  carEnabled=true;
  playEngineSound=true;
  engineSoundDone=false;  // ADD THIS LINE
  server.send(200,"text/plain","STARTED");
});


  server.on("/stop", [](){
    carEnabled=false;
    applyBrake();
    noTone(BUZZER);
    server.send(200,"text/plain","STOPPED");
  });

  server.on("/brake", [](){
    applyBrake();
    server.send(200,"text/plain","BRAKE");
  });

  server.on("/horn", [](){
    if(!enginePlaying) tone(BUZZER,800,200);
    server.send(200,"text/plain","HORN");
  });

  server.begin();
}

// ================= LOOP =================
void loop(){

  server.handleClient();

  // ===== ENGINE SOUND SAFE =====
  // ===== In loop(), modify the ENGINE SOUND section =====
if(playEngineSound){
  enginePlaying = true;
  engineStartSound();
  enginePlaying = false;
  playEngineSound = false;
  engineSoundDone = true;  // ADD THIS LINE
}


  // ===== SENSOR =====
  // ===== In loop(), modify the SENSOR section =====
// ===== SENSOR =====
if(engineSoundDone && millis()-lastSensor>80){  // ADD engineSoundDone check
  lastSensor=millis();
  distance=getDistance();
}


  if(!carEnabled) return;

  // ===== OBSTACLE FIX (SAFE ESCAPE) =====
  if(distance < 5){

    if(!closeAlertActive){
      if(!enginePlaying) tone(BUZZER,3000);
      closeAlertActive = true;
    }

    // SAFE REVERSE via normal motor system
    targetSpeed = -120;
    return;

  } else {

    if(closeAlertActive){
      noTone(BUZZER);
      closeAlertActive = false;
    }
  }

  // ===== WARNING =====
  if(distance<20 && millis()-lastBeep>150){
    lastBeep=millis();
    if(!enginePlaying)
      tone(BUZZER,map(distance,5,40,3000,1200),80);
  }

  // ===== SPEED RAMP =====
  if(millis()-lastRamp>20){
    lastRamp=millis();

    if(currentSpeed < targetSpeed) currentSpeed+=5;
    else if(currentSpeed > targetSpeed) currentSpeed-=10;
  }
}

// ================= MOTOR =================
void updateMotors(){

  int threshold=8;

  if(abs(pitch)<threshold){
    targetSpeed=0;
    applyBrake();
    return;
  }

  targetSpeed = map(abs(pitch),0,60,120,255);

  int speed = currentSpeed;
  bool forward = pitch > 0;

  int turn = map(abs(roll),0,45,0,255);

  int left=speed, right=speed;

  if(abs(roll)>threshold){
    if(roll>0){
      right -= turn;
      if(turn>150) right = -speed;
    }else{
      left -= turn;
      if(turn>150) left = -speed;
    }
  }

  auto drive=[&](int in1,int in2,int val){
    if(val>=0){
      digitalWrite(in1,HIGH);
      digitalWrite(in2,LOW);
    }else{
      digitalWrite(in1,LOW);
      digitalWrite(in2,HIGH);
      val=-val;
    }
    return constrain(val,0,255);
  };

  int l = drive(AIN1,AIN2, forward?left:-left);
  int r = drive(BIN1,BIN2, forward?right:-right);

  analogWrite(PWMA,l);
  analogWrite(PWMB,r);
}

// ================= BRAKE =================
void applyBrake(){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,HIGH);

  analogWrite(PWMA,255);
  analogWrite(PWMB,255);

  currentSpeed=0;
  targetSpeed=0;
}

// ================= ULTRASONIC =================
int getDistance(){

  digitalWrite(TRIG,LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG,LOW);

  long d=pulseIn(ECHO,HIGH,20000);
  if(d==0) return 999;

  int cm=d*0.034/2;
  return (cm>400||cm<=0)?999:cm;
}
