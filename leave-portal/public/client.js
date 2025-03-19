document.addEventListener('DOMContentLoaded', function() {
    const loginForm = document.getElementById('login-form');
    const errorMessage = document.getElementById('error-message');

    loginForm.addEventListener('submit', async function(e) {
        e.preventDefault();
        
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;
        const userType = document.querySelector('input[name="user-type"]:checked').value;
        
        try {
            const response = await fetch('/api/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ username, password, userType })
            });
            
            const data = await response.json();
            
            if (data.success) {
                // Store token in localStorage for session management
                localStorage.setItem('authToken', data.token);
                localStorage.setItem('userType', userType);
                
                // Redirect based on user role
                window.location.href = data.redirect;
            } else {
                // Show error message with animation
                errorMessage.textContent = data.message || 'Invalid username or password';
                errorMessage.style.display = 'block';
                errorMessage.classList.add('shake');
                
                // Remove animation class after animation completes
                setTimeout(() => {
                    errorMessage.classList.remove('shake');
                }, 600);
            }
        } catch (error) {
            console.error('Error:', error);
            errorMessage.textContent = 'An error occurred. Please try again.';
            errorMessage.style.display = 'block';
        }
    });
    
    // Clear error message when user starts typing
    document.getElementById('username').addEventListener('input', clearError);
    document.getElementById('password').addEventListener('input', clearError);
    
    function clearError() {
        errorMessage.style.display = 'none';
    }
});
