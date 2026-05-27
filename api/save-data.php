<?php
// =============================================
// API SAVE DATA - Menerima data dari ESP32
// =============================================
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../db.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST' || $_SERVER['REQUEST_METHOD'] === 'GET') {
    // Support both POST and GET untuk fleksibilitas ESP32
    $temp = isset($_REQUEST['temp']) ? floatval($_REQUEST['temp']) : 0;
    $hum = isset($_REQUEST['hum']) ? floatval($_REQUEST['hum']) : 0;
    $window = isset($_REQUEST['window']) ? intval($_REQUEST['window']) : 0;
    $fan = isset($_REQUEST['fan']) ? intval($_REQUEST['fan']) : 0;
    $alarm = isset($_REQUEST['alarm']) ? intval($_REQUEST['alarm']) : 0;
    $mode = isset($_REQUEST['mode']) ? $_REQUEST['mode'] : 'auto';
    
    try {
        // Simpan data sensor
        $stmt = $pdo->prepare("
            INSERT INTO sensor_data (temperature, humidity, window_status, fan_status, alarm_status, mode) 
            VALUES (?, ?, ?, ?, ?, ?)
        ");
        $stmt->execute([$temp, $hum, $window, $fan, $alarm, $mode]);
        
        // Ambil perintah kontrol dari web (untuk dikirim ke ESP32)
        $ctrl = $pdo->query("SELECT * FROM controls WHERE id = 1")->fetch(PDO::FETCH_ASSOC);
        
        echo json_encode([
            'status' => 'success',
            'message' => 'Data saved',
            'id' => $pdo->lastInsertId(),
            'controls' => [
                'mode' => $ctrl['mode'],
                'window_cmd' => intval($ctrl['window_command']),
                'fan_cmd' => intval($ctrl['fan_command']),
                'threshold' => floatval($ctrl['temp_threshold'])
            ]
        ]);
        
    } catch(PDOException $e) {
        echo json_encode([
            'status' => 'error',
            'message' => $e->getMessage()
        ]);
    }
} else {
    echo json_encode([
        'status' => 'error',
        'message' => 'Invalid request method'
    ]);
}
?>