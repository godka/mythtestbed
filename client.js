require('process');

var PORT = 23583;
var HOST = '166.111.138.139=';

var dgram = require('dgram');
var chars = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'];
//fixed random
function randomint(start,end){
    delta = end - start - 1;
    return Math.floor(start + Math.random() * delta);
}

function generateMixed(n) {
    var res = "";
    for (var i = 0; i < n; i++) {
        var id = randomint(800,1500);
        res += chars[id];
    }
    return res;
}

function generatejson() {
    var res = { time: Math.floor(process.uptime() * 1000), values: generateMixed(Math.floor(Math.random() * 1000)) };
    return JSON.stringify(res);
}

var client = dgram.createSocket('udp4');
function sendData(message, callback) {
    client.send(message, 0, message.length, PORT, HOST, function (err, bytes) {
        return callback(err, bytes)
    });
}
var static_fps = 30;

setInterval(() => {
    var times = randomint(17,34);
    for (var i = 0; i < times; i++) {
        msg = generatejson();
        sendData(msg, (err, bytes) => {
            if (err) {
                console.log(err);
            }
        });
    }
}, 1000 / static_fps);
