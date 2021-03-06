const char index_html[] PROGMEM = R"rawliteral(
<html>
    <title>
        Smart Thermometer
    </title>
    <head>
<style>
* { box-sizing: border-box; }

body { font-family: sans-serif; position: relative; margin: 0; }

.nav {
    overflow: hidden;
    background-color:rgb(214, 70, 70);
    color: #fff;
  }

h1 {
  text-align: center;
    font-size: 1.8rem;
    color: white;
}

  h2{
    text-align: center;
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }

p{
  text-align: center;
}

.content {
    padding: 30px;
    max-width: 370px;
    max-height: 600px;
    margin: 0 auto;
    padding-left: 50px;
  }
  .card {
    background-color: #F8F7F9;
    border-radius: 40px;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }

  .btn2 {
  -webkit-border-radius: 8;
  -moz-border-radius: 8;
  align-self: center;
  border-radius: 8px;
  font-family: Arial;
  color: #ffffff;
  font-size: 20px;
  background: #de5b5b;
  padding: 10px 20px 10px 20px;
  text-decoration: none;
}

.btn2:hover {
  background: #ff0000;
  text-decoration: none;
}

 .btn {
  -webkit-border-radius: 8;
  -moz-border-radius: 8;
  align-self: center;
  border-radius: 8px;
  font-family: Arial;
  color: #ffffff;
  font-size: 20px;
  background: #3498db;
  padding: 10px 20px 10px 20px;
  text-decoration: none;
}

.btn:hover {
  background: #83bee6;
  text-decoration: none;
}

.thermometer_container {
  width: 80px;
  height: 309px;
  display: block;
  position: relative;
  margin: 50px auto;
}

.thermometer {
  border: 5px solid #5C525F;
  border-bottom: transparent;
  box-sizing: border-box;
  display: block;
  position: absolute;
  z-index: 1;
  left: 20px;
  width: 40px;
  height: 238px;
  border-radius: 50px 50px 0 0;
  background: #FFF;
}

.base {
  box-shadow: 0 0 0 5px #FFF, 0 0 0 10px #5C525F; 
  background: rgb(214, 70, 70);
  display: block;
  text-align: center;
  font-size: 14px;
  line-height: 60px;
  color: #FFF;
  position: absolute;
  bottom: 10px;
  left: 10px;
  width: 60px;
  height: 60px;
  border-radius: 60px;
}

.stem {
  background: #EFEFEF;
  position: relative;
  display: block;
  width: 20px;
  height: 232px;
  border-radius: 10px 10px 0 0;
  margin: 5px auto;
}

.stem .fluid {
  background: rgb(214, 70, 70);;
  display: block;
  width: 20%;
  height: 0%;
  border-radius: 25px 25px 0 0;
  position: absolute;
  bottom: 0;
}
    </style>
    </head>
    <body>
      <div class="nav">
        <h1>THERMOMETER</h1>
      </div>
      <div class="content">
      <div class="card">
        <h2>Real-time data</h2>
        <p><span style="color: red;">Temperature</span> : <span style="padding-right:10px;" class="tempData">0</span>
      </div>
      <div class="content">
        <button class="btn" id="button" onclick="takeReading()">
          Start
        </button>
        <button class="btn2" id="button2" onclick="deepSleep()">
          Sleep
        </button>
      </div>
      <div class="thermometer_container">
        <div class="thermometer">
          <div class="stem">
            <div class="fluid"></div>
          </div>
        </div>
        <div class="base">0</div>
      </div>

    </body>
    <script>
      var gateway = `ws://${window.location.hostname}/ws`;
      var websocket;
      var currentClass = "";
      var soundControl = false;
      var cube = document.querySelector('.cube');
      
      window.addEventListener('load', onLoad);
      
        function initWebSocket() {
          console.log('Trying to open a WebSocket connection...');
          websocket = new WebSocket(gateway);
          websocket.onopen    = onOpen;
          websocket.onclose   = onClose;
          websocket.onmessage = onMessage; // <-- add this line
        }
        function onOpen(event) {
          console.log('Connection opened');
        }
        function onClose(event) {
          console.log('Connection closed');
          setTimeout(initWebSocket, 2000);
        }
        function onMessage(event) {

          var str = event.data;
          if(str === "measuring")
          {
            document.getElementById('button').innerHTML = "wait..";
            return;
          }
          if(str === "stop")
          {
            document.getElementById('button').innerHTML = "Start";
            return;
          }
          document.querySelector('.tempData').innerHTML = str;
          document.querySelector('.fluid').style.height = str+"%";
          document.querySelector('.base').innerHTML = str+" C";
        }
        
        function onLoad(event) {
          initWebSocket();
        }  

        function takeReading(){
        websocket.send('start');
        }

        function deepSleep(){
        websocket.send('sleep');
        window.close();
        }
   </script>
</html>
)rawliteral";

void notifyClients(String data) {
  ws.textAll(data);
  #if DEBUG
  Serial.println("sending data to server :"+data);
  #endif
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
      if (strcmp((char*)data, "start") == 0) {
      StartReadingTemp = true;
      notifyClients("measuring");
        #if DEBUG
        Serial.println("receiving data from server :");
        Serial.println(*data);
        #endif
    }
    else if(strcmp((char*)data, "sleep") == 0)
    {
      deepSleepMCU();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      WebSocketConnected = true;
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      WebSocketConnected = false;
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var) {
  return var;
}

void StartServer() {
  // Route for root / web page

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}