# 🚗 U Cab – Smart IoT-Based Car Rental System

**U Cab** is an intelligent, real-time vehicle rental control and monitoring system. It combines embedded systems, telematics, and cloud integration to automate the rental process, enforce mileage-based access control, and ensure vehicle safety. Designed for rental agencies and individual vehicle owners, this device transforms any vehicle into a smart, self-managing rental unit.

---

## 🔧 Project Overview

The U Cab system is composed of a custom hardware module and two mobile applications (Owner and Customer apps). Once installed in a vehicle, it connects to the ECU via **OBD2 CAN bus**, collects critical data like engine status, speed, and mileage, and uploads it to a remote server using **GSM (SIM800L)**.

Customers can book mileage packages via the app, and once their mileage limit is exhausted, the car is automatically disabled using a **power relay**. Owners can track vehicle status, location, and remotely control the vehicle from anywhere.

---

## 🧠 Key Features

| Feature                     | Description                                                                 |
|----------------------------|-----------------------------------------------------------------------------|
| 🔐 Mileage Control         | Customers pay for a specific distance; car auto-disables after limit.       |
| 📡 Real-Time Monitoring    | Speed, location, engine status, and mileage are viewable live.              |
| 🛑 Remote Shutdown         | Vehicle can be shut down manually by the owner via app.                     |
| 🧰 Vehicle Diagnostics      | Reads fault codes (DTCs) and other OBD2 data for remote maintenance.         |
| 🌐 Cloud Data Sync         | Sends data to a server using HTTP POST via the SIM800L GSM module.          |
| 📲 Dual App System         | Mobile apps for both customer and owner interfaces.                         |

---

## 🧩 Hardware Components

| Component        | Purpose                                                              |
|------------------|----------------------------------------------------------------------|
| **ATmega328P**   | Collects vehicle data from CAN bus, manages mileage limit logic.     |
| **ESP32**        | Aggregates data, manages SIM800L and GPS modules, acts as central MCU|
| **SIM800L**      | Sends data to cloud server via GSM (HTTP POST).                      |
| **Neo-8M GPS**   | Tracks vehicle location in real-time.                                |
| **MCP2515 CAN**  | Communicates with vehicle ECU over OBD2 CAN protocol.                |
| **Power Relay**  | Physically controls ignition to enable/disable the car.              |

---

## 🗺️ Use Case: Car Rental Process

1. **Owner installs** the U Cab device in the vehicle via the OBD2 port.
2. **Customer books** mileage via the mobile app and makes a payment.
3. **Device starts tracking** vehicle metrics in real-time (mileage, speed, engine status).
4. When the **purchased mileage limit is reached**, the car is **automatically disabled**.
5. **Owner app** receives full status updates, live GPS location, and diagnostic data.
6. The **vehicle can be re-enabled** manually or by purchasing additional mileage.

---

## 🔌 System Architecture

```plaintext
[Vehicle ECU]
   ↓ (CAN Bus via MCP2515)
[ATmega328P] → Logic Control, Distance Monitoring
   ↓
[ESP32]
   ├── SIM800L (HTTP POST to Cloud)
   ├── Neo-8M GPS
   └── Relay Trigger (Car Disable)
