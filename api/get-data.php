<?php
// =============================================
// API GET DATA - Mengambil data untuk dashboard
// =============================================
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../db.php';

try {
    // Get latest 30 sensor data untuk grafik
    $stmt = $pdo->query("SELECT * FROM sensor_data ORDER BY id DESC LIMIT 30");
    $sensorData = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Get latest single data untuk display
    $latest = $sensorData[0] ?? [
        'temperature' => 0,
        'humidity' => 0,
        'window_status' => 0,
        'fan_status' => 0,
        'alarm_status' => 0,
        'mode' => 'auto',
        'timestamp' => date('Y-m-d H:i:s')
    ];
    
    // Get current control settings
    $ctrl = $pdo->query("SELECT * FROM controls WHERE id = 1")->fetch(PDO::FETCH_ASSOC);
    
    // Jika belum ada data control, buat default
    if (!$ctrl) {
        $pdo->exec("INSERT INTO controls (id, mode, window_command, fan_command, temp_threshold) VALUES (1, 'auto', 0, 0, 30.0)");
        $ctrl = [
            'mode' => 'auto',
            'window_command' => 0,
            'fan_command' => 0,
            'temp_threshold' => 30.0
        ];
    }
    
    echo json_encode([
        'status' => 'success',
        'latest' => $latest,
        'sensor_data' => $sensorData,
        'controls' => $ctrl
    ]);
    
} catch(PDOException $e) {
    echo json_encode([
        'status' => 'error',
        'message' => $e->getMessage()
    ]);
}
?>