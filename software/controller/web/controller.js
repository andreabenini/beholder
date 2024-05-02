// Module attributes
let wsHandle;
let width, height, xOrigin, yOrigin;
let oldX, oldY, oldSpeed, oldAngle
let oldLeft  = 0;                           // Previous motors dutycycle
let oldRight = 0;
let canvas, ctx, container;
let coord = { x: 0, y: 0 };
let paint = false;                          // flag: painting on screen (true|false)

const CURSORSTEP = 20;

// Cursor keys detection
document.addEventListener("keydown", function(event) { 
    switch (event.key) {
        case "ArrowUp":
            if (coord.y < RADIUS) {
                coord.y += CURSORSTEP;
            }
            cursorUpdate();
            break;
        case "ArrowDown":
            if (coord.y > -RADIUS) {
                coord.y -= CURSORSTEP;
            }
            cursorUpdate();
            break;
        case "ArrowLeft":
            if (coord.x > -RADIUS) {
                coord.x -= CURSORSTEP;
            }
            cursorUpdate();
            break;
        case "ArrowRight":
            if (coord.x < RADIUS) {
                coord.x += CURSORSTEP;
            }
            cursorUpdate();
            break;
    }
});

function cursorUpdate() {
    // console.log(coord);
    // startDrawingCursor();
}


// Websocket connection (input controls)
function initWebSocket() {
    wsHandle = new WebSocket('ws://' + SERVER + '/ws');
    wsHandle.onclose = function (event) {
        console.log("WebSocket connection closed, reconnecting...");
        wsHandle = null;
        setTimeout(initWebSocket, 5000);
        setTimeout(videoConnect,  5000);
    }; /**/
    wsHandle.onerror = function (error) {
        console.log('WebSocket Error ', error);
        alert('WebSocket Error ', error);
    }; /**/
    wsHandle.onmessage = function (e) {
        console.log('<', e.data);
        wsHandle.Locked = false;
    }; /**/
    wsHandle.onopen = function () {
        if (wsHandle==null) {
            setTimeout(initWebSocket, 5000);
            return;
        }
        wsHandle.Locked = true;
        console.log("WebSockets Connected");
        wsHandle.send("status");
    }; /**/
} /**/
function videoConnect() {
    // Video stream
    let streamElement = document.getElementById('videostream');
    streamElement.src = "http://" + SERVER + "/video";
    console.log("Video stream connected");
    // Refresh button
    let buttonrefresh = document.getElementById('videorefresh');
    if (buttonrefresh && buttonrefresh.addEventListener) {
        buttonrefresh.removeEventListener("click", function() {}, false); // Remove all click listeners
        buttonrefresh.addEventListener("click", function() {
            setTimeout(videoConnect, 5000);
            console.log("Reconnecting video...");
        }, false);
    }
} /**/
function socketConnect() {
    // Socket button
    let buttonSocket = document.getElementById('connect');
    if (buttonSocket && buttonSocket.addEventListener) {
        buttonSocket.removeEventListener("click", function() {}, false); // Remove all click listeners
        buttonSocket.addEventListener("click", function() {
            initWebSocket();
            videoConnect();
        }, false);
    }
    initWebSocket();
    videoConnect();
}

/**
 * Send data to the robot through WebSocket interface
 * @param {int} x cursor position
 * @param {int} y cursor position
 * @param {int} speed cursor (range: 0..100)
 * @returns (void) It might be aborted if mutex is locked (@see notes)
 * 
 * @see Avoid using wsHandle.send() directly, use this function instead.
 *      Mutex response is properly handled with [wsHandle.Locked]
 */
function command(x, y, speed) {
    wsHandle.Locked = true;
    speed = progressiveSpeed(speed);        // Speed range: 0..1
    let [left, right] = controlRobot(x, y, speed);
    if (left!=oldLeft || right!=oldRight) { // Same speed as before? Do not send it again
        console.log(`> ${x},${y} [${speed}]    Motors: `+String([left, right]));
        wsHandle.send("m "+left+" "+right);
        oldLeft  = left;
        oldRight = right;
    } else {
        wsHandle.Locked = false;
    }
}; /**/


/**
 * Alter speed with linear interpolation from the joystick bot control
 * @param {int} speed Real speed detected from virtual joystick
 */
function progressiveSpeed(speed) {
    switch (true) {
        case (speed==0):     // 0
            break;
        case (speed<25):    // 1..24    ->  1..50
            speed =  1 + ((speed-1 ) * 49 / 23);
            break;
        case (speed<50):    // 25..49   -> 50..75
            speed = 50 + ((speed-25) * 25 / 24);
            break;
        case (speed<75):    // 50..74   -> 75..90
            speed = 75 + ((speed-50) * 15 / 24);
            break;
        default:            // 75..100  -> 90..100  ((speed-75) * (100-90) / (100-75))
            speed = 90 + ((speed-75) * 10 / 25);
            break;
    }
    return (speed/100).toFixed(2);      // Adapt range in the [0..1] scale
} /**/


/**
 * Map the values onto the defined range (minJoystick, maxJoystick, minSpeed, maxSpeed) -> RADIUS,THROTTLE
 * @returns {int} [speed] adapted to min..max RADIUS
 */
function controlMap(speed) {
    if (speed < -RADIUS) {      // Check that the value is at least -RADIUS
        speed = -RADIUS;
    }
    if (speed > RADIUS) {       // Check that the value is at most RADIUS
        speed = RADIUS;
    }
    return Math.floor((speed - (-RADIUS)) * (THROTTLE - (-THROTTLE)) / (RADIUS - (-RADIUS)) + (-THROTTLE));
} /**/


/**
 * Control robot motors for a two wheeled drivetrain (differential drive)
 * Using just one joystick for the whole robot (driving them independently on two different axes is too easy)
 * @param   {int} x axis position (X)
 * @param   {int} y axis position (Y)
 * @param   {float} speed (0..1)
 * @returns {int, int} [left, right] Duty cycle to apply on left and right motor
 */
function controlRobot(x, y, speed) {
    let leftSpeed, rightSpeed;
    let z   = Math.sqrt(x*x + y*y);                     // Compute the angle in deg, first hypotenuse
    let rad = Math.acos(Math.abs(x)/z);                 // Angle in radians
    if (isNaN(rad)) rad=0;
    let angle = rad * 180 / Math.PI;                    // Angle in degrees
    /* [angle] indicates the measure of turn.
       Along a straight line, with an angle o, the turn co-efficient is same.
       This applies for angles between 0-90, with angle 0 the coeff is -1,
       with angle 45, the co-efficient is 0 and with angle 90, it is 1.     */
    let tcoeff = -1 + (angle / 90) * 2;
    let turn   = tcoeff * Math.abs(Math.abs(y) - Math.abs(x));
    turn = Math.round(turn * 100, 0) / 100;
    let movement = Math.max(Math.abs(y), Math.abs(x));  // And max of [y] or [x] is the [movement]
    if ((x>=0 && y>=0) || (x<0 && y<0)) {               // First and third quadrant
        leftSpeed  = movement;
        rightSpeed = turn;
    } else {                                            // Second and fourth quadrant
        rightSpeed = movement;
        leftSpeed  = turn;
    }
    if (y < 0) {                                        // Reverse polarity
        leftSpeed  = 0 - leftSpeed;
        rightSpeed = 0 - rightSpeed;
    }
    // Adapt motor vector force to current [speed]
    rightSpeed *= speed;
    leftSpeed  *= speed;
    // Map the values onto the defined range (RADIUS, THROTTLE)
    rightSpeed = controlMap(rightSpeed);
    leftSpeed  = controlMap(leftSpeed);
    return [leftSpeed, rightSpeed];
} /**/


//
function send(x, y, speed, angle) {
    if (wsHandle==null) return;
    if (x < oldX - CURSORTOLERANCE || x > oldX + CURSORTOLERANCE ||
        y < oldY - CURSORTOLERANCE || y > oldY + CURSORTOLERANCE ||
        speed < oldSpeed - CURSORTOLERANCE || speed > oldSpeed + CURSORTOLERANCE ||
        angle < oldAngle - CURSORTOLERANCE || angle > oldAngle + CURSORTOLERANCE) {
            
            updateCoordinates(x, y, speed, angle);
            if (!wsHandle.Locked || (x===y && y===0)) {
                command(x, y, speed);
            }      
    }
}; /**/

// Update coordinates on screen in dedicated fields
function updateCoordinates(xRelative, yRelative, speed, angle) {
    document.getElementById("xCoordinate").innerText = xRelative;
    document.getElementById("yCoordinate").innerText = yRelative;
    document.getElementById("speed").innerText = speed;
    document.getElementById("angle").innerText = angle;
    oldX = xRelative;
    oldY = yRelative;
    oldSpeed = speed;
    oldAngle = angle;
}; /**/

// Init on page load()
window.addEventListener('load', () => {
    canvas = document.getElementById('canvas');
    ctx = canvas.getContext('2d');
    container = canvas.parentNode;
    if (container && container.nodeName === 'DIV') {
        resize();
    }
    socketConnect();
    document.addEventListener('mousedown',   startDrawingMouse);
    document.addEventListener('mouseup',     stopDrawing);
    document.addEventListener('mousemove',   Draw);
    document.addEventListener('touchstart',  startDrawingMouse);
    document.addEventListener('touchend',    stopDrawing);
    document.addEventListener('touchcancel', stopDrawing);
    document.addEventListener('touchmove',   Draw);
    window.addEventListener('resize', resize);
    updateCoordinates(0, 0, 0, 0);
});

// Resize joystick movement on window resize
function resize() {
    width = container.offsetWidth - 20;
    height = container.offsetHeight - 40;
    ctx.canvas.width = width;
    ctx.canvas.height = height;
    console.log("Joystick area size: "+String([width, height]));
    background();
    joystick(width / 2, height / 2);
}

function joystickSize() {
    let size = RADIUS+20;
    if (size*2>width) {
        size = width/3 - 20;
    }
    if (size*2>height) {
        size = height/3 - 20;
    }
    return size;
}
// Draw the whole background and the central joystick position
function background() {
    xOrigin = width / 2;
    yOrigin = height / 2;
    ctx.beginPath();
    ctx.arc(xOrigin, yOrigin, joystickSize(), 0, Math.PI * 2, true);
    ctx.fillStyle = CURSORCOLORSHADOW;
    ctx.fill();
}
// Draw current joystick position
function joystick(width, height) {
    ctx.beginPath();
    ctx.arc(width, height, joystickSize(), 0, Math.PI * 2, true);
    ctx.fillStyle = CURSORCOLORINNER;
    ctx.fill();
    ctx.strokeStyle = CURSORCOLOROUTER;
    ctx.lineWidth = 8;
    ctx.stroke();
}
// Get current joystick position
function getPosition(event) {
    if (event == undefined) return;
    let mouse_x = event.clientX || event.touches[0].clientX;
    let mouse_y = event.clientY || event.touches[0].clientY;
    coord.x = mouse_x - canvas.offsetLeft;
    coord.y = mouse_y - canvas.offsetTop;
}
// As it said, detect if it's in the circle or not
function isItInTheCircle() {
    let current_radius = Math.sqrt(Math.pow(coord.x - xOrigin, 2) + Math.pow(coord.y - yOrigin, 2));
    if (RADIUS >= current_radius) return true
    else return false
} /**/
function startDrawingMouse(event) {
    paint = true;
    getPosition(event);
    if (isItInTheCircle()) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        background();
        joystick(coord.x, coord.y);
        Draw();
    }
} /**/
function startDrawingCursor() {
    paint = true;
    if (isItInTheCircle()) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        background();
        joystick(coord.x, coord.y);
        Draw();
    }
} /**/
function stopDrawing() {
    paint = false;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    background();
    joystick(width / 2, height / 2);
    send(0, 0, 0, 0);           // Sending back last event to stop every movement
}
function Draw(event) {
    if (paint) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        background();
        let angleDegree, x, y, speed;
        let angle = Math.atan2((coord.y - yOrigin), (coord.x - xOrigin));
        if (Math.sign(angle) == -1) {
            angleDegree = Math.round(-angle * 180 / Math.PI);
        }
        else {
            angleDegree = Math.round(360 - angle * 180 / Math.PI);
        }
        if (isItInTheCircle()) {
            joystick(coord.x, coord.y);
            x = coord.x;
            y = coord.y;
        } else {
            x = RADIUS * Math.cos(angle) + xOrigin;
            y = RADIUS * Math.sin(angle) + yOrigin;
            joystick(x, y);
        }
        getPosition(event);
        speed = Math.round(100 * Math.sqrt(Math.pow(x - xOrigin, 2) + Math.pow(y - yOrigin, 2)) / RADIUS);
        let xRelative = Math.round(x - xOrigin);
        let yRelative = Math.round(y - yOrigin) * -1;
        send(xRelative, yRelative, speed, angleDegree);
    }
} /**/




/// TODO: WTF is this ? Do I need to keep it ?

var bodyTag;
var executionTime = new Date().getTime(); // JavaScript execution time

// Document ready
//----------------
documentReady(function() {
	// Don't hide address bar after a distracting amount of time
	var readyTime = new Date().getTime()
	if((readyTime - executionTime) < 3000) hideAddressBar(true);

});

// Run specified function when document is ready (HTML5)
//------------------------------------------------------
function documentReady(readyFunction) {
	document.addEventListener('DOMContentLoaded', function() {
		document.removeEventListener('DOMContentLoaded', arguments.callee, false);
		readyFunction();

	}, false);

}

// Hide address bar on devices like the iPhone
//---------------------------------------------
function hideAddressBar(bPad) {
	// Big screen. Fixed chrome likely.
	if(screen.width > 980 || screen.height > 980) return;

	// Standalone (full screen webapp) mode
	if(window.navigator.standalone === true) return;

	// Page zoom or vertical scrollbars
	if(window.innerWidth !== document.documentElement.clientWidth) {
		// Sometimes one pixel too much. Compensate.
		if((window.innerWidth - 1) !== document.documentElement.clientWidth) return;

	}

	// Pad content if necessary.
	if(bPad === true && (document.documentElement.scrollHeight <= document.documentElement.clientHeight)) {
		// Extend body height to overflow and cause scrolling
		bodyTag = document.getElementsByTagName('body')[0];

		// Viewport height at fullscreen
		bodyTag.style.height = document.documentElement.clientWidth / screen.width * screen.height + 'px';

	}

	setTimeout(function() {
		// Already scrolled?
		if(window.pageYOffset !== 0) return;

		// Perform autoscroll
		window.scrollTo(0, 1);

		// Reset body height and scroll
		if(bodyTag !== undefined) bodyTag.style.height = window.innerHeight + 'px';
		window.scrollTo(0, 0);

	}, 1000);

}

// Quick address bar hide on devices like the iPhone
//---------------------------------------------------
function quickHideAddressBar() {
	setTimeout(function() {
		if(window.pageYOffset !== 0) return;
		window.scrollTo(0, window.pageYOffset + 1);

	}, 1000);

}
