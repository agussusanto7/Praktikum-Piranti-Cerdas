# Smart Greenhouse IoT - Flow Documentation

## Gambaran Arsitektur Sistem

```
┌─────────────────┐         ┌─────────────────┐         ┌─────────────────┐
│                 │         │                 │         │                 │
│   ESP32         │◄───────►│   WEB SERVER    │◄───────►│   BROWSER       │
│   (Hardware)    │  HTTP   │  (PHP + MySQL)  │  HTTP   │   (Dashboard)   │
│                 │         │                 │         │                 │
└─────────────────┘         └─────────────────┘         └─────────────────┘
```

---

## 1. LOKASI PERANGKAT IoT (Hardware)

### ESP32 Pinout - Wiring Diagram

| Komponen | Pin ESP32 | Keterangan |
|----------|-----------|------------|
| **DHT11** | GPIO 26 | Sensor Suhu & Kelembaban |
| **Servo SG90** | GPIO 27 | Kontrol Jendela (0°=Tutup, 60°=Buka) |
| **DC Fan** | GPIO 32 | Kipas Pendingin (LOW=Nyala, HIGH=Mati) |
| **Buzzer** | GPIO 25 | Alarm Suhu Tinggi |
| **LCD I2C SDA** | GPIO 21 | Data LCD |
| **LCD I2C SCL** | GPIO 22 | Clock LCD |

**Power:**
- DHT11: 3.3V
- Servo: 5V (External power recommended)
- Fan: 5V
- Buzzer: 3.3V/5V
- LCD: 5V

---

## 2. FLOW KETIKA USER MENGETUK DI WEB

### 2.1 User Klik Tombol Kontrol Jendela

```
USER CLICK WEB (index.php)
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  JavaScript: controlWindow(1) atau controlWindow(0)             │
│  File: assets/css/js/main.js (line 236-258)                    │
└─────────────────────────────────────────────────────────────────┘
    │
    │ 1. Cek mode (harus MANUAL)
    │ 2. Buat FormData dengan action='window' dan value=(1/0)
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  HTTP POST ke: api/control.php                                  │
│  File: api/control.php (line 35-47)                            │
└─────────────────────────────────────────────────────────────────┘
    │
    │ 1. Terima action='window'
    │ 2. Update database: UPDATE controls SET window_command = ?
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  DATABASE: Table 'controls'                                     │
│  ┌─────────────┬──────────────────┬─────────────────┐          │
│  │ id=1        │ window_command   │ 0 atau 1        │          │
│  └─────────────┴──────────────────┴─────────────────┘          │
└─────────────────────────────────────────────────────────────────┘
    │
    ▼
  Tunggu ESP32 polling (maksimal 3 detik)
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  ESP32 Loop() - sendDataToServer()                              │
│  File: esp32baru.md (ESP32 Code line 169-226)                   │
│  Dipanggi setiap 3 detik (SEND_INTERVAL = 3000ms)               │
└─────────────────────────────────────────────────────────────────┘
    │
    │ 1. HTTP GET ke: api/save-data.php
    │ 2. Kirim data: temp, hum, window, fan, alarm, mode
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  PHP: api/save-data.php (line 27)                              │
│  Query: SELECT * FROM controls WHERE id = 1                     │
└─────────────────────────────────────────────────────────────────┘
    │
    │ Response JSON:
    │ {
    │   "status": "success",
    │   "controls": {
    │     "window_cmd": 1,  ← Perintah buka jendela
    │     "fan_cmd": 0,
    │     "mode": "manual",
    │     "threshold": 30.0
    │   }
    │ }
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  ESP32: Parse JSON response                                     │
│  File: esp32baru.md (line 198-218)                             │
│  webWindowCmd = controls["window_cmd"]                          │
└─────────────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  ESP32: manualControl()                                         │
│  File: esp32baru.md (line 248-256)                             │
│                                                                 │
│  if (webWindowCmd == 1 && !windowOpen) openWindow();           │
│  else if (webWindowCmd == 0 && windowOpen) closeWindow();      │
└─────────────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│  ESP32: openWindow() atau closeWindow()                         │
│  File: esp32baru.md (line 258-276)                             │
│                                                                 │
│  openWindow():                                                  │
│    myServo.attach(SERVO_PIN);                                   │
│    myServo.write(SERVO_OPEN);  // 60 derajat                   │
│    windowOpen = true;                                           │
│                                                                 │
│  closeWindow():                                                 │
│    myServo.write(SERVO_CLOSE); // 0 derajat                    │
│    windowOpen = false;                                          │
└─────────────────────────────────────────────────────────────────┘
    │
    ▼
  JENDELA FISIK BERGERAK! (Servo motor berputar)
```

---

### 2.2 User Klik Tombol Kontrol Kipas

Flow sama seperti jendela, hanya beda endpoint:

```
controlFan(1/0) di main.js (line 261-283)
    │
    ▼
api/control.php?action=fan&value=1 (line 50-62)
    │
    ▼
UPDATE controls SET fan_command = ?
    │
    ▼
ESP32 baca webFanCmd dari save-data.php response
    │
    ▼
manualControl():
  if (webFanCmd == 1 && !fanActive) turnFanOn();
  else if (webFanCmd == 0 && fanActive) turnFanOff();
    │
    ▼
turnFanOn(): digitalWrite(FAN_PIN, LOW);
turnFanOff(): digitalWrite(FAN_PIN, HIGH);
    │
    ▼
KIPAS NYALA/MATI!
```

---

### 2.3 User Ganti Mode (AUTO/MANUAL)

```
setMode('manual') di main.js (line 209-233)
    │
    ▼
api/control.php?action=mode&value=manual (line 15-32)
    │
    ▼
UPDATE controls SET mode = 'manual'
    │
    │ Jika mode='auto', reset command:
    │ UPDATE controls SET window_command = 0, fan_command = 0
    ▼
ESP32 baca currentMode dari response
    │
    ▼
loop():
  if (currentMode == "auto") autoControl();
  else manualControl();
```

---

### 2.4 User Set Threshold Suhu

```
setThreshold() di main.js (line 286-306)
    │
    ▼
api/control.php?action=threshold&value=30 (line 65-77)
    │
    ▼
UPDATE controls SET temp_threshold = 30.0
    │
    ▼
ESP32 baca TEMP_THRESHOLD dari response
    │
    ▼
autoControl():
  if (currentTemp > TEMP_THRESHOLD) {
    alarmTriggered = true;
    openWindow();
    turnFanOn();
  }
```

---

## 3. FLOW DARI ESP32 KE WEB

### 3.1 ESP32 Kirim Data Sensor

```
ESP32 Loop() - setiap 100ms
    │
    ├─► readSensors() - Baca DHT11
    │   currentTemp = dht.readTemperature();
    │   currentHum = dht.readHumidity();
    │
    ├─► sendDataToServer() - Setiap 3 detik
    │   │
    │   └─► HTTP GET ke api/save-data.php
    │       ?temp=28.5&hum=65&window=0&fan=1&alarm=0&mode=auto
    │
    ▼
api/save-data.php (line 19-24)
    │
    └─► INSERT INTO sensor_data
        (temperature, humidity, window_status, fan_status,
         alarm_status, mode)
        VALUES (?, ?, ?, ?, ?, ?)
    │
    └─► SELECT * FROM controls WHERE id = 1
    │
    └─► Response JSON dengan controls
```

### 3.2 Web Update Tampilan

```
Browser - setInterval(updateData, 2000)
    │ Setiap 2 detik
    ▼
updateData() di main.js (line 86-165)
    │
    └─► HTTP GET ke api/get-data.php
    │
    ▼
api/get-data.php (line 10-26)
    │
    ├─► SELECT * FROM sensor_data ORDER BY id DESC LIMIT 30
    │
    └─► SELECT * FROM controls WHERE id = 1
    │
    ▼
Response JSON: { latest, sensor_data[], controls }
    │
    ▼
JavaScript update UI:
    ├─► document.getElementById('tempValue').textContent = ...
    ├─► document.getElementById('humValue').textContent = ...
    ├─► document.getElementById('windowValue').textContent = ...
    ├─► document.getElementById('fanValue').textContent = ...
    ├─► Update Chart.js grafik
    └─► Update tabel data
```

---

## 4. FLOW MODE OTOMATIS

```
ESP32 Loop() - mode = "auto"
    │
    ▼
autoControl() di esp32baru.md (line 228-246)
    │
    ├─► if (currentTemp > TEMP_THRESHOLD)
    │   │
    │   ├─► alarmTriggered = true
    │   ├─► openWindow()
    │   └─► turnFanOn()
    │
    └─► else (currentTemp <= TEMP_THRESHOLD)
        │
        ├─► alarmTriggered = false
        ├─► closeWindow()
        └─► turnFanOff()
    │
    ▼
handleBuzzerAlarm()
    │
    └─► if (alarmTriggered)
        └─► Buzzer blink tiap 500ms
```

---

## 5. SUMMARY API ENDPOINTS

| Method | Endpoint | Dari | Ke | Fungsi |
|--------|----------|------|----|----|
| GET/POST | `/api/save-data.php` | ESP32 | PHP | Kirim sensor data, terima command |
| GET | `/api/get-data.php` | Browser | PHP | Ambil data untuk dashboard |
| POST | `/api/control.php` | Browser | PHP | Simpan command kontrol |

---

## 6. DATABASE SCHEMA

### Table: sensor_data
```sql
CREATE TABLE sensor_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    temperature FLOAT,
    humidity FLOAT,
    window_status INT,        -- 0=Tutup, 1=Buka
    fan_status INT,           -- 0=Mati, 1=Nyala
    alarm_status INT,         -- 0=Normal, 1=Alert
    mode VARCHAR(10),         -- 'auto' atau 'manual'
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Table: controls
```sql
CREATE TABLE controls (
    id INT PRIMARY KEY,
    mode VARCHAR(10),         -- 'auto' atau 'manual'
    window_command INT,       -- 0=Tutup, 1=Buka
    fan_command INT,          -- 0=Mati, 1=Nyala
    temp_threshold FLOAT      -- Default 30.0
);

-- Data awal:
INSERT INTO controls VALUES (1, 'auto', 0, 0, 30.0);
```

---

## 7. TIMING DIAGRAM

```
Time:    0s    2s    3s    4s    6s    9s
         │     │     │     │     │     │
Browser: ●─────●─────●─────●─────●─────●   <-- GET get-data.php (2s interval)
         │     │     │     │     │     │
ESP32:    ●─────────●─────────●─────────●   <-- GET save-data.php (3s interval)
         │         │         │         │
         └─► Kirim sensor & terima command
```

---

## 8. CATATAN PENTING

### Logika Status Jendela (PERHATIAN!)
| Variabel ESP32 | Kirim ke Server | Tampilan di Web | Kondisi Fisik | Servo |
|----------------|-----------------|-----------------|---------------|-------|
| `windowOpen = true` | 1 | TERTUTUP | Jendela TERBUKA | 60° |
| `windowOpen = false` | 0 | TERBUKA | Jendela TERTUTUP | 0° |

**Ini diperbaiki di esp32baru.md line 181:**
```cpp
url += "&window=" + String(windowOpen ? 1 : 0);
```

### Command Manual
| Command | Fungsi |
|---------|--------|
| `webWindowCmd = 1` | Buka jendela (servo ke 60°) |
| `webWindowCmd = 0` | Tutup jendela (servo ke 0°) |
| `webFanCmd = 1` | Nyala kipas (GPIO LOW) |
| `webFanCmd = 0` | Mati kipas (GPIO HIGH) |

---

## 9. FILE STRUCTURE

```
UAPIOT5/
├── index.php                  # Halaman dashboard web
├── db.php                     # Koneksi database
├── esp32baru.md               # Kode ESP32 Arduino
├── flow.md                    # Dokumentasi flow (ini)
├── api/
│   ├── save-data.php          # Terima data dari ESP32
│   ├── get-data.php           # Kirim data ke web
│   └── control.php            # Terima command dari web
└── assets/
    └── css/
        ├── style.css          # Styles
        └── js/
            └── main.js        # JavaScript frontend
```

---

**Generated for Smart Greenhouse IoT System**
