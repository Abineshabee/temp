-- Create database
CREATE DATABASE IF NOT EXISTS leave_portal;
USE leave_portal;

-- Students table
CREATE TABLE IF NOT EXISTS students (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    department VARCHAR(50) NOT NULL,
    roll_number VARCHAR(20) NOT NULL UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Teachers table
CREATE TABLE IF NOT EXISTS teachers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    department VARCHAR(50) NOT NULL,
    employee_id VARCHAR(20) NOT NULL UNIQUE,
    is_admin BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- Leave requests table
CREATE TABLE IF NOT EXISTS leave_requests (
    id INT AUTO_INCREMENT PRIMARY KEY,
    student_id INT,
    teacher_id INT,
    start_date DATE NOT NULL,
    end_date DATE NOT NULL,
    reason TEXT NOT NULL,
    status ENUM('pending', 'approved', 'rejected') DEFAULT 'pending',
    approved_by INT,
    comments TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,
    FOREIGN KEY (teacher_id) REFERENCES teachers(id) ON DELETE CASCADE,
    FOREIGN KEY (approved_by) REFERENCES teachers(id) ON DELETE SET NULL
);

-- Insert sample data for testing
-- Note: Passwords are hashed versions of "password123"
INSERT INTO students (username, password, name, email, department, roll_number)
VALUES
('student1', '$2b$10$6jKu15cZ1xHxKSS8SSYB9Oli/xMrJ9fzptwxcEVVoavK4zEq9hUP6', 'John Student', 'john@example.com', 'Computer Science', 'CS2023001'),
('student2', '$2b$10$6jKu15cZ1xHxKSS8SSYB9Oli/xMrJ9fzptwxcEVVoavK4zEq9hUP6', 'Jane Student', 'jane@example.com', 'Electrical Engineering', 'EE2023002');

INSERT INTO teachers (username, password, name, email, department, employee_id, is_admin)
VALUES
('teacher1', '$2b$10$6jKu15cZ1xHxKSS8SSYB9Oli/xMrJ9fzptwxcEVVoavK4zEq9hUP6', 'Dr. Smith', 'smith@example.com', 'Computer Science', 'FAC2023001', FALSE),
('admin1', '$2b$10$6jKu15cZ1xHxKSS8SSYB9Oli/xMrJ9fzptwxcEVVoavK4zEq9hUP6', 'Admin User', 'admin@example.com', 'Administration', 'ADM2023001', TRUE);
