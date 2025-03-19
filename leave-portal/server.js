const express = require('express');
const mysql = require('mysql2/promise');
const bcrypt = require('bcrypt');
const jwt = require('jsonwebtoken');
const path = require('path');
const dotenv = require('dotenv');

// Load environment variables
dotenv.config();

const app = express();
const PORT = process.env.PORT || 3000;
const JWT_SECRET = process.env.JWT_SECRET || 'your_jwt_secret_key';

// Middleware
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

// Database connection pool
const pool = mysql.createPool({
    host: process.env.DB_HOST || 'localhost',
    user: process.env.DB_USER || 'root',
    password: process.env.DB_PASSWORD || 'Abinesh1010',
    database: process.env.DB_NAME || 'leave_portal',
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0
});
app.post('/api/login', async (req, res) => {
    try {
        const { username, password, userType } = req.body;
        
        console.log('Login attempt:', { username, userType });
        
        if (!username || !password) {
            return res.status(400).json({ 
                success: false, 
                message: 'Username and password are required' 
            });
        }
        
        // Get user from database based on userType
        const tableName = userType === 'student' ? 'students' : 'teachers';
        console.log(`Querying ${tableName} table for username: ${username}`);
        
        const [rows] = await pool.query(
            `SELECT * FROM ${tableName} WHERE username = ?`,
            [username]
        );
        
        console.log('Query result:', rows.length > 0 ? 'User found' : 'User not found');
        
        const user = rows[0];
        
        // Check if user exists
        if (!user) {
            return res.status(401).json({ 
                success: false, 
                message: 'Invalid username or password (user not found)' 
            });
        }
        
        // Compare passwords - log actual password for debugging (Remove in production!)
        console.log('Comparing password with hash:', user.password);
        const passwordMatch = await bcrypt.compare(password, user.password);
        console.log('Password comparison result:', passwordMatch);
        
        if (!passwordMatch) {
            return res.status(401).json({ 
                success: false, 
                message: 'Invalid username or password (password mismatch)' 
            });
        }
        
        // Generate JWT token
        const token = jwt.sign(
            { id: user.id, username: user.username, userType },
            JWT_SECRET,
            { expiresIn: '24h' }
        );
        
        // Determine redirect path based on user type
        const redirectPath = userType === 'student' ? '/student-dashboard.html' : '/teacher-dashboard.html';
        
        res.json({
            success: true,
            token,
            redirect: redirectPath,
            user: {
                id: user.id,
                username: user.username,
                name: user.name,
                userType
            }
        });
        
        // Log successful login
        console.log(`User ${username} (${userType}) logged in successfully`);
        
    } catch (error) {
        console.error('Login error:', error);
        res.status(500).json({ 
            success: false, 
            message: 'Server error. Please try again later.' 
        });
    }
});
// Protected route middleware
function authenticateToken(req, res, next) {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1];
    
    if (!token) return res.status(401).json({ message: 'Access denied' });
    
    jwt.verify(token, JWT_SECRET, (err, user) => {
        if (err) return res.status(403).json({ message: 'Invalid or expired token' });
        req.user = user;
        next();
    });
}

// Example protected route
app.get('/api/profile', authenticateToken, (req, res) => {
    res.json({ 
        message: 'Protected data retrieved successfully', 
        user: req.user 
    });
});

// Serve HTML files
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start server
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});
