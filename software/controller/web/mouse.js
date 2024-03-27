/**
 * Globals
 */
const server = "192.168.0.230"

var canvas, ctx, container;
var width, height, radius, x_orig, y_orig;
let coord = { x: 0, y: 0 };
let paint = false;


// Websocket connection
var connection = new WebSocket('ws://' + server + ':80/', ['ws']);
connection.onopen = function () {
    connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
    alert('WebSocket Error ', error);
};
connection.onmessage = function (e) {
    console.log('Server: ', e.data);
};
function send(x,y,speed,angle){
    var data = {"x":x,"y":y,"speed":speed,"angle":angle};
    data = JSON.stringify(data);
    console.log(data);
    // connection.send(data);   // FIXME: Put the candle back
}


window.addEventListener('load', () => {
    const streamElement = document.getElementById('videostream');
    streamElement.src = "http://"+server+"/video";
    canvas = document.getElementById('canvas');
    ctx = canvas.getContext('2d');          
    container = canvas.parentNode;
    if (container && container.nodeName === 'DIV') {
        resize(); 
    }
    document.addEventListener('mousedown', startDrawing);
    document.addEventListener('mouseup', stopDrawing);
    document.addEventListener('mousemove', Draw);

    document.addEventListener('touchstart', startDrawing);
    document.addEventListener('touchend', stopDrawing);
    document.addEventListener('touchcancel', stopDrawing);
    document.addEventListener('touchmove', Draw);
    window.addEventListener('resize', resize);

    document.getElementById("x_coordinate").innerText = 0;
    document.getElementById("y_coordinate").innerText = 0;
    document.getElementById("speed").innerText = 0;
    document.getElementById("angle").innerText = 0;
});

function resize() {
    radius = 200;
    width = container.offsetWidth-20;
    height = container.offsetHeight-40;
    ctx.canvas.width = width;
    ctx.canvas.height = height;
    console.log(width, height);
    background();
    joystick(width / 2, height / 2);
}
function background() {
    x_orig = width / 2;
    y_orig = height / 2;
    ctx.beginPath();
    ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2, true);
    ctx.fillStyle = '#ECE5E5';
    ctx.fill();
}
function joystick(width, height) {
    ctx.beginPath();
    ctx.arc(width, height, radius, 0, Math.PI * 2, true);
    ctx.fillStyle = '#F08080';
    ctx.fill();
    ctx.strokeStyle = '#F6ABAB';
    ctx.lineWidth = 8;
    ctx.stroke();
}
function getPosition(event) {
    if (event==undefined) return;
    var mouse_x = event.clientX || event.touches[0].clientX;
    var mouse_y = event.clientY || event.touches[0].clientY;
    coord.x = mouse_x - canvas.offsetLeft;
    coord.y = mouse_y - canvas.offsetTop;
}
function is_it_in_the_circle() {
    var current_radius = Math.sqrt(Math.pow(coord.x - x_orig, 2) + Math.pow(coord.y - y_orig, 2));
    if (radius >= current_radius) return true
    else return false
}
function startDrawing(event) {
    paint = true;
    getPosition(event);
    if (is_it_in_the_circle()) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        background();
        joystick(coord.x, coord.y);
        Draw();
    }
}
function stopDrawing() {
    paint = false;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    background();
    joystick(width / 2, height / 2);
    document.getElementById("x_coordinate").innerText = 0;
    document.getElementById("y_coordinate").innerText = 0;
    document.getElementById("speed").innerText = 0;
    document.getElementById("angle").innerText = 0;
    send(0, 0, 0, 0); // Sending back last event to stop every movement
}
function Draw(event) {
    if (paint) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        background();
        var angle_in_degrees,x, y, speed;
        var angle = Math.atan2((coord.y - y_orig), (coord.x - x_orig));
        if (Math.sign(angle) == -1) {
            angle_in_degrees = Math.round(-angle * 180 / Math.PI);
        }
        else {
            angle_in_degrees =Math.round( 360 - angle * 180 / Math.PI);
        }
        if (is_it_in_the_circle()) {
            joystick(coord.x, coord.y);
            x = coord.x;
            y = coord.y;
        } else {
            x = radius * Math.cos(angle) + x_orig;
            y = radius * Math.sin(angle) + y_orig;
            joystick(x, y);
        }
        getPosition(event);
        var speed =  Math.round(100 * Math.sqrt(Math.pow(x - x_orig, 2) + Math.pow(y - y_orig, 2)) / radius);
        var x_relative = Math.round(x - x_orig);
        var y_relative = Math.round(y - y_orig);
        
        document.getElementById("x_coordinate").innerText =  x_relative;
        document.getElementById("y_coordinate").innerText =y_relative ;
        document.getElementById("speed").innerText = speed;
        document.getElementById("angle").innerText = angle_in_degrees;
        send(x_relative, y_relative, speed, angle_in_degrees);
    }
} 
