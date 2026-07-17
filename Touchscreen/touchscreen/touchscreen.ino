#include <WiFi.h>
#include <WebServer.h>

// ---------- ESP32 Access Point ----------
const char* ssid = "ESP32_Touch";
const char* password = "12345678";

WebServer server(80);


// ---------- Touch ----------
const int TOUCH_PIN = 4;

int TOUCH_THRESHOLD = 10;

int rawTouch = 0;
bool currentTouchState = false;
bool lastTouchState = false;


// ---------- DAC ----------
const int DAC_PIN = 25;

float dacVoltage = 0.0;
float voltageTrim = 0.25;     // Default trim amount

int dacValue = 0;



// ---------- Gesture Timing ----------
const unsigned long DEBOUNCE_TIME = 30;
const unsigned long LONG_PRESS_TIME = 600;
const unsigned long DOUBLE_TAP_GAP = 300;


unsigned long touchStartTime = 0;
unsigned long touchEndTime = 0;

int tapCount = 0;
bool waitingForDoubleTap = false;

String lastGesture = "None";



// ---------- DAC Function ----------

void updateDAC()
{
  // Convert 0-3.3V to 0-255
  dacValue = (dacVoltage / 3.3) * 255.0;

  dacValue = constrain(dacValue,0,255);

  dacWrite(DAC_PIN,dacValue);
}



// ---------- Web Page ----------

String webpage()
{

return R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body {
font-family:Arial;
text-align:center;
margin-top:40px;
}

.value {
font-size:45px;
font-weight:bold;
}

input,button {
font-size:22px;
padding:8px;
}

</style>

</head>



<body>


<h1>ESP32 Touch DAC Controller</h1>


<p>Touch Reading</p>
<div class="value" id="raw">---</div>


<p>Threshold</p>

<input id="threshold" type="number" value="10">
<button onclick="setThreshold()">Set</button>


<p>Voltage Trim (V)</p>

<input id="trim" type="number" step="0.01" value="0.25">
<button onclick="setTrim()">Set</button>



<h2 id="voltage">Voltage: ---</h2>

<h2 id="state">---</h2>

<h2 id="gesture">---</h2>



<script>


function update()
{

fetch("/status")

.then(r=>r.json())

.then(data=>{


document.getElementById("raw").innerHTML=data.raw;


document.getElementById("state").innerHTML =
data.touch ? "TOUCHED":"RELEASED";


document.getElementById("gesture").innerHTML =
"Gesture: "+data.gesture;


document.getElementById("voltage").innerHTML =
"Voltage: "+data.voltage+" V";


if (document.activeElement.id !== "threshold")
{
  document.getElementById("threshold").value=data.threshold;
}


if (document.activeElement.id !== "trim")
{
  document.getElementById("trim").value=data.trim;
}


});

}




function setThreshold()
{
let val=document.getElementById("threshold").value;

fetch("/threshold?value="+val);
}



function setTrim()
{
let val=document.getElementById("trim").value;

fetch("/trim?value="+val);
}




setInterval(update,100);


</script>


</body>
</html>


)rawliteral";

}





// ---------- Web Handlers ----------


void root()
{
server.send(200,"text/html",webpage());
}



void status()
{

rawTouch = touchRead(TOUCH_PIN);


String json="{";

json += "\"raw\":"+String(rawTouch)+",";

json += "\"threshold\":"+String(TOUCH_THRESHOLD)+",";

json += "\"trim\":"+String(voltageTrim)+",";

json += "\"voltage\":"+String(dacVoltage,2)+",";

json += "\"touch\":"+String(currentTouchState)+",";

json += "\"gesture\":\""+lastGesture+"\"";


json+="}";


server.send(200,"application/json",json);

}




void threshold()
{

if(server.hasArg("value"))
{
TOUCH_THRESHOLD =
server.arg("value").toInt();
}

server.send(200,"text/plain","OK");

}




void trimAmount()
{

if(server.hasArg("value"))
{

voltageTrim =
server.arg("value").toFloat();


Serial.print("Voltage trim: ");
Serial.println(voltageTrim);

}


server.send(200,"text/plain","OK");

}





// ---------- Gesture Detection ----------


void detectGesture()
{

unsigned long now = millis();


rawTouch = touchRead(TOUCH_PIN);

currentTouchState =
rawTouch < TOUCH_THRESHOLD;




if(currentTouchState != lastTouchState)
{


delay(DEBOUNCE_TIME);



if(currentTouchState)
{

touchStartTime = now;

}



else
{

touchEndTime = now;


unsigned long duration =
touchEndTime-touchStartTime;



if(duration > DEBOUNCE_TIME)
{


if(duration >= LONG_PRESS_TIME)
{

lastGesture="LONG PRESS";

tapCount=0;

waitingForDoubleTap=false;

}



else
{

tapCount++;


if(!waitingForDoubleTap)
waitingForDoubleTap=true;


}


}


}



lastTouchState=currentTouchState;


}





if(waitingForDoubleTap && !currentTouchState)
{


if(now-touchEndTime > DOUBLE_TAP_GAP)
{


if(tapCount>=2)
{

lastGesture="DOUBLE TAP";


// Increase voltage
dacVoltage += voltageTrim;

if(dacVoltage > 3.3)
dacVoltage=3.3;

updateDAC();

}



else if(tapCount==1)
{

lastGesture="SHORT PRESS";


// Decrease voltage
dacVoltage -= voltageTrim;


if(dacVoltage < 0)
dacVoltage=0;


updateDAC();

}



tapCount=0;

waitingForDoubleTap=false;


}


}


}





// ---------- Setup ----------


void setup()
{


Serial.begin(115200);


// DAC starts at 0V

dacVoltage=0;

updateDAC();



WiFi.softAP(ssid,password);



Serial.println("ESP32 Access Point Started");

Serial.print("IP:");

Serial.println(WiFi.softAPIP());



server.on("/",root);

server.on("/status",status);

server.on("/threshold",threshold);

server.on("/trim",trimAmount);



server.begin();


}





// ---------- Loop ----------


void loop()
{

server.handleClient();

detectGesture();

}