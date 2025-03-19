-- Create database
CREATE DATABASE IF NOT EXISTS student_leave_portal;

-- Use the database
USE student_leave_portal;

-- Create students table
CREATE TABLE IF NOT EXISTS students (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_id VARCHAR(20) UNIQUE NOT NULL,
  name VARCHAR(100) NOT NULL,
  class VARCHAR(50) NOT NULL,
  remaining_leaves INT DEFAULT 10,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create teachers table
CREATE TABLE IF NOT EXISTS teachers (
  id INT AUTO_INCREMENT PRIMARY KEY,
  name VARCHAR(100) NOT NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create leave_requests table
CREATE TABLE IF NOT EXISTS leave_requests (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_id VARCHAR(20) NOT NULL,
  leave_type VARCHAR(50) NOT NULL,
  from_date DATE NOT NULL,
  to_date DATE NOT NULL,
  reason TEXT NOT NULL,
  teacher_id INT NOT NULL,
  status ENUM('pending', 'approved', 'rejected') DEFAULT 'pending',
  rejection_reason TEXT,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (student_id) REFERENCES students(student_id),
  FOREIGN KEY (teacher_id) REFERENCES teachers(id)
);

-- Insert sample data
-- Insert sample student
INSERT INTO students (student_id, name, class, remaining_leaves)
VALUES ('ST12345', 'John Doe', 'Class 10-A', 5);

-- Insert sample teachers
INSERT INTO teachers (name) VALUES 
('Ms. Sarah Johnson'),
('Mr. Robert Smith'),
('Mrs. Emily Davis');

-- Insert sample leave requests
INSERT INTO leave_requests 
(student_id, leave_type, from_date, to_date, reason, teacher_id, status, rejection_reason)
VALUES 
('ST12345', 'medical', '2025-03-12', '2025-03-14', 'Doctor\'s appointment', 1, 'pending', NULL),
('ST12345', 'personal', '2025-03-05', '2025-03-05', 'Family event', 2, 'approved', NULL),
('ST12345', 'family', '2025-02-28', '2025-03-02', 'Emergency situation', 3, 'rejected', 'Insufficient information provided');
