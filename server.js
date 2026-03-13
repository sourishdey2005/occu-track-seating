const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');
const os = require('os');

const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const app = express();
const server = http.createServer(app);

const io = socketIo(server, {
    cors: {
        origin: "*",
        methods: ["GET", "POST"]
    },
    transports: ['websocket', 'polling']
});

const PORT = process.env.PORT || 8000;

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));


// ===============================
// BROWSER CONNECTION MONITOR
// ===============================

io.on('connection', (socket) => {

    console.log(`✅ Browser Connected: ${socket.id}`);

    socket.on('disconnect', () => {
        console.log(`❌ Browser Disconnected: ${socket.id}`);
    });

});


// ===============================
// ESP32 WIFI DATA ENDPOINT
// ===============================

app.post('/update', (req, res) => {

    const { seat_id, status, weight, dist } = req.body;

    console.log(
        `[WiFi] Seat ${seat_id} | ${status} | Weight: ${weight}kg | Dist: ${dist}cm`
    );

    io.emit('seatUpdate', {
        seat_id: seat_id,
        status: status,
        weight: weight || 0,
        dist: dist || 0,
        time: new Date().toLocaleTimeString()
    });

    res.status(200).send("Data Received");

});


// ===============================
// USB SERIAL DATA (ESP32 CABLE)
// ===============================

try {

    const port = new SerialPort({
        path: "COM5",      // CHANGE THIS TO YOUR ESP32 PORT
        baudRate: 115200
    });

    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

    console.log("🔌 USB Serial listener started");

    parser.on('data', (line) => {

        console.log("[USB]", line);

        /*
        Expected format from ESP32:

        W: 2.34 kg | D: 45.1 cm | PARTIALLY
        */

        const match = line.match(/W:\s*([\d\.]+).*D:\s*([\d\.]+).*?\|\s*(\w+)/);

        if (match) {

            const weight = parseFloat(match[1]);
            const dist = parseFloat(match[2]);
            const status = match[3];

            io.emit('seatUpdate', {
                seat_id: 101,
                status: status,
                weight: weight,
                dist: dist,
                time: new Date().toLocaleTimeString()
            });

        }

    });

} catch (error) {

    console.log("⚠️ Serial not connected yet");

}


// ===============================
// SERVE DASHBOARD
// ===============================

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});


// ===============================
// START SERVER
// ===============================

server.listen(PORT, '0.0.0.0', () => {

    console.log(`\n🚀 LuminaDesk Server Running`);

    const interfaces = os.networkInterfaces();

    console.log(`\n--- ACCESS URLS ---`);

    console.log(`Local PC:`);
    console.log(`http://localhost:${PORT}`);

    for (let name in interfaces) {
        for (let iface of interfaces[name]) {

            if (iface.family === 'IPv4' && !iface.internal) {

                console.log(`Network (ESP32 / Phone):`);
                console.log(`http://${iface.address}:${PORT}`);

            }
        }
    }

    console.log(`\nWaiting for ESP32 data...\n`);

});