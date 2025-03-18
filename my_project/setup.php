<?php
// Database setup script
// Save this as setup.php

// Database connection parameters
$db_host = 'localhost';
$db_user = 'root';
$db_pass = 'Abinesh1010';
$db_name = 'leave_portal';

// Create connection
$conn = new mysqli($db_host, $db_user, $db_pass);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Create database
$sql = "CREATE DATABASE IF NOT EXISTS $db_name";
if ($conn->query($sql) === TRUE) {
    echo "Database created successfully<br>";
} else {
    echo "Error creating database: " . $conn->error . "<br>";
    exit;
}

// Select the database
$conn->select_db($db_name);

// Create tables
$tables = [
    // Users table
    "CREATE TABLE IF NOT EXISTS users (
        id INT AUTO_INCREMENT PRIMARY KEY,
        username VARCHAR(50) UNIQUE NOT NULL,
        password VARCHAR(255) NOT NULL,
        role ENUM('student', 'teacher', 'admin') NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )",
    
    // Students table
    "CREATE TABLE IF NOT EXISTS students (
        id INT AUTO_INCREMENT PRIMARY KEY,
        user_id INT NOT NULL,
        student_id VARCHAR(20) UNIQUE NOT NULL,
        name VARCHAR(100) NOT NULL,
        class VARCHAR(20) NOT NULL,
        remaining_leaves INT DEFAULT 10,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    )",
    
    // Teachers table
    "CREATE TABLE IF NOT EXISTS teachers (
        id INT AUTO_INCREMENT PRIMARY KEY,
        user_id INT NOT NULL,
        teacher_id VARCHAR(20) UNIQUE NOT NULL,
        name VARCHAR(100) NOT NULL,
        department VARCHAR(50) NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    )",
    
    // Teacher-Class mapping
    "CREATE TABLE IF NOT EXISTS teacher_classes (
        id INT AUTO_INCREMENT PRIMARY KEY,
        teacher_id INT NOT NULL,
        class VARCHAR(20) NOT NULL,
        FOREIGN KEY (teacher_id) REFERENCES teachers(id) ON DELETE CASCADE
    )",
    
    // Leave requests table
    "CREATE TABLE IF NOT EXISTS leave_requests (
        id INT AUTO_INCREMENT PRIMARY KEY,
        student_id INT NOT NULL,
        teacher_id INT NOT NULL,
        leave_type ENUM('medical', 'personal', 'family', 'other') NOT NULL,
        from_date DATE NOT NULL,
        to_date DATE NOT NULL,
        reason TEXT NOT NULL,
        status ENUM('pending', 'approved', 'rejected') DEFAULT 'pending',
        response_comment TEXT,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
        FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,
        FOREIGN KEY (teacher_id) REFERENCES teachers(id) ON DELETE CASCADE
    )"
];

// Execute the table creation queries
$success = true;
foreach ($tables as $table_sql) {
    if ($conn->query($table_sql) !== TRUE) {
        echo "Error creating table: " . $conn->error . "<br>";
        $success = false;
    }
}

if ($success) {
    echo "All tables created successfully<br>";
}

// Insert sample data
echo "Inserting sample data...<br>";

// Create sample users (password is 'password' - hashed)
$password_hash = password_hash('password', PASSWORD_DEFAULT);

$sample_data = [
    // Users
    "INSERT INTO users (username, password, role) VALUES 
    ('student1', '$password_hash', 'student'),
    ('student2', '$password_hash', 'student'),
    ('teacher1', '$password_hash', 'teacher'),
    ('teacher2', '$password_hash', 'teacher'),
    ('admin', '$password_hash', 'admin')",
    
    // Students
    "INSERT INTO students (user_id, student_id, name, class, remaining_leaves) VALUES 
    (1, 'ST12345', 'John Doe', '10-A', 10),
    (2, 'ST12346', 'Jane Smith', '9-B', 10)",
    
    // Teachers
    "INSERT INTO teachers (user_id, teacher_id, name, department) VALUES 
    (3, 'TCH001', 'Ms. Sarah Johnson', 'Science'),
    (4, 'TCH002', 'Mr. Robert Smith', 'Mathematics')",
    
    // Teacher-Class mapping
    "INSERT INTO teacher_classes (teacher_id, class) VALUES 
    (1, '10-A'),
    (1, '9-B'),
    (1, '8-C'),
    (2, '10-B'),
    (2, '9-A'),
    (2, '8-B')",
    
    // Sample leave requests
    "INSERT INTO leave_requests (student_id, teacher_id, leave_type, from_date, to_date, reason, status) VALUES 
    (1, 1, 'medical', '2025-03-12', '2025-03-14', 'I am suffering from high fever and the doctor has advised bed rest for 3 days.', 'pending'),
    (2, 1, 'personal', '2025-03-15', '2025-03-15', 'I need to attend a family function.', 'pending'),
    (1, 2, 'family', '2025-03-05', '2025-03-05', 'Family emergency', 'approved'),
    (2, 2, 'other', '2025-02-28', '2025-03-02', 'Participating in a science exhibition', 'rejected')"
];

foreach ($sample_data as $query) {
    try {
        if ($conn->query($query) !== TRUE) {
            echo "Error inserting sample data: " . $conn->error . "<br>";
        }
    } catch (Exception $e) {
        echo "Exception: " . $e->getMessage() . "<br>";
    }
}

echo "Sample data has been inserted<br>";
echo "<br><strong>Setup complete!</strong><br>";
echo "You can now access the system with the following credentials:<br>";
echo "Student: username 'student1', password 'password'<br>";
echo "Teacher: username 'teacher1', password 'password'<br>";
echo "Admin: username 'admin', password 'password'<br>";
echo "<br><a href='login.html'>Go to Login Page</a>";

$conn->close();
?>
