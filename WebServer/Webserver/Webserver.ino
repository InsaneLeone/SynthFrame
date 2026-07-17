#include <WiFi.h>
#include <WebServer.h>

// ---------- ESP32 Access Point ----------
const char* ssid = "ESP32_Touch";
const char* password = "12345678";   // minimum 8 characters

WebServer server(80);


// ---------- Touch ----------
const int TOUCH_PIN = 4;
int TOUCH_THRESHOLD = 10;

int rawTouch = 0;
bool touched = false;


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
  font-family: Arial;
  text-align:center;
  margin-top:40px;
}

.value {
  font-size:50px;
  font-weight:bold;
}

input,button {
  font-size:22px;
  padding:8px;
}

</style>

</head>

<body>

<h1>ESP32 Touch Monitor</h1>

<p>Touch Reading</p>
<div class="value" id="raw">---</div>

<p>State:</p>
<h2 id="state">---</h2>


<p>
Threshold:
<input id="threshold" type="number" value="10">
<button onclick="setThreshold()">Set</button>
</p>


<script>

function update()
{
 fetch('/value')
 .then(r=>r.json())
 .then(data=>{

 document.getElementById("raw").innerHTML=data.raw;

 if(data.touch)
   document.getElementById("state").innerHTML="TOUCHED";
 else
   document.getElementById("state").innerHTML="RELEASED";

 });
}


function setThreshold()
{
 let val=document.getElementById("threshold").value;

 fetch('/threshold?value='+val);
}


setInterval(update,200);

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


void value()
{
  rawTouch = touchRead(TOUCH_PIN);

  touched = rawTouch < TOUCH_THRESHOLD;


  String json="{";
  json += "\"raw\":"+String(rawTouch)+",";
  json += "\"touch\":"+String(touched);
  json += "}";


  server.send(200,"application/json",json);
}



void threshold()
{
  if(server.hasArg("value"))
  {
    TOUCH_THRESHOLD = server.arg("value").toInt();

    Serial.print("Threshold changed to: ");
    Serial.println(TOUCH_THRESHOLD);
  }

  server.send(200,"text/plain","OK");
}



// ---------- Setup ----------

void setup()
{
  Serial.begin(115200);


  // Create WiFi network
  WiFi.softAP(ssid,password);


  Serial.println();
  Serial.println("ESP32 Access Point Started");

  Serial.print("Network: ");
  Serial.println(ssid);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());


  server.on("/",root);
  server.on("/value",value);
  server.on("/threshold",threshold);


  server.begin();

  Serial.println("Web server running");
}



// ---------- Loop ----------

void loop()
{
  server.handleClient();
}