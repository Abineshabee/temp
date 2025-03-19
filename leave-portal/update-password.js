// Save this as update-password.js
const mysql = require('mysql2/promise');
const bcrypt = require('bcrypt');
const dotenv = require('dotenv');

dotenv.config();

async function updatePassword() {
    try {
        // Create database connection
        const connection = await mysql.createConnection({
            host: process.env.DB_HOST || 'localhost',
            user: process.env.DB_USER || 'root',
            password: process.env.DB_PASSWORD || 'Abinesh1010',
            database: process.env.DB_NAME || 'leave_portal'
        });
        
        // Generate new hash for 'password123'
        const saltRounds = 10;
        const password = 'password123';
        const hash = await bcrypt.hash(password, saltRounds);
        
        console.log('Generated hash for "password123":', hash);
        
        // Update passwords in database
        await connection.query('UPDATE students SET password = ? WHERE username = ?', [hash, 'student1']);
        await connection.query('UPDATE students SET password = ? WHERE username = ?', [hash, 'student2']);
        await connection.query('UPDATE teachers SET password = ? WHERE username = ?', [hash, 'teacher1']);
        await connection.query('UPDATE teachers SET password = ? WHERE username = ?', [hash, 'admin1']);
        
        console.log('Passwords updated successfully');
        
        await connection.end();
    } catch (error) {
        console.error('Error updating passwords:', error);
    }
}

updatePassword();
