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
 #item:active {
     background-color: rgba(168, 218, 220, 1.00);
}
 #item:hover {
     cursor: pointer;
     border-width: 20px;
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
<input type="range" min="-180" max="180" value="0" step="1" class="slider-color" oninput="showValue(3,this.value)" />
<input type="range" min="0" max="360" value="240" step="1" class="slider-color" oninput="showValue(2,this.value)" />
<br>
  <div id='container'>
    <div id='item'> </div>
  </div>
</div>

<script>
const WS_URL = "ws://" + window.location.host + ":82";
const ws = new WebSocket(WS_URL);
const joystickfactor = 2.8;
    
var dragItem = document.querySelector('#item');
var container = document.querySelector('#container');
var active = false;
var autocenter = true;
var currentX;
var currentY;
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
    if (e.type === 'touchstart') {
        initialX = e.touches[0].clientX - xOffset;
        initialY = e.touches[0].clientY - yOffset;
    } else {
        initialX = e.clientX - xOffset;
        initialY = e.clientY - yOffset;
    }
    if (e.target === dragItem) {
        active = true;
    }
}

function dragEnd(e) {
    if (e.type === 'touchend') {
        if (e.touches.length >0) return; 
    }
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

function drag(e) {
    if (active) {
        e.preventDefault();
        if (e.type === 'touchmove') {
            currentX = e.touches[0].clientX - initialX;
            currentY = e.touches[0].clientY - initialY;
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

// limit sending to one message every 100 ms
// https://github.com/neonious/lowjs_esp32_examples/blob/master/neonious_one/cellphone_controlled_rc_car/www/index.html
function send(txt) {
    var now = new Date().getTime();

    if (sendTimeout)
    {
       clearTimeout(sendTimeout);
       sendTimeout = null;
    }
    if(lastSend === undefined || now - lastSend >= 100) {
        try {
            ws.send(txt);
            lastSend = new Date().getTime();
            return;
        } catch(e) {
            console.log(e);
        }
    }
    else
    {
        lastText = txt;
        var ms = lastSend !== undefined ? 100 - (now - lastSend) : 100;
        if(ms < 0)
            ms = 0;
        sendTimeout = setTimeout(() => {
            sendTimeout = null;
            send(lastText);
        }, ms);
    }
}

function setTranslate(xPos, yPos, el) {
    el.style.transform = 'translate3d(' + xPos + 'px, ' + yPos + 'px, 0)';
    var xval = xPos * 180 / (container.offsetWidth / joystickfactor);
    var yval = yPos * 180 / (container.offsetHeight / joystickfactor);
  send('1:'+Math.round(xval) + ',' + Math.round(yval));
}

function showValue(id,v) {
  send(id+':'+v+',0');
}

</script>
</body>
</html>

)=====";
