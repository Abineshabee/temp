const mysql = require('mysql2/promise');
const fs = require('fs');
const path = require('path');
const dotenv = require('dotenv');

// Load environment variables
dotenv.config();

async function setupDatabase() {
    try {
        console.log('Setting up database...');
        
        // Create connection without database selection first
        const connection = await mysql.createConnection({
            host: process.env.DB_HOST || 'localhost',
            user: process.env.DB_USER || 'root',
            password: process.env.DB_PASSWORD || 'Abinesh1010',
            multipleStatements: true
        });
        
        console.log('Connected to MySQL server');
        
        // Read SQL file
        const sqlFilePath = path.join(__dirname, 'schema.sql');
        const sqlScript = fs.readFileSync(sqlFilePath, 'utf8');
        
        console.log('Executing SQL script...');
        
        // Execute the SQL script
        await connection.query(sqlScript);
        
        console.log('Database setup completed successfully!');
        
        // Close the connection
        await connection.end();
        console.log('Connection closed');
        
    } catch (error) {
        console.error('Error setting up database:', error);
        process.exit(1);
    }
}

setupDatabase();
