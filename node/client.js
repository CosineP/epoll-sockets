var net = require("net");

var client = new net.Socket();
client.connect(5507, "127.0.0.1", () => {
    client.write("you guys not know how to play tag?");
});

client.on("data", data => {
    console.log(`Echoed: ${data}`);
    client.destroy();
});

