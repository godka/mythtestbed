require('process');

var PORT = 23583;
var HOST = '127.0.0.1';

var dgram = require('dgram');
var chars = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'];

function generateMixed(n) {
    var res = "";
    for (var i = 0; i < n; i++) {
        var id = Math.ceil(Math.random() * 35);
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
    var times = Math.floor(Math.random() * 50);
    for (var i = 0; i < times; i++) {
        msg = generatejson();
        sendData(msg, (err, bytes) => {
            if (err) {
                console.log(err);
            }
        });
    }
}, 1000 / static_fps);
