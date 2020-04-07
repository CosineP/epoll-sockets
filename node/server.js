// https://gist.github.com/tedmiston/5935757

var net = require("net");

var server = net.createServer(socket => {
    socket.pipe(socket);
});

server.listen(5507, "0.0.0.0");
