// Script.js - Updated to use API calls to server

// DOM elements
const studentNameElement = document.getElementById('student-name');
const studentIdElement = document.getElementById('student-id');
const studentClassElement = document.getElementById('student-class');
const remainingLeavesElement = document.getElementById('remaining-leaves');
const leaveItemsContainer = document.getElementById('leave-items-container');
const leaveForm = document.getElementById('leave-form');
const teacherSelect = document.getElementById('teacher');
const logoutBtn = document.getElementById('logout-btn');

// API URL - change this to match your server
const API_URL = 'http://localhost:8080/api';

// Function to format date for display
function formatDate(dateString) {
    const date = new Date(dateString);
    return date.toLocaleDateString('en-US', { day: '2-digit', month: 'short', year: 'numeric' });
}

// Function to load student information
async function loadStudentInfo() {
    try {
        const response = await fetch(`${API_URL}/student-info`);
        const result = await response.json();
        
        if (result.success) {
            const student = result.data;
            studentNameElement.textContent = student.name;
            studentIdElement.textContent = student.student_id;
            studentClassElement.textContent = student.class;
            remainingLeavesElement.textContent = student.remaining_leaves;
        } else {
            console.error('Failed to load student info:', result.message);
        }
    } catch (error) {
        console.error('Error loading student info:', error);
    }
}

// Function to load teachers list
async function loadTeachers() {
    try {
        const response = await fetch(`${API_URL}/teachers`);
        const result = await response.json();
        
        if (result.success) {
            // Clear existing options except the first one
            while (teacherSelect.options.length > 1) {
                teacherSelect.remove(1);
            }
            
            // Add teacher options
            result.data.forEach(teacher => {
                const option = document.createElement('option');
                option.value = teacher.id;
                option.textContent = teacher.name;
                teacherSelect.appendChild(option);
            });
        } else {
            console.error('Failed to load teachers:', result.message);
        }
    } catch (error) {
        console.error('Error loading teachers:', error);
    }
}

// Function to display leave requests
async function displayLeaveRequests() {
    try {
        const response = await fetch(`${API_URL}/leave-requests`);
        const result = await response.json();
        
        if (result.success) {
            leaveItemsContainer.innerHTML = '';
            
            result.data.forEach(leave => {
                const leaveItem = document.createElement('div');
                leaveItem.className = `leave-item ${leave.status}`;
                
                const fromDateFormatted = formatDate(leave.from_date);
                const toDateFormatted = formatDate(leave.to_date);
                
                // Convert leave_type to display format
                const leaveTypeDisplay = leave.leave_type.charAt(0).toUpperCase() + leave.leave_type.slice(1) + ' Leave';
                
                leaveItem.innerHTML = `
                    <h3>${leaveTypeDisplay}</h3>
                    <p><strong>Period:</strong> ${fromDateFormatted} - ${toDateFormatted}</p>
                    <p><strong>Teacher:</strong> ${leave.teacher_name}</p>
                    <p><strong>Status:</strong> <span class="status status-${leave.status}">${leave.status.charAt(0).toUpperCase() + leave.status.slice(1)}</span></p>
                    ${leave.status === 'rejected' && leave.rejection_reason ? `<p><strong>Reason:</strong> ${leave.rejection_reason}</p>` : ''}
                `;
                
                leaveItemsContainer.appendChild(leaveItem);
            });
        } else {
            console.error('Failed to load leave requests:', result.message);
        }
    } catch (error) {
        console.error('Error loading leave requests:', error);
    }
}

// Function to handle form submission
async function handleFormSubmit(e) {
    e.preventDefault();
    
    const leaveType = document.getElementById('leave-type').value;
    const fromDate = document.getElementById('from-date').value;
    const toDate = document.getElementById('to-date').value;
    const reason = document.getElementById('reason').value;
    const teacherId = document.getElementById('teacher').value;
    
    // Validate form
    if (!leaveType || !fromDate || !toDate || !reason || !teacherId) {
        alert('Please fill all required fields');
        return;
    }
    
    try {
        const response = await fetch(`${API_URL}/submit-leave`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                leaveType,
                fromDate,
                toDate,
                reason,
                teacherId
            })
        });
        
        const result = await response.json();
        
        if (result.success) {
            // Reset form
            leaveForm.reset();
            
            // Refresh leave requests
            await displayLeaveRequests();
            
            // Show success message
            alert('Leave request submitted successfully');
        } else {
            alert(`Error: ${result.message}`);
        }
    } catch (error) {
        console.error('Error submitting leave request:', error);
        alert('An error occurred while submitting your request');
    }
}

// Function to handle logout
function handleLogout() {
    // In a real application, this would involve clearing session data
    // For now, we'll just redirect to a hypothetical login page
    alert('You have been logged out');
    // window.location.href = 'login.html';
}

// Initialize the application
async function initApp() {
    try {
        // Load student information
        await loadStudentInfo();
        
        // Load teachers
        await loadTeachers();
        
        // Display leave requests
        await displayLeaveRequests();
        
        // Add form submission handler
        if (leaveForm) {
            leaveForm.addEventListener('submit', handleFormSubmit);
        }
        
        // Add logout handler
        if (logoutBtn) {
            logoutBtn.addEventListener('click', handleLogout);
        }
    } catch (error) {
        console.error('Error initializing app:', error);
    }
}

// Add event listeners when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', initApp);
