const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const path = require('path');
const os = require('os');
const { GoogleGenerativeAI } = require("@google/generative-ai");

const API_KEY = process.env.GEMINI_API_KEY || "AIzaSyCqrC_jq2mSCnyCafg48lE-ybSdrz4LGBM";
const genAI = new GoogleGenerativeAI(API_KEY);
const model = genAI.getGenerativeModel({ model: "gemini-2.0-flash" }); 

const app = express();
const server = http.createServer(app);

// 1. Updated Socket.io with explicit CORS and transport settings
const io = socketIo(server, {
    cors: {
        origin: "*",
        methods: ["GET", "POST"]
    },
    transports: ['websocket', 'polling'] 
});

const PORT = process.env.PORT || 8000;

app.use(express.json());
// Ensure your index.html is inside a folder named 'public'
app.use(express.static(path.join(__dirname, 'public')));

// MONITOR BROWSER CONNECTIONS
io.on('connection', (socket) => {
    console.log(`\n✅ Browser Connected! ID: ${socket.id}`);
    
    socket.on('disconnect', () => {
        console.log(`❌ Browser Disconnected: ${socket.id}`);
    });
});

// API Endpoint for ESP32
app.post('/update', (req, res) => {
    const { seat_id, status, weight, dist } = req.body;
    
    // Log to terminal (Confirmed working on your end)
    console.log(`[ESP32] Seat ${seat_id}: ${status} | Weight: ${weight}kg | Dist: ${dist}cm`);
    
    // 2. BROADCAST to all connected browsers
    // We use io.sockets.emit to ensure it hits all active listeners
    io.sockets.emit('seatUpdate', {
        seat_id: seat_id,
        status: status,
        weight: weight || 0,
        dist: dist || 0,
        time: new Date().toLocaleTimeString()
    });

    res.status(200).send("Data Received");
});

// AI Analysis Endpoint
app.post('/analyze', async (req, res) => {
    try {
        const { dataSnapshot } = req.body;
        
        const prompt = `
            Analyze this real-time occupancy data from OccuTrack.io:
            ${JSON.stringify(dataSnapshot, null, 2)}
            
            Based on this data, provide:
            1. Occupancy Status: Summary of current usage.
            2. Anomalies: Detect anything unusual (e.g. single item on a station for a long time if history was provided, or high weight on "PARTIALLY" status).
            3. Recommendation: Optimization or safety tips.
            4. Prediction: Estimate peak usage based on current "PARTIALLY" vs "FULL" ratio.
            
            Format the response as clear, brief bullet points. Be professional and concise.
        `;

        const result = await model.generateContent(prompt);
        const response = await result.response;
        const text = response.text();
        
        res.json({ analysis: text });
    } catch (error) {
        console.error("AI Analysis Error:", error);
        res.status(500).json({ error: "Failed to generate AI insights" });
    }
});

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Listen on 0.0.0.0
server.listen(PORT, '0.0.0.0', () => {
    console.log(`\n🚀 LuminaDesk Server is LIVE!`);
    
    const interfaces = os.networkInterfaces();
    console.log(`\n--- ACCESS URLS ---`);
    console.log(`Local (This PC): http://localhost:${PORT}`);
    
    for (let interfaceName in interfaces) {
        for (let iface of interfaces[interfaceName]) {
            if (iface.family === 'IPv4' && !iface.internal) {
                console.log(`Network (ESP32 & Phone): http://${iface.address}:${PORT}`);
            }
        }
    }
    console.log(`\nWaiting for data...`);
});