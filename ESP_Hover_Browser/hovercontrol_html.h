const char index_html[] PROGMEM = R"=====(
<html>
<head>
<meta name='viewport'         content='width=device-width,         initial-scale=1.0,         user-scalable=no' />
<title>HoverControl</title>

<style>
#outerContainer {
  width: 80%;
  margin: auto;
}
</style>


<style> 
#container {
     width: 100%;
     height: 65vh;
     background-color: #333;
     display: flex;
     align-items: center;
     justify-content: center;
     overflow: hidden;
     border-radius: 7px;
     touch-action: none;
}
 #item {
     width: 100px;
     height: 100px;
     background-color: rgb(245, 230, 99);
     border: 10px solid rgba(136, 136, 136, .5);
     border-radius: 50%;
     touch-action: none;
     user-select: none;
}
 #item:hover {
     cursor: pointer;
     border-width: 20px;
}
 #item:active {
     background-color: rgba(168, 218, 220, 1.00);
}
</style>
<style>
.slider-color {
  -webkit-appearance: none;
  width: 100%;
  height: 20px;
  margin-top: 10px;
  margin-bottom: 15px;
  border-radius: 5px;
  background: #d3d3d3;
  outline: none;
  opacity:0.7;
  -webkit-transition: opacity .15s ease-in-out;
  transition: opacity .15s ease-in-out;
}
.slider-color:hover {
  opacity:1;
}
.slider-color::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 40px;
  height: 40px;
  border-radius: 50%;
  background: #4CAF50;
  cursor: pointer;
}
.slider-color::-moz-range-thumb {
  width: 40px;
  height: 40px;
  border: 0;
  border-radius: 50%;
  background: #4CAF50;
  cursor: pointer;
}


</style>
</head>
<body>
<div id='outerContainer'>
<span id="connectiondisplay">Trying to connect</span>
<input type="range" min="-180" max="180" value="0" step="1" class="slider-color" oninput="showValue(3,this.value)" />
<input type="range" min="0" max="360" value="240" step="1" class="slider-color" oninput="showValue(2,this.value)" />
<br>
  <div id='container'>
    <div id='item'> </div>
  </div>
</div>

<script>
var retransmitInterval;
const connectiondisplay= document.getElementById('connectiondisplay');
const WS_URL = "ws://" + window.location.host + ":82";
var ws;

function connect_ws() {
  ws = new WebSocket(WS_URL);
  
  ws.onopen = function() {
    connectiondisplay.textContent = "Connected";
    retransmitInterval=setInterval(function ws_onopen_ping() {
      if (ws.bufferedAmount == 0)
      {
        ws.send("0");
      }
    }, 1000);
  };

  ws.onclose = function() {
    if (checkConnectionInterval)
    {
      connectiondisplay.textContent = "Disconnected";
    }
    else
    {
      connectiondisplay.textContent = "Disconnected. Another client is active, refresh to continue";
    }

    if (retransmitInterval)    
    {        
      clearInterval(retransmitInterval);        
      retransmitInterval = null;     
    }
  };

  ws.onmessage = function (message) {
    if (typeof message.data === "string") {
      if (message.data === "CLOSE")
      {
        if (checkConnectionInterval)
        {        
          clearInterval(checkConnectionInterval);
          checkConnectionInterval= null;     
        }
      }
      else
      {
        connectiondisplay.textContent = message.data;
      }
    }
  };
}

connect_ws();

var checkConnectionInterval = setInterval(function check_connection_interval() {
  if (ws.readyState == WebSocket.CLOSED) {
    connectiondisplay.textContent = "Reconnecting ...";
    connect_ws();
  }
}, 5000);

const joystickfactor = 2.8;
    
var dragItem = document.querySelector('#item');
var container = document.querySelector('#container');
var active = false;
var autocenter = true;
var currentX;
var currentY;
var touchid;
var initialX;
var initialY;
var xOffset = 0;
var yOffset = 0;
var lastText, lastSend, sendTimeout;
container.addEventListener('touchstart', dragStart, false);
container.addEventListener('touchend', dragEnd, false);
container.addEventListener('touchmove', drag, false);
container.addEventListener('mousedown', dragStart, false);
container.addEventListener('mouseup', dragEnd, false);
container.addEventListener('mousemove', drag, false);

function dragStart(e) {
  if (e.target === dragItem) {
    if (e.type === 'touchstart') {
        touchid = e.changedTouches[0].identifier;
        initialX = e.changedTouches[0].clientX - xOffset;
        initialY = e.changedTouches[0].clientY - yOffset;
    } else {
        initialX = e.clientX - xOffset;
        initialY = e.clientY - yOffset;
    }
    active = true;
  }
}

function dragEnd(e) {
    if (e.target === dragItem) {
      if (autocenter)
      {
            currentX=0; currentY=0;
            xOffset =0; yOffset =0;
      }
      initialX = currentX;
      initialY = currentY;
      active = false;
      setTranslate(currentX, currentY, dragItem);
    }
}

function drag(e) {
    if (active) {
        e.preventDefault();
        if (e.type === 'touchmove') {
          for (var i=0; i<e.changedTouches.length; i++) {
              var id = e.changedTouches[i].identifier;
              if (id == touchid) {
                currentX = e.changedTouches[i].clientX - initialX;
                currentY = e.changedTouches[i].clientY - initialY;
              }
          }  
        } else {
            currentX = e.clientX - initialX;
            currentY = e.clientY - initialY;
        }
        if (currentY >= (container.offsetHeight / joystickfactor))  {
            currentY = container.offsetHeight / joystickfactor;
        }
        if (currentY <= (-container.offsetHeight / joystickfactor))  {
            currentY = -container.offsetHeight / joystickfactor;
        }
        if (currentX >= (container.offsetWidth / joystickfactor))  {
            currentX = container.offsetWidth / joystickfactor;
        }
        if (currentX <= (-container.offsetWidth / joystickfactor))  {
            currentX = -container.offsetWidth / joystickfactor;
        }
        xOffset = currentX;
        yOffset = currentY;
        setTranslate(currentX, currentY, dragItem);
    }
}

function send(txt) {
    const min_time_transmit = 80;
    var now = new Date().getTime();
    if (sendTimeout)
    {
       clearTimeout(sendTimeout);
       sendTimeout = null;
    }
    if (ws.readyState !== WebSocket.OPEN) {
      return;
    }
    if(lastSend === undefined || now - lastSend >= min_time_transmit) {
        if (ws.bufferedAmount>0)
        {
          lastText = txt;
          sendTimeout = setTimeout(function send_trafficjam() {
            sendTimeout = null;
            send(lastText);
          }, min_time_transmit);
        }
        else
        {
          try {
            ws.send(txt);
            lastSend = new Date().getTime();
            return;
          } catch(e) {
            console.log(e);
          }
        }
    }
    else
    {
        lastText = txt;
        var ms = lastSend !== undefined ? min_time_transmit - (now - lastSend) : min_time_transmit;
        if(ms < 0)
            ms = 0;
        sendTimeout = setTimeout(function send_waittransmit() {
            sendTimeout = null;
            send(lastText);
        }, ms);
    }
}

function setTranslate(xPos, yPos, el) {
    var transformstr = 'translate(' + xPos + 'px, ' + yPos + 'px)';
    el.style.transform = transformstr;
    el.style.webkitTransform = transformstr;
    var xval = xPos * 180 / (container.offsetWidth / joystickfactor);
    var yval = yPos * 180 / (container.offsetHeight / joystickfactor);
  send('1:'+Math.round(xval) + ',' + Math.round(yval));
}

function showValue(id,v) {
  if (ws.readyState !== WebSocket.OPEN) {
    return;
  }
  ws.send(id+':'+v+',0');
}

</script>
</body>
</html>

)=====";
