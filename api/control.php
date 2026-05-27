<?php
// =============================================
// API CONTROL - Mengontrol perangkat dari web
// =============================================
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../db.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $action = $_POST['action'] ?? '';
    
    try {
        switch($action) {
            // Set mode (auto/manual)
            case 'mode':
                $mode = $_POST['value'] ?? 'auto';
                $mode = ($mode === 'manual') ? 'manual' : 'auto';
                
                $stmt = $pdo->prepare("UPDATE controls SET mode = ? WHERE id = 1");
                $stmt->execute([$mode]);
                
                // Jika kembali ke auto, reset command
                if ($mode === 'auto') {
                    $pdo->exec("UPDATE controls SET window_command = 0, fan_command = 0 WHERE id = 1");
                }
                
                echo json_encode([
                    'status' => 'success', 
                    'action' => 'mode', 
                    'value' => $mode
                ]);
                break;
                
            // Kontrol jendela (servo)
            case 'window':
                $cmd = intval($_POST['value'] ?? 0);
                $cmd = ($cmd === 1) ? 1 : 0;
                
                $stmt = $pdo->prepare("UPDATE controls SET window_command = ? WHERE id = 1");
                $stmt->execute([$cmd]);
                
                echo json_encode([
                    'status' => 'success', 
                    'action' => 'window', 
                    'value' => $cmd
                ]);
                break;
                
            // Kontrol kipas
            case 'fan':
                $cmd = intval($_POST['value'] ?? 0);
                $cmd = ($cmd === 1) ? 1 : 0;
                
                $stmt = $pdo->prepare("UPDATE controls SET fan_command = ? WHERE id = 1");
                $stmt->execute([$cmd]);
                
                echo json_encode([
                    'status' => 'success', 
                    'action' => 'fan', 
                    'value' => $cmd
                ]);
                break;
                
            // Set threshold suhu
            case 'threshold':
                $threshold = floatval($_POST['value'] ?? 30.0);
                $threshold = max(20, min(45, $threshold)); // Batasi 20-45°C
                
                $stmt = $pdo->prepare("UPDATE controls SET temp_threshold = ? WHERE id = 1");
                $stmt->execute([$threshold]);
                
                echo json_encode([
                    'status' => 'success', 
                    'action' => 'threshold', 
                    'value' => $threshold
                ]);
                break;
                
            default:
                echo json_encode([
                    'status' => 'error', 
                    'message' => 'Invalid action'
                ]);
        }
    } catch(PDOException $e) {
        echo json_encode([
            'status' => 'error', 
            'message' => $e->getMessage()
        ]);
    }
} else {
    echo json_encode([
        'status' => 'error', 
        'message' => 'Invalid request method. Use POST.'
    ]);
}
?>