<?php
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "sensor_db";

// Buat koneksi
$conn = new mysqli($servername, $username, $password, $dbname);

// Cek koneksi
if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Connection failed: " . $conn->connect_error]));
}

// Query ambil data terbaru
$sql = "SELECT * FROM data_cahaya ORDER BY id_cahaya DESC LIMIT 100"; // ambil 10 data terakhir
$result = $conn->query($sql);

$data = [];

if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $data[] = $row;
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
        "data" => []
    ]);
}

$conn->close();
exit();
?>
