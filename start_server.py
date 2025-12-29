from flask import Flask, request
import subprocess
import os

app = Flask(__name__)

# Change to the directory where the executable is
BASE_DIR = r'd:\momen\Projects\Programming\C++\B+tree'
os.chdir(BASE_DIR)

# HTML template
with open('index.html', 'r', encoding='utf-8') as f:
    HTML_TEMPLATE = f.read()

@app.route('/')
def index():
    return HTML_TEMPLATE

@app.route('/cgi-bin/bplus_tree.exe', methods=['POST'])
def run_bplus_tree():
    # Get form data
    operation = request.form.get('operation', 'display')
    value = request.form.get('value', '')
    
    # Prepare the POST data string that the C program expects
    post_data = f"operation={operation}"
    if value:
        post_data += f"&value={value}"
    
    # Set up environment variables for CGI
    env = os.environ.copy()
    env['REQUEST_METHOD'] = 'POST'
    env['CONTENT_LENGTH'] = str(len(post_data))
    env['CONTENT_TYPE'] = 'application/x-www-form-urlencoded'
    
    try:
        # Run the C program directly from main directory
        result = subprocess.run(
            ['bplus_tree.exe'],  # Runs from current directory
            input=post_data.encode('utf-8'),
            capture_output=True,
            text=False,
            env=env,
            cwd=BASE_DIR
        )
        
        # Get output and decode
        output = result.stdout.decode('utf-8', errors='replace')
        
        # Remove CGI header if present
        if 'Content-Type:' in output:
            output = output.split('\r\n\r\n', 1)[-1]
            output = output.split('\n\n', 1)[-1]
        
        return output, 200, {'Content-Type': 'text/plain; charset=utf-8'}
    
    except Exception as e:
        return f"Error running program: {str(e)}", 500, {'Content-Type': 'text/plain'}

if __name__ == '__main__':
    print("\n" + "="*50)
    print("🌳 B+ Tree Web Server Starting...")
    print("="*50)
    print(f"\n✓ Server running on: http://localhost:5000")
    print(f"✓ Access the interface at: http://localhost:5000\n")
    print("Press Ctrl+C to stop the server\n")
    print("="*50 + "\n")
    
    app.run(host='0.0.0.0', port=5000, debug=False)