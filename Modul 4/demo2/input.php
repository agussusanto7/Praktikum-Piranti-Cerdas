<?php
// Set header JSON
header('Content-Type: application/json');

// Log untuk debugging
error_reporting(E_ALL);
ini_set('display_errors', 1);

// ===== KONFIGURASI DATABASE =====
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "sensor_dht";

// ===== KONEKSI KE DATABASE =====
$conn = new mysqli($servername, $username, $password, $dbname);

// Cek koneksi database
if ($conn->connect_error) {
    http_response_code(500);
    echo json_encode([
        "status" => "error", 
        "message" => "Koneksi database gagal: " . $conn->connect_error,
        "hint" => "Pastikan MySQL sudah berjalan dan database 'sensor_dht' sudah dibuat"
    ]);
    exit();
}

// Set charset
$conn->set_charset("utf8mb4");

// ===== TERIMA DATA JSON DARI ESP32 =====
$input = file_get_contents('php://input');
$data = json_decode($input, true);

// Log data yang diterima (untuk debugging)
error_log("Data diterima: " . $input);

// ===== VALIDASI DATA =====
if (!$data) {
    http_response_code(400);
    echo json_encode([
        "status" => "error",
        "message" => "Format JSON tidak valid",
        "received" => $input
    ]);
    $conn->close();
    exit();
}

if (!isset($data["temperature"]) || !isset($data["humidity"])) {
    http_response_code(400);
    echo json_encode([
        "status" => "error",
        "message" => "Data tidak lengkap",
        "required" => ["temperature", "humidity"],
        "received" => array_keys($data)
    ]);
    $conn->close();
    exit();
}

// ===== PROSES DATA =====
$temperature = floatval($data["temperature"]);
$humidity = floatval($data["humidity"]);

// Validasi range data
if ($temperature < -40 || $temperature > 80) {
    http_response_code(400);
    echo json_encode([
        "status" => "error",
        "message" => "Nilai suhu tidak valid (range: -40 sampai 80°C)",
        "received" => $temperature
    ]);
    $conn->close();
    exit();
}

if ($humidity < 0 || $humidity > 100) {
    http_response_code(400);
    echo json_encode([
        "status" => "error",
        "message" => "Nilai kelembaban tidak valid (range: 0 sampai 100%)",
        "received" => $humidity
    ]);
    $conn->close();
    exit();
}

// ===== INSERT DATA KE DATABASE =====
$sql = "INSERT INTO data_dht11 (temperature, humidity) VALUES (?, ?)";
$stmt = $conn->prepare($sql);

if (!$stmt) {
    http_response_code(500);
    echo json_encode([
        "status" => "error",
        "message" => "Prepare statement gagal: " . $conn->error,
        "hint" => "Pastikan tabel 'data_dht11' sudah dibuat"
    ]);
    $conn->close();
    exit();
}

$stmt->bind_param("dd", $temperature, $humidity);

if ($stmt->execute()) {
    $insert_id = $stmt->insert_id;
    
    http_response_code(200);
    echo json_encode([
        "status" => "success",
        "message" => "Data berhasil disimpan",
        "data" => [
            "id" => $insert_id,
            "temperature" => $temperature,
            "humidity" => $humidity,
            "timestamp" => date('Y-m-d H:i:s')
        ]
    ]);
} else {
    http_response_code(500);
    echo json_encode([
        "status" => "error",
        "message" => "Gagal menyimpan data: " . $stmt->error
    ]);
}

$stmt->close();
$conn->close();
?>