// www/script.js
// Interactive functionality for the HTTP server dashboard

// API Testing Functions
async function testGet(endpoint) {
    const responseDiv = document.getElementById('get-response');
    responseDiv.classList.remove('hidden');
    responseDiv.innerHTML = '<div class="spinner"></div>';
    
    try {
        const response = await fetch(endpoint);
        const text = await response.text();
        
        responseDiv.innerHTML = `
            <strong>Status:</strong> ${response.status} ${response.statusText}<br>
            <strong>Headers:</strong><br>
            <pre>${formatHeaders(response.headers)}</pre>
            <strong>Body:</strong><br>
            <pre>${escapeHtml(text.substring(0, 1000))}${text.length > 1000 ? '... (truncated)' : ''}</pre>
        `;
    } catch (error) {
        responseDiv.innerHTML = `<strong>Error:</strong> ${error.message}`;
    }
}

async function testPost() {
    const postData = document.getElementById('postData').value;
    const responseDiv = document.getElementById('post-response');
    responseDiv.classList.remove('hidden');
    responseDiv.innerHTML = '<div class="spinner"></div>';
    
    try {
        const response = await fetch('/api/test', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: postData
        });
        
        const text = await response.text();
        
        responseDiv.innerHTML = `
            <strong>Status:</strong> ${response.status} ${response.statusText}<br>
            <strong>Headers:</strong><br>
            <pre>${formatHeaders(response.headers)}</pre>
            <strong>Body:</strong><br>
            <pre>${escapeHtml(text)}</pre>
        `;
    } catch (error) {
        responseDiv.innerHTML = `<strong>Error:</strong> ${error.message}`;
    }
}

// Directory browsing
async function loadDirectory() {
    const fileList = document.getElementById('fileList');
    fileList.innerHTML = '<li class="file-item"><div class="spinner"></div></li>';
    
    try {
        const response = await fetch('/api/directory');
        const files = await response.json();
        
        if (files.length === 0) {
            fileList.innerHTML = '<li class="file-item">No files found</li>';
            return;
        }
        
        fileList.innerHTML = files.map(file => `
            <li class="file-item">
                <div class="file-icon">
                    ${file.isDirectory ? '<i class="fas fa-folder"></i>' : '<i class="fas fa-file"></i>'}
                </div>
                <a href="${file.path}" class="file-name">
                    ${file.name}${file.isDirectory ? '/' : ''}
                </a>
                ${file.size ? `<div class="file-size">${formatFileSize(file.size)}</div>` : ''}
            </li>
        `).join('');
    } catch (error) {
        fileList.innerHTML = `<li class="file-item">Error loading directory: ${error.message}</li>`;
    }
}

// Utility Functions
function formatHeaders(headers) {
    let result = '';
    headers.forEach((value, key) => {
        result += `${key}: ${value}\n`;
    });
    return result;
}

function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

function formatFileSize(bytes) {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

// Simulate server status updates
function updateServerStatus() {
    const indicator = document.querySelector('.status-indicator');
    if (Math.random() > 0.1) { // 90% chance server is running
        indicator.innerHTML = '<i class="fas fa-circle"></i> Server Running';
        indicator.style.color = '#10b981';
    } else {
        indicator.innerHTML = '<i class="fas fa-circle"></i> Server Busy';
        indicator.style.color = '#f59e0b';
    }
}

// Update status every 5 seconds
setInterval(updateServerStatus, 5000);

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    console.log('C++ HTTP Server Dashboard loaded');
    loadDirectory();
});