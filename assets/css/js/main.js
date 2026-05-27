// =============================================
// SMART GREENHOUSE IoT - MAIN JAVASCRIPT
// =============================================

// Setup Chart.js
const ctx = document.getElementById('sensorChart').getContext('2d');
const chart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [
            {
                label: 'Suhu (°C)',
                data: [],
                borderColor: '#f5576c',
                backgroundColor: 'rgba(245, 87, 108, 0.1)',
                tension: 0.4,
                fill: true,
                yAxisID: 'y'
            },
            {
                label: 'Kelembaban (%)',
                data: [],
                borderColor: '#4facfe',
                backgroundColor: 'rgba(79, 172, 254, 0.1)',
                tension: 0.4,
                fill: true,
                yAxisID: 'y1'
            }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        interaction: {
            mode: 'index',
            intersect: false,
        },
        scales: {
            x: {
                display: true,
                title: {
                    display: true,
                    text: 'Waktu'
                }
            },
            y: {
                type: 'linear',
                display: true,
                position: 'left',
                title: {
                    display: true,
                    text: 'Suhu (°C)'
                },
                min: 0,
                max: 50
            },
            y1: {
                type: 'linear',
                display: true,
                position: 'right',
                title: {
                    display: true,
                    text: 'Kelembaban (%)'
                },
                min: 0,
                max: 100,
                grid: {
                    drawOnChartArea: false,
                },
            }
        },
        plugins: {
            legend: {
                display: true,
                position: 'top'
            }
        }
    }
});

// Current mode
let currentMode = 'auto';

// ===== UPDATE DATA DARI SERVER =====
function updateData() {
    fetch('api/get-data.php')
        .then(response => response.json())
        .then(data => {
            if (data.status === 'success') {
                const latest = data.latest;
                const controls = data.controls;
                
                // Update sensor values
                document.getElementById('tempValue').textContent = parseFloat(latest.temperature).toFixed(1);
                document.getElementById('humValue').textContent = parseFloat(latest.humidity).toFixed(1);
                
                // Update window status
                const windowStatus = parseInt(latest.window_status);
                document.getElementById('windowValue').textContent = windowStatus ? 'TERTUTUP' : 'TERBUKA';
                document.getElementById('windowIcon').className = windowStatus ? 'fas fa-window-maximize' : 'fas fa-door-open';
                
                // Update fan status
                const fanStatus = parseInt(latest.fan_status);
                document.getElementById('fanValue').textContent = fanStatus ? 'MENYALA' : 'MATI';
                const fanIcon = document.getElementById('fanIcon');
                if (fanStatus) {
                    fanIcon.classList.add('fan-spinning');
                } else {
                    fanIcon.classList.remove('fan-spinning');
                }
                
                // Update alarm status
                const alarmStatus = parseInt(latest.alarm_status);
                const alarmBox = document.getElementById('alarmBox');
                const alarmText = document.getElementById('alarmText');
                if (alarmStatus) {
                    alarmBox.className = 'alarm-box alert';
                    alarmBox.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>⚠️ PERINGATAN: Suhu Tinggi!</span>';
                } else {
                    alarmBox.className = 'alarm-box normal';
                    alarmBox.innerHTML = '<i class="fas fa-check-circle"></i><span>✓ Sistem Normal</span>';
                }

                // Update system status pill
                const systemStatus = document.getElementById('systemStatus');
                if (systemStatus) {
                    systemStatus.textContent = alarmStatus ? 'Peringatan' : 'Normal';
                    systemStatus.className = 'status-pill ' + (alarmStatus ? 'status-alert' : 'status-ok');
                }
                
                // Update mode badge from controls
                if (controls && controls.mode) {
                    currentMode = controls.mode;
                    const modeBadge = document.getElementById('modeBadge');
                    modeBadge.textContent = controls.mode.toUpperCase();
                    modeBadge.className = 'mode-badge ' + (controls.mode === 'auto' ? 'mode-auto' : 'mode-manual');
                    
                    // Update button states
                    updateButtonStates();
                }
                
                // Update threshold display
                if (controls && controls.temp_threshold) {
                    document.getElementById('thresholdValue').textContent = controls.temp_threshold;
                    document.getElementById('thresholdSlider').value = controls.temp_threshold;
                    document.getElementById('thresholdDisplay').textContent = controls.temp_threshold + '°C';
                }
                
                // Update chart
                if (data.sensor_data && data.sensor_data.length > 0) {
                    const chartData = data.sensor_data.slice().reverse();
                    chart.data.labels = chartData.map(d => {
                        const date = new Date(d.timestamp);
                        return date.toLocaleTimeString('id-ID', {hour: '2-digit', minute: '2-digit', second: '2-digit'});
                    });
                    chart.data.datasets[0].data = chartData.map(d => parseFloat(d.temperature));
                    chart.data.datasets[1].data = chartData.map(d => parseFloat(d.humidity));
                    chart.update('none'); // Update tanpa animasi untuk performa
                }
                
                // Update data table
                updateDataTable(data.sensor_data.slice(0, 10));
                
                // Update last update time
                document.getElementById('lastUpdate').textContent = new Date().toLocaleString('id-ID');
            }
        })
        .catch(error => {
            console.error('Error fetching data:', error);
            const systemStatus = document.getElementById('systemStatus');
            if (systemStatus) {
                systemStatus.textContent = 'Koneksi Terputus';
                systemStatus.className = 'status-pill status-offline';
            }
        });
}

// ===== UPDATE DATA TABLE =====
function updateDataTable(data) {
    const tbody = document.getElementById('dataBody');
    tbody.innerHTML = '';
    
    data.forEach(row => {
        const tr = document.createElement('tr');
        const date = new Date(row.timestamp);
        const timeStr = date.toLocaleString('id-ID');
        
        tr.innerHTML = `
            <td>${timeStr}</td>
            <td>${parseFloat(row.temperature).toFixed(1)}°C</td>
            <td>${parseFloat(row.humidity).toFixed(1)}%</td>
            <td>${parseInt(row.window_status) ? '🔴 Tertutup' : '🟢 Terbuka'}</td>
            <td>${parseInt(row.fan_status) ? '🟢 Nyala' : '🔴 Mati'}</td>
            <td class="${parseInt(row.alarm_status) ? 'status-alert' : 'status-normal'}">
                ${parseInt(row.alarm_status) ? '⚠️ Alert' : '✓ Normal'}
            </td>
        `;
        tbody.appendChild(tr);
    });
}

// ===== UPDATE BUTTON STATES =====
function updateButtonStates() {
    const isManual = currentMode === 'manual';
    
    // Mode buttons
    document.getElementById('btnAuto').classList.toggle('active', !isManual);
    document.getElementById('btnManual').classList.toggle('active', isManual);
    
    // Disable/enable control buttons based on mode
    const controlButtons = ['btnWindowOpen', 'btnWindowClose', 'btnFanOn', 'btnFanOff'];
    controlButtons.forEach(id => {
        const btn = document.getElementById(id);
        btn.style.opacity = isManual ? '1' : '0.5';
        btn.style.pointerEvents = isManual ? 'auto' : 'none';
    });
}

// ===== SET MODE =====
function setMode(mode) {
    const formData = new FormData();
    formData.append('action', 'mode');
    formData.append('value', mode);
    
    fetch('api/control.php', {
        method: 'POST',
        body: formData
    })
    .then(response => response.json())
    .then(data => {
        console.log('Mode changed:', data);
        if (data.status === 'success') {
            currentMode = mode;
            const modeBadge = document.getElementById('modeBadge');
            modeBadge.textContent = mode.toUpperCase();
            modeBadge.className = 'mode-badge ' + (mode === 'auto' ? 'mode-auto' : 'mode-manual');
            updateButtonStates();
            
            // Show notification
            showNotification(`Mode berubah ke ${mode.toUpperCase()}`);
        }
    })
    .catch(error => console.error('Error:', error));
}

// ===== CONTROL WINDOW =====
function controlWindow(value) {
    if (currentMode !== 'manual') {
        showNotification('Ubah ke mode MANUAL terlebih dahulu!', 'warning');
        return;
    }
    
    const formData = new FormData();
    formData.append('action', 'window');
    formData.append('value', value);
    
    fetch('api/control.php', {
        method: 'POST',
        body: formData
    })
    .then(response => response.json())
    .then(data => {
        console.log('Window control:', data);
        if (data.status === 'success') {
            showNotification(`Jendela akan ${value ? 'DIBUKA' : 'DITUTUP'}`);
        }
    })
    .catch(error => console.error('Error:', error));
}

// ===== CONTROL FAN =====
function controlFan(value) {
    if (currentMode !== 'manual') {
        showNotification('Ubah ke mode MANUAL terlebih dahulu!', 'warning');
        return;
    }
    
    const formData = new FormData();
    formData.append('action', 'fan');
    formData.append('value', value);
    
    fetch('api/control.php', {
        method: 'POST',
        body: formData
    })
    .then(response => response.json())
    .then(data => {
        console.log('Fan control:', data);
        if (data.status === 'success') {
            showNotification(`Kipas akan ${value ? 'DINYALAKAN' : 'DIMATIKAN'}`);
        }
    })
    .catch(error => console.error('Error:', error));
}

// ===== SET THRESHOLD =====
function setThreshold() {
    const value = document.getElementById('thresholdSlider').value;
    
    const formData = new FormData();
    formData.append('action', 'threshold');
    formData.append('value', value);
    
    fetch('api/control.php', {
        method: 'POST',
        body: formData
    })
    .then(response => response.json())
    .then(data => {
        console.log('Threshold set:', data);
        if (data.status === 'success') {
            document.getElementById('thresholdValue').textContent = value;
            showNotification(`Threshold diubah ke ${value}°C`);
        }
    })
    .catch(error => console.error('Error:', error));
}

// ===== UPDATE THRESHOLD DISPLAY =====
function updateThresholdDisplay(value) {
    document.getElementById('thresholdDisplay').textContent = value + '°C';
}

// ===== SHOW NOTIFICATION =====
function showNotification(message, type = 'success') {
    // Create notification element
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 15px 25px;
        border-radius: 10px;
        color: white;
        font-weight: 600;
        z-index: 9999;
        animation: slideIn 0.3s ease;
        box-shadow: 0 5px 20px rgba(0,0,0,0.3);
    `;
    
    if (type === 'warning') {
        notification.style.background = 'linear-gradient(135deg, #f5af19 0%, #f12711 100%)';
    } else {
        notification.style.background = 'linear-gradient(135deg, #11998e 0%, #38ef7d 100%)';
    }
    
    notification.textContent = message;
    document.body.appendChild(notification);
    
    // Remove after 3 seconds
    setTimeout(() => {
        notification.style.animation = 'slideOut 0.3s ease';
        setTimeout(() => notification.remove(), 300);
    }, 3000);
}

// Add animation keyframes
const style = document.createElement('style');
style.textContent = `
    @keyframes slideIn {
        from { transform: translateX(100%); opacity: 0; }
        to { transform: translateX(0); opacity: 1; }
    }
    @keyframes slideOut {
        from { transform: translateX(0); opacity: 1; }
        to { transform: translateX(100%); opacity: 0; }
    }
`;
document.head.appendChild(style);

// ===== INITIAL LOAD & INTERVAL =====
updateData();
setInterval(updateData, 2000); // Update setiap 2 detik