# Expo Mimi - OccuTrack

LuminaDesk/OccuTrack is a real-time occupancy tracking system with AI insights.

## Project Structure

- `server.js`: Node.js server with Socket.io and Gemini AI integration.
- `server.py`: FastAPI server for occupancy simulation.
- `public/`: Frontend assets (HTML/CSS/JS).
- `models.json`: Data for AI analysis.

## How to Run

### 1. Node.js Server (Recommended)
This server handles real-time updates via Socket.io and AI analysis using Gemini.

```bash
# Install dependencies
npm install

# Start the server
node server.js
```
Access the dashboard at `http://localhost:8000`.

### 2. Python Server (FastAPI)
Used for simulation of seat statuses.

```bash
# Install dependencies
pip install fastapi uvicorn python-multipart

# Start the server
uvicorn server:app --reload
```
The server will be available at `http://localhost:8000`.

## Features
- Real-time seat occupancy updates via ESP32 API or simulation.
- AI-powered insights for occupancy optimization.
- Live dashboard with visual seat mapping.
