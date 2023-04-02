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
<input type="range" min="-180" max="180" value="0"   step="1" class="slider-color" oninput="send(3, this.value,80,this)" onChange="send(3, this.value,0,this)" />
<input type="range" min="0"    max="360" value="240" step="1" class="slider-color" oninput="send(2, this.value,80,this)" onChange="send(2, this.value,0,this)" />
<br>
  <div id='container'>
    <div id='item'> </div>
  </div>
</div>

<script>
function send(id,value,min_time_transmit,elem) {
    var now = new Date().getTime();
    if (elem.sendTimeout)
    {
       clearTimeout(elem.sendTimeout);
       elem.sendTimeout = null;
    }
    if (ws.readyState !== WebSocket.OPEN) {
      return;
    }
    if(elem.lastSend === undefined || now - elem.lastSend >= min_time_transmit) {
        if (ws.bufferedAmount>0)
        {
          elem.lastId = id;
          elem.lastValue = value;
          elem.sendTimeout = setTimeout(function send_trafficjam() {
            elem.sendTimeout = null;
            send(elem.lastId,elem.lastValue,min_time_transmit,elem);
          }, min_time_transmit);
        }
        else
        {
          try {
            ws.send(id+':'+value);
            elem.lastSend = new Date().getTime();
            return;
          } catch(e) {
            console.log(e);
          }
        }
    }
    else
    {
        elem.lastValue = value;
        elem.lastId = id;
        var ms = elem.lastSend !== undefined ? min_time_transmit - (now - elem.lastSend) : min_time_transmit;
        if(ms < 0)
            ms = 0;
        elem.sendTimeout = setTimeout(function send_waittransmit() {
            elem.sendTimeout = null;
            send(elem.lastId,elem.lastValue,min_time_transmit,elem);
        }, ms);
    }
}

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
var joystick = document.querySelector('#container');

// currentX, currentY, touchid, initialX, initialY
joystick.active = false;
joystick.autocenter = true;
joystick.xOffset = 0;
joystick.yOffset = 0;

joystick.dragStart = function (e) {
  if (e.target === dragItem) {
    if (e.type === 'touchstart') {
        this.touchid = e.changedTouches[0].identifier;
        this.initialX = e.changedTouches[0].clientX - this.xOffset;
        this.initialY = e.changedTouches[0].clientY - this.yOffset;
    } else {
        this.initialX = e.clientX - this.xOffset;
        this.initialY = e.clientY - this.yOffset;
    }
    this.active = true;
  }
}

joystick.dragEnd = function(e) {
    if (e.target === dragItem) {
      if (this.autocenter)
      {
            this.currentX=0; this.currentY=0;
            this.xOffset =0; this.yOffset =0;
      }
      this.initialX = this.currentX;
      this.initialY = this.currentY;
      this.active = false;
      this.setTranslate(this.currentX, this.currentY, dragItem);
    }
}

joystick.drag = function (e) {
    if (this.active) {
        e.preventDefault();
        if (e.type === 'touchmove') {
          for (var i=0; i<e.changedTouches.length; i++) {
              var id = e.changedTouches[i].identifier;
              if (id == this.touchid) {
                this.currentX = e.changedTouches[i].clientX - this.initialX;
                this.currentY = e.changedTouches[i].clientY - this.initialY;
              }
          }  
        } else {
            this.currentX = e.clientX - this.initialX;
            this.currentY = e.clientY - this.initialY;
        }
        if (this.currentY >= (this.offsetHeight / joystickfactor))  {
            this.currentY = this.offsetHeight / joystickfactor;
        }
        if (this.currentY <= (-this.offsetHeight / joystickfactor))  {
            this.currentY = -this.offsetHeight / joystickfactor;
        }
        if (this.currentX >= (this.offsetWidth / joystickfactor))  {
            this.currentX = this.offsetWidth / joystickfactor;
        }
        if (this.currentX <= (-this.offsetWidth / joystickfactor))  {
            this.currentX = -this.offsetWidth / joystickfactor;
        }
        this.xOffset = this.currentX;
        this.yOffset = this.currentY;
        this.setTranslate(this.currentX, this.currentY, dragItem);
    }
}

function setTranslate(xPos, yPos, el) {
    var transformstr = 'translate(' + xPos + 'px, ' + yPos + 'px)';
    el.style.transform = transformstr;
    el.style.webkitTransform = transformstr;
    var xval = xPos * 180 / (this.offsetWidth / joystickfactor);
    var yval = yPos * 180 / (this.offsetHeight / joystickfactor);
    send('1',Math.round(xval) + ',' + Math.round(yval),80,this);
}

joystick.addEventListener('touchstart', function(event) { this.dragStart(event); }.bind(joystick), false);
joystick.addEventListener('touchend',   function(event) { this.dragEnd(event);   }.bind(joystick), false);
joystick.addEventListener('touchmove',  function(event) { this.drag(event);      }.bind(joystick), false);
joystick.addEventListener('mousedown',  function(event) { this.dragStart(event); }.bind(joystick), false);
joystick.addEventListener('mouseup',    function(event) { this.dragEnd(event);   }.bind(joystick), false);
joystick.addEventListener('mousemove',  function(event) { this.drag(event);      }.bind(joystick), false);

</script>
</body>
</html>

)=====";
