<?php
// Database connection
$db_config = [
    'host' => 'localhost',
    'username' => 'root',
    'password' => '',
    'database' => 'leave_portal'
];

function connectDB() {
    global $db_config;
    $conn = new mysqli(
        $db_config['host'],
        $db_config['username'],
        $db_config['password'],
        $db_config['database']
    );
    
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    
    return $conn;
}

// Create database tables
function createTables() {
    $conn = connectDB();
    
    // Users table
    $sql = "CREATE TABLE IF NOT EXISTS users (
        id INT AUTO_INCREMENT PRIMARY KEY,
        username VARCHAR(50) UNIQUE NOT NULL,
        password VARCHAR(255) NOT NULL,
        role ENUM('student', 'teacher', 'admin') NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )";
    
    if (!$conn->query($sql)) {
        echo "Error creating users table: " . $conn->error;
    }
    
    // Students table
    $sql = "CREATE TABLE IF NOT EXISTS students (
        id INT AUTO_INCREMENT PRIMARY KEY,
        user_id INT NOT NULL,
        student_id VARCHAR(20) UNIQUE NOT NULL,
        name VARCHAR(100) NOT NULL,
        class VARCHAR(20) NOT NULL,
        remaining_leaves INT DEFAULT 10,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    )";
    
    if (!$conn->query($sql)) {
        echo "Error creating students table: " . $conn->error;
    }
    
    // Teachers table
    $sql = "CREATE TABLE IF NOT EXISTS teachers (
        id INT AUTO_INCREMENT PRIMARY KEY,
        user_id INT NOT NULL,
        teacher_id VARCHAR(20) UNIQUE NOT NULL,
        name VARCHAR(100) NOT NULL,
        department VARCHAR(50) NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    )";
    
    if (!$conn->query($sql)) {
        echo "Error creating teachers table: " . $conn->error;
    }
    
    // Teacher-Class mapping
    $sql = "CREATE TABLE IF NOT EXISTS teacher_classes (
        id INT AUTO_INCREMENT PRIMARY KEY,
        teacher_id INT NOT NULL,
        class VARCHAR(20) NOT NULL,
        FOREIGN KEY (teacher_id) REFERENCES teachers(id) ON DELETE CASCADE
    )";
    
    if (!$conn->query($sql)) {
        echo "Error creating teacher_classes table: " . $conn->error;
    }
    
    // Leave requests table
    $sql = "CREATE TABLE IF NOT EXISTS leave_requests (
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
    )";
    
    if (!$conn->query($sql)) {
        echo "Error creating leave_requests table: " . $conn->error;
    }
    
    $conn->close();
    echo "Database tables created successfully";
}

// Authentication functions
function login($username, $password) {
    $conn = connectDB();
    
    $stmt = $conn->prepare("SELECT id, password, role FROM users WHERE username = ?");
    $stmt->bind_param("s", $username);
    $stmt->execute();
    $result = $stmt->get_result();
    
    if ($result->num_rows === 1) {
        $user = $result->fetch_assoc();
        if (password_verify($password, $user['password'])) {
            session_start();
            $_SESSION['user_id'] = $user['id'];
            $_SESSION['role'] = $user['role'];
            
            return [
                'success' => true,
                'role' => $user['role']
            ];
        }
    }
    
    return [
        'success' => false,
        'message' => 'Invalid username or password'
    ];
}

function checkAuth() {
    session_start();
    if (!isset($_SESSION['user_id'])) {
        header("Location: login.php");
        exit;
    }
    return $_SESSION['user_id'];
}

function checkRole($allowed_roles) {
    session_start();
    if (!isset($_SESSION['role']) || !in_array($_SESSION['role'], $allowed_roles)) {
        header("Location: unauthorized.php");
        exit;
    }
    return $_SESSION['role'];
}

// Student functions
function getStudentInfo($user_id) {
    $conn = connectDB();
    
    $stmt = $conn->prepare("SELECT s.* FROM students s JOIN users u ON s.user_id = u.id WHERE u.id = ?");
    $stmt->bind_param("i", $user_id);
    $stmt->execute();
    $result = $stmt->get_result();
    
    if ($result->num_rows === 1) {
        return $result->fetch_assoc();
    }
    
    return null;
}

function submitLeaveRequest($data) {
    $conn = connectDB();
    
    $user_id = checkAuth();
    $student = getStudentInfo($user_id);
    
    if (!$student) {
        return [
            'success' => false,
            'message' => 'Student not found'
        ];
    }
    
    // Check if student has enough leaves
    $from_date = new DateTime($data['fromDate']);
    $to_date = new DateTime($data['toDate']);
    $interval = $to_date->diff($from_date);
    $num_days = $interval->days + 1; // Include both start and end dates
    
    if ($student['remaining_leaves'] < $num_days) {
        return [
            'success' => false,
            'message' => 'Insufficient leave balance'
        ];
    }
    
    // Insert leave request
    $stmt = $conn->prepare("INSERT INTO leave_requests (student_id, teacher_id, leave_type, from_date, to_date, reason) VALUES (?, ?, ?, ?, ?, ?)");
    $stmt->bind_param("iissss", $student['id'], $data['teacher'], $data['leaveType'], $data['fromDate'], $data['toDate'], $data['reason']);
    
    if ($stmt->execute()) {
        return [
            'success' => true,
            'message' => 'Leave request submitted successfully'
        ];
    } else {
        return [
            'success' => false,
            'message' => 'Failed to submit leave request: ' . $conn->error
        ];
    }
}

function getStudentLeaves($status = null) {
    $conn = connectDB();
    
    $user_id = checkAuth();
    $student = getStudentInfo($user_id);
    
    if (!$student) {
        return [];
    }
    
    // Build query based on status filter
    $query = "SELECT lr.*, t.name as teacher_name 
              FROM leave_requests lr 
              JOIN teachers t ON lr.teacher_id = t.id 
              WHERE lr.student_id = ?";
    
    if ($status) {
        $query .= " AND lr.status = ?";
    }
    
    $query .= " ORDER BY lr.created_at DESC";
    
    $stmt = $conn->prepare($query);
    
    if ($status) {
        $stmt->bind_param("is", $student['id'], $status);
    } else {
        $stmt->bind_param("i", $student['id']);
    }
    
    $stmt->execute();
    $result = $stmt->get_result();
    
    $leaves = [];
    while ($row = $result->fetch_assoc()) {
        $leaves[] = $row;
    }
    
    return $leaves;
}

// Teacher functions
function getTeacherInfo($user_id) {
    $conn = connectDB();
    
    $stmt = $conn->prepare("SELECT t.* FROM teachers t JOIN users u ON t.user_id = u.id WHERE u.id = ?");
    $stmt->bind_param("i", $user_id);
    $stmt->execute();
    $result = $stmt->get_result();
    
    if ($result->num_rows === 1) {
        $teacher = $result->fetch_assoc();
        
        // Get classes taught by this teacher
        $stmt = $conn->prepare("SELECT class FROM teacher_classes WHERE teacher_id = ?");
        $stmt->bind_param("i", $teacher['id']);
        $stmt->execute();
        $classes_result = $stmt->get_result();
        
        $classes = [];
        while ($class = $classes_result->fetch_assoc()) {
            $classes[] = $class['class'];
        }
        
        $teacher['classes'] = $classes;
        
        return $teacher;
    }
    
    return null;
}

function getTeacherLeaveRequests($status = null) {
    $conn = connectDB();
    
    $user_id = checkAuth();
    $teacher = getTeacherInfo($user_id);
    
    if (!$teacher) {
        return [];
    }
    
    // Build query based on status filter
    $query = "SELECT lr.*, s.name as student_name, s.student_id as student_code, s.class 
              FROM leave_requests lr 
              JOIN students s ON lr.student_id = s.id 
              WHERE lr.teacher_id = ?";
    
    if ($status) {
        $query .= " AND lr.status = ?";
    }
    
    $query .= " ORDER BY lr.created_at DESC";
    
    $stmt = $conn->prepare($query);
    
    if ($status) {
        $stmt->bind_param("is", $teacher['id'], $status);
    } else {
        $stmt->bind_param("i", $teacher['id']);
    }
    
    $stmt->execute();
    $result = $stmt->get_result();
    
    $requests = [];
    while ($row = $result->fetch_assoc()) {
        $requests[] = $row;
    }
    
    return $requests;
}

function respondToLeaveRequest($request_id, $action, $comments) {
    $conn = connectDB();
    
    $user_id = checkAuth();
    $teacher = getTeacherInfo($user_id);
    
    if (!$teacher) {
        return [
            'success' => false,
            'message' => 'Teacher not found'
        ];
    }
    
    // Check if this leave request belongs to this teacher
    $stmt = $conn->prepare("SELECT * FROM leave_requests WHERE id = ? AND teacher_id = ?");
    $stmt->bind_param("ii", $request_id, $teacher['id']);
    $stmt->execute();
    
    if ($stmt->get_result()->num_rows !== 1) {
        return [
            'success' => false,
            'message' => 'Leave request not found or not authorized'
        ];
    }
    
    // Update leave request status
    $status = ($action === 'approve') ? 'approved' : 'rejected';
    
    $stmt = $conn->prepare("UPDATE leave_requests SET status = ?, response_comment = ? WHERE id = ?");
    $stmt->bind_param("ssi", $status, $comments, $request_id);
    
    if ($stmt->execute()) {
        // If approved, update student's remaining leaves
        if ($status === 'approved') {
            $stmt = $conn->prepare("
                SELECT s.id, s.remaining_leaves, DATEDIFF(lr.to_date, lr.from_date) + 1 as days_requested
                FROM leave_requests lr
                JOIN students s ON lr.student_id = s.id
                WHERE lr.id = ?
            ");
            $stmt->bind_param("i", $request_id);
            $stmt->execute();
            $result = $stmt->get_result();
            
            if ($row = $result->fetch_assoc()) {
                $new_leaves = $row['remaining_leaves'] - $row['days_requested'];
                
                $stmt = $conn->prepare("UPDATE students SET remaining_leaves = ? WHERE id = ?");
                $stmt->bind_param("ii", $new_leaves, $row['id']);
                $stmt->execute();
            }
        }
        
        return [
            'success' => true,
            'message' => 'Leave request ' . $status . ' successfully'
        ];
    } else {
        return [
            'success' => false,
            'message' => 'Failed to update leave request: ' . $conn->error
        ];
    }
}

// API Endpoints

// Student submit leave request
if (basename($_SERVER['PHP_SELF']) == 'submit_leave.php') {
    header('Content-Type: application/json');
    checkRole(['student']);
    
    $data = [
        'leaveType' => $_POST['leaveType'],
        'fromDate' => $_POST['fromDate'],
        'toDate' => $_POST['toDate'],
        'reason' => $_POST['reason'],
        'teacher' => $_POST['teacher']
    ];
    
    echo json_encode(submitLeaveRequest($data));
}

// Get student info
if (basename($_SERVER['PHP_SELF']) == 'get_student_info.php') {
    header('Content-Type: application/json');
    checkRole(['student']);
    
    $user_id = checkAuth();
    $student = getStudentInfo($user_id);
    
    if ($student) {
        echo json_encode([
            'name' => $student['name'],
            'id' => $student['student_id'],
            'class' => $student['class'],
            'remainingLeaves' => $student['remaining_leaves']
        ]);
    } else {
        echo json_encode(['error' => 'Student not found']);
    }
}

// Get recent student leaves
if (basename($_SERVER['PHP_SELF']) == 'get_recent_leaves.php') {
    header('Content-Type: application/json');
    checkRole(['student']);
    
    $leaves = getStudentLeaves();
    echo json_encode($leaves);
}

// Get teacher info
if (basename($_SERVER['PHP_SELF']) == 'get_teacher_info.php') {
    header('Content-Type: application/json');
    checkRole(['teacher']);
    
    $user_id = checkAuth();
    $teacher = getTeacherInfo($user_id);
    
    if ($teacher) {
        echo json_encode([
            'name' => $teacher['name'],
            'id' => $teacher['teacher_id'],
            'department' => $teacher['department'],
            'classes' => $teacher['classes']
        ]);
    } else {
        echo json_encode(['error' => 'Teacher not found']);
    }
}

// Respond to leave request
if (basename($_SERVER['PHP_SELF']) == 'respond_leave.php') {
    header('Content-Type: application/json');
    checkRole(['teacher']);
    
    $request_id = $_POST['requestId'];
    $action = $_POST['action'];
    $comments = $_POST['comments'];
    
    echo json_encode(respondToLeaveRequest($request_id, $action, $comments));
}

// Login script
if (basename($_SERVER['PHP_SELF']) == 'login_process.php') {
    header('Content-Type: application/json');
    
    $username = $_POST['username'];
    $password = $_POST['password'];
    
    $result = login($username, $password);
    
    if ($result['success']) {
        $redirect = ($result['role'] == 'student') ? 'index.html' : 'teacher_dashboard.html';
        $result['redirect'] = $redirect;
    }
    
    echo json_encode($result);
}

// Logout script
if (basename($_SERVER['PHP_SELF']) == 'logout.php') {
    session_start();
    session_destroy();
    header('Location: login.html');
    exit;
}
?>
