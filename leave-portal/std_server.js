// server.js
const express = require('express');
const mysql = require('mysql2/promise');
const bodyParser = require('body-parser');
const cors = require('cors');
const path = require('path');

const app = express();
const port = 8080;

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

// Database connection pool
const pool = mysql.createPool({
  host: 'localhost',
  user: 'root',        // Replace with your MySQL username
  password: 'Abinesh1010', // Replace with your MySQL password
  database: 'student_leave_portal',
  waitForConnections: true,
  connectionLimit: 10,
  queueLimit: 0
});

// API Routes
// 1. Get student information
app.get('/api/student-info', async (req, res) => {
  try {
    // In a real app, you'd get the student ID from the session
    const studentId = req.query.id || 'ST12345';
    
    const [rows] = await pool.query(
      'SELECT name, student_id, class, remaining_leaves FROM students WHERE student_id = ?',
      [studentId]
    );
    
    if (rows.length > 0) {
      res.json({
        success: true,
        data: rows[0]
      });
    } else {
      res.status(404).json({
        success: false,
        message: 'Student not found'
      });
    }
  } catch (error) {
    console.error('Error fetching student info:', error);
    res.status(500).json({
      success: false,
      message: 'Internal server error'
    });
  }
});

// 2. Get leave requests
app.get('/api/leave-requests', async (req, res) => {
  try {
    // In a real app, you'd get the student ID from the session
    const studentId = req.query.id || 'ST12345';
    
    const [rows] = await pool.query(
      `SELECT l.id, l.leave_type, l.from_date, l.to_date, l.reason, 
      l.status, l.rejection_reason, t.name as teacher_name, t.id as teacher_id
      FROM leave_requests l
      JOIN teachers t ON l.teacher_id = t.id
      WHERE l.student_id = ?
      ORDER BY l.created_at DESC`,
      [studentId]
    );
    
    res.json({
      success: true,
      data: rows
    });
  } catch (error) {
    console.error('Error fetching leave requests:', error);
    res.status(500).json({
      success: false,
      message: 'Internal server error'
    });
  }
});

// 3. Submit leave request
app.post('/api/submit-leave', async (req, res) => {
  try {
    const { leaveType, fromDate, toDate, reason, teacherId } = req.body;
    
    // In a real app, you'd get the student ID from the session
    const studentId = req.body.studentId || 'ST12345';
    
    // Data validation
    if (!leaveType || !fromDate || !toDate || !reason || !teacherId) {
      return res.status(400).json({
        success: false,
        message: 'All fields are required'
      });
    }
    
    // Insert into database
    const [result] = await pool.query(
      `INSERT INTO leave_requests 
      (student_id, leave_type, from_date, to_date, reason, teacher_id, status, created_at) 
      VALUES (?, ?, ?, ?, ?, ?, 'pending', NOW())`,
      [studentId, leaveType, fromDate, toDate, reason, teacherId]
    );
    
    res.json({
      success: true,
      message: 'Leave request submitted successfully',
      data: {
        id: result.insertId
      }
    });
  } catch (error) {
    console.error('Error submitting leave request:', error);
    res.status(500).json({
      success: false,
      message: 'Internal server error'
    });
  }
});

// 4. Get teachers list
app.get('/api/teachers', async (req, res) => {
  try {
    const [rows] = await pool.query('SELECT id, name FROM teachers ORDER BY name');
    
    res.json({
      success: true,
      data: rows
    });
  } catch (error) {
    console.error('Error fetching teachers:', error);
    res.status(500).json({
      success: false,
      message: 'Internal server error'
    });
  }
});

// Start server
app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});

// Initialize database function - run this once to set up tables
async function initializeDatabase() {
  try {
    const connection = await mysql.createConnection({
      host: 'localhost',
      user: 'root',
      password: 'Abinesh1010',
    });
    
    // Create database if it doesn't exist
    await connection.query('CREATE DATABASE IF NOT EXISTS student_leave_portal');
    
    // Use the database
    await connection.query('USE student_leave_portal');
    
    // Create students table
    await connection.query(`
      CREATE TABLE IF NOT EXISTS students (
        id INT AUTO_INCREMENT PRIMARY KEY,
        student_id VARCHAR(20) UNIQUE NOT NULL,
        name VARCHAR(100) NOT NULL,
        class VARCHAR(50) NOT NULL,
        remaining_leaves INT DEFAULT 10,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
      )
    `);
    
    // Create teachers table
    await connection.query(`
      CREATE TABLE IF NOT EXISTS teachers (
        id INT AUTO_INCREMENT PRIMARY KEY,
        name VARCHAR(100) NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
      )
    `);
    
    // Create leave_requests table
    await connection.query(`
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
      )
    `);
    
    // Insert sample data
    // Check if sample data already exists
    const [studentRows] = await connection.query('SELECT * FROM students LIMIT 1');
    if (studentRows.length === 0) {
      // Insert sample student
      await connection.query(`
        INSERT INTO students (student_id, name, class, remaining_leaves)
        VALUES ('ST12345', 'John Doe', 'Class 10-A', 5)
      `);
      
      // Insert sample teachers
      await connection.query(`
        INSERT INTO teachers (name) VALUES 
        ('Ms. Sarah Johnson'),
        ('Mr. Robert Smith'),
        ('Mrs. Emily Davis')
      `);
      
      // Insert sample leave requests
      await connection.query(`
        INSERT INTO leave_requests 
        (student_id, leave_type, from_date, to_date, reason, teacher_id, status, rejection_reason)
        VALUES 
        ('ST12345', 'medical', '2025-03-12', '2025-03-14', 'Doctor\'s appointment', 1, 'pending', NULL),
        ('ST12345', 'personal', '2025-03-05', '2025-03-05', 'Family event', 2, 'approved', NULL),
        ('ST12345', 'family', '2025-02-28', '2025-03-02', 'Emergency situation', 3, 'rejected', 'Insufficient information provided')
      `);
    }
    
    console.log('Database initialized successfully');
    
    await connection.end();
  } catch (error) {
    console.error('Error initializing database:', error);
  }
}

// Uncomment to initialize database on first run
//cd initializeDatabase();
