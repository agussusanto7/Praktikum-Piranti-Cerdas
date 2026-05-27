<?php
$servername = "localhost";  // Ganti jika menggunakan server yang berbeda
$username = "root";  // Username MySQL
$password = "";  // Password MySQL
$dbname = "sensor_db";  // Nama database MySQL

// Buat koneksi ke database
$conn = new mysqli($servername, $username, $password, $dbname);

// Cek koneksi
if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Connection failed: " . $conn->connect_error]));
}

// Terima input JSON dari body
$data = json_decode(file_get_contents('php://input'), true);

// Debugging: Memeriksa data JSON yang diterima
file_put_contents('php://stderr', print_r($data, TRUE));  // Debugging log

// Validasi data JSON yang diterima
if (isset($data["ldr_value"])) {
    $ldr_value = $data["ldr_value"];

    // Query untuk memasukkan data ke dalam tabel
    $sql = "INSERT INTO data_cahaya (ldr_value) VALUES (" . $ldr_value . ")";

    if ($conn->query($sql) === TRUE) {
        echo json_encode(["status" => "success", "message" => "Data berhasil ditambahkan"]);
    } else {
        echo json_encode(["status" => "error", "message" => $conn->error]);
    }

    $conn->close();
} else {
    echo json_encode(["status" => "error", "message" => "Data JSON tidak valid"]);
}
exit();
?>
