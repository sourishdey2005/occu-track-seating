# Step-by-Step Local Setup Guide

Follow these steps to connect your ESP32 to any WiFi and send data to your local machine.

## Step 1: Find Your Laptop's IP Address
1.  Press `Win + R`, type `cmd`, and press **Enter**.
2.  Type `ipconfig` and press **Enter**.
3.  Look for the **IPv4 Address** under your active connection (usually Wi-Fi).
    - It should look like `192.168.x.x` (e.g., `192.168.1.5`).
    - **Write this down!**

## Step 2: Flash the ESP32
1.  In VS Code, go to `arduino/mimi_v2_advanced.ino`.
2.  Make sure you have the following libraries installed in the Arduino IDE:
    - `WiFiManager` by tablatronix
    - `HX711` by Bogdan Necula
3.  In the Arduino IDE:
    - Go to **Tools > Board > ESP32 > ESP32 Dev Module**.
    - Go to **Tools > Upload Speed** and set it to `115200` (safer for CP2102).
4.  Connect your ESP32 to your laptop.
4.  Compile and Upload the code.

## Step 3: Configure WiFi and Server IP
1.  Once flashed, the ESP32 will start and (if it can't find a network) create its own WiFi hotspot.
2.  On your phone or laptop, look for a WiFi network named **OccuTrack_Setup**.
3.  Connect to it. A configuration page should open automatically.
    - If it doesn't, open a web browser and go to `192.168.4.1`.
4.  Click **Configure WiFi**.
5.  Select your home/office WiFi and enter its password.
6.  In the **Server IP** field, enter the IP address you wrote down in Step 1.
7.  Click **Save**. The ESP32 will reboot and connect to your WiFi.

## Step 4: Run the Local Server
1.  On your laptop, make sure the server is running.
2.  In the VS Code terminal, run:
    ```bash
    node server.js
    ```
3.  Open your browser and go to `http://localhost:8000`.

## Step 5: Verify Connection
1.  Watch the VS Code terminal. You should see incoming data like:
    `[ESP32] Seat 101: EMPTY | Weight: 0.00kg | Dist: 120.0cm`
2.  Refresh your dashboard at `http://localhost:8000` to see the real-time updates!

---

> [!TIP]
> **Important**: Both your ESP32 and your laptop MUST be connected to the SAME WiFi network for this to work.
