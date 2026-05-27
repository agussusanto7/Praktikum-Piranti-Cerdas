<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="Smart Greenhouse IoT - Sistem monitoring dan kontrol greenhouse otomatis">
    <title>🌱 Smart Greenhouse IoT - Dashboard</title>
    <link rel="stylesheet" href="assets/css/style.css">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <div class="container">
        <h1><i class="fas fa-seedling"></i> Smart Greenhouse IoT</h1>

        <!-- Sensor Cards -->
        <div class="sensor-grid">
            <div class="sensor-card temp-card">
                <div class="sensor-icon">
                    <i class="fas fa-thermometer-half"></i>
                </div>
                <h3>Suhu</h3>
                <div class="sensor-value" id="tempValue">--</div>
                <p>°Celsius</p>
                <div class="threshold-info">Threshold: <span id="thresholdValue">30</span>°C</div>
            </div>
            
            <div class="sensor-card hum-card">
                <div class="sensor-icon">
                    <i class="fas fa-tint"></i>
                </div>
                <h3>Kelembaban</h3>
                <div class="sensor-value" id="humValue">--</div>
                <p>% RH</p>
            </div>
            
            <div class="sensor-card window-card">
                <div class="sensor-icon">
                    <i class="fas fa-window-maximize" id="windowIcon"></i>
                </div>
                <h3>Jendela</h3>
                <div class="sensor-value" id="windowValue">--</div>
                <p>Status Ventilasi</p>
            </div>
            
            <div class="sensor-card fan-card">
                <div class="sensor-icon">
                    <i class="fas fa-fan" id="fanIcon"></i>
                </div>
                <h3>Kipas</h3>
                <div class="sensor-value" id="fanValue">--</div>
                <p>Status Pendingin</p>
            </div>
        </div>

        <!-- Alarm Status -->
        <div class="alarm-container" id="alarmContainer">
            <div class="alarm-box" id="alarmBox">
                <i class="fas fa-check-circle"></i>
                <span id="alarmText">Sistem Normal</span>
            </div>
        </div>

        <!-- Chart Section -->
        <div class="card">
            <h2><i class="fas fa-chart-line"></i> Grafik Sensor Real-Time</h2>
            <div class="chart-container">
                <canvas id="sensorChart"></canvas>
            </div>
        </div>

        <!-- Control Panel -->
        <div class="card">
            <h2>
                <i class="fas fa-gamepad"></i> Panel Kontrol
                <span class="mode-badge" id="modeBadge">AUTO</span>
            </h2>
            <div class="control-panel">
                <!-- Mode Control -->
                <div class="btn-group">
                    <h3><i class="fas fa-robot"></i> Mode Operasi</h3>
                    <button class="btn-success" onclick="setMode('auto')" id="btnAuto">
                        <i class="fas fa-magic"></i> OTOMATIS
                    </button>
                    <button class="btn-warning" onclick="setMode('manual')" id="btnManual">
                        <i class="fas fa-hand-paper"></i> MANUAL
                    </button>
                </div>
                
                <!-- Window Control -->
                <div class="btn-group">
                    <h3><i class="fas fa-window-restore"></i> Kontrol Jendela</h3>
                    <button class="btn-primary" onclick="controlWindow(1)" id="btnWindowOpen">
                        <i class="fas fa-door-open"></i> buka
                    </button>
                    <button class="btn-secondary" onclick="controlWindow(0)" id="btnWindowClose">
                        <i class="fas fa-door-closed"></i> tutup
                    </button>
                </div>
                
                <!-- Fan Control -->
                <div class="btn-group">
                    <h3><i class="fas fa-fan"></i> Kontrol Kipas</h3>
                    <button class="btn-info" onclick="controlFan(1)" id="btnFanOn">
                        <i class="fas fa-power-off"></i> NYALA
                    </button>
                    <button class="btn-secondary" onclick="controlFan(0)" id="btnFanOff">
                        <i class="fas fa-stop"></i> MATI
                    </button>
                </div>

                <!-- Threshold Setting -->
                <div class="btn-group">
                    <h3><i class="fas fa-sliders-h"></i> Threshold Suhu</h3>
                    <div class="threshold-control">
                        <input type="range" id="thresholdSlider" min="20" max="45" value="30" 
                               oninput="updateThresholdDisplay(this.value)">
                        <span id="thresholdDisplay">30°C</span>
                    </div>
                    <button class="btn-primary" onclick="setThreshold()">
                        <i class="fas fa-save"></i> SIMPAN
                    </button>
                </div>
            </div>
        </div>

        <!-- Data Log -->
        <div class="card">
            <h2><i class="fas fa-history"></i> Log Data Terakhir</h2>
            <div class="table-container">
                <table id="dataTable">
                    <thead>
                        <tr>
                            <th>Waktu</th>
                            <th>Suhu</th>
                            <th>Kelembaban</th>
                            <th>Jendela</th>
                            <th>Kipas</th>
                            <th>Status</th>
                        </tr>
                    </thead>
                    <tbody id="dataBody">
                        <!-- Data akan diisi oleh JavaScript -->
                    </tbody>
                </table>
            </div>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>🌱 Smart Greenhouse IoT System | ESP32 + DHT11 + Servo + Fan</p>
            <p>Last Update: <span id="lastUpdate">--</span></p>
        </div>
    </div>

    <script src="assets/css/js/main.js"></script>
</body>
</html>