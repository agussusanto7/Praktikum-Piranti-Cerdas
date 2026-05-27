<?php
header('Content-Type: application/json');

// Konfigurasi database
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "sensor_dht";

// Koneksi ke database
$conn = new mysqli($servername, $username, $password, $dbname);

// Cek koneksi
if ($conn->connect_error) {
    die(json_encode([
        "status" => "error", 
        "message" => "Connection failed: " . $conn->connect_error
    ]));
}

// Ambil parameter limit (default 100 data)
$limit = isset($_GET['limit']) ? intval($_GET['limit']) : 100;

// Query ambil data terbaru
$sql = "SELECT * FROM data_dht11 ORDER BY id_dht DESC LIMIT ?";
$stmt = $conn->prepare($sql);
$stmt->bind_param("i", $limit);
$stmt->execute();
$result = $stmt->get_result();

$data = [];

if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $data[] = [
            "id" => $row["id_dht"],
            "temperature" => floatval($row["temperature"]),
            "humidity" => floatval($row["humidity"]),
            "timestamp" => $row["waktu"]
        ];
    }
    echo json_encode([
        "status" => "success",
        "count" => count($data),
        "data" => $data
    ]);
} else {
    echo json_encode([
        "status" => "success",
        "count" => 0,
        "data" => [],
        "message" => "Belum ada data"
    ]);
}

$stmt->close();
$conn->close();
?>