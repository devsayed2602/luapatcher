"""
Steam Lua Patcher - Webserver
Flask application migrated to Supabase (PostgreSQL) to handle user profiles and the new Social Friends System.
"""

from flask import Flask, send_from_directory, jsonify, abort, request, Response, send_file
from werkzeug.security import generate_password_hash, check_password_hash
import os
import json
import io
import zipfile
from datetime import datetime
from functools import wraps
from dotenv import load_dotenv
from supabase import create_client, Client

# Load environment variables from .env file
load_dotenv()

app = Flask(__name__)

# Security configuration
ACCESS_TOKEN = os.environ.get('SERVER_ACCESS_TOKEN')
ADMIN_PASSWORD = os.environ.get('ADMIN_PASSWORD')

# Supabase Configuration
SUPABASE_URL = os.environ.get('SUPABASE_URL')
SUPABASE_KEY = os.environ.get('SUPABASE_KEY')

supabase: Client = None
if SUPABASE_URL and SUPABASE_KEY:
    try:
        supabase = create_client(SUPABASE_URL, SUPABASE_KEY)
    except Exception as e:
        print(f"WARNING: Could not connect to Supabase: {e}")
else:
    print("WARNING: SUPABASE_URL or SUPABASE_KEY not set!")

# Directory paths
GAMES_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'games')
GAMES_ZIP = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'games.zip')
FIX_FILES_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'game-fix-files')

# --- Helper Functions ---

def get_lua_content(app_id):
    filename = f"{app_id}.lua"
    file_path = os.path.join(GAMES_DIR, filename)
    if os.path.exists(file_path):
        with open(file_path, 'rb') as f:
            return f.read(), filename
    if os.path.exists(GAMES_ZIP):
        try:
            with zipfile.ZipFile(GAMES_ZIP, 'r') as zf:
                if filename in zf.namelist():
                    with zf.open(filename) as f:
                        return f.read(), filename
        except Exception as e:
            print(f"Error reading {GAMES_ZIP}: {e}")
    return None, None

def require_token(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        token = request.headers.get('X-Access-Token')
        if not token or token != ACCESS_TOKEN:
            return jsonify({'error': 'Unauthorized', 'message': 'Invalid or missing access token'}), 401
        return f(*args, **kwargs)
    return decorated_function

def check_auth(username, password):
    return username == 'admin' and password == ADMIN_PASSWORD

def authenticate():
    return Response(
        'Could not verify your access level for that URL.\n'
        'You have to login with proper credentials', 401,
        {'WWW-Authenticate': 'Basic realm="Login Required"'}
    )

# --- Core Routes ---

@app.route('/')
def health_check():
    return jsonify({
        'status': 'ok',
        'service': 'Steam Lua Patcher API (Supabase Edition)',
        'version': '2.0.0'
    })

@app.route('/api/games_index.json')
@require_token
def serve_index():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    return send_from_directory(base_dir, 'games_index.json', mimetype='application/json')

@app.route('/lua/<filename>')
@require_token
def serve_lua(filename):
    app_id = filename.replace('.lua', '')
    content, actual_filename = get_lua_content(app_id)
    if not content:
        abort(404)
    return Response(content, mimetype='text/plain')

# --- User & Social System (Supabase) ---

@app.route('/api/user/check/<username>')
def check_username(username):
    if not supabase: return jsonify({'error': 'Database unavailable'}), 503
    
    res = supabase.table('profiles').select('id').eq('username', username).execute()
    return jsonify({
        'username': username,
        'available': len(res.data) == 0
    })

@app.route('/api/user/register', methods=['POST'])
def register_user():
    if not supabase: return jsonify({'error': 'Database unavailable'}), 503
    
    data = request.get_json()
    username = data.get('username', '').strip()
    password = data.get('password', '')
    
    if len(username) < 3 or len(password) < 6:
        return jsonify({'error': 'Invalid username or password length'}), 400
    
    # Check existence
    existing = supabase.table('profiles').select('id').eq('username', username).execute()
    if len(existing.data) > 0:
        return jsonify({'error': 'Username already taken'}), 409
    
    # Insert
    user_data = {
        'username': username,
        'password_hash': generate_password_hash(password),
        'level': 1,
        'xp': 0,
        'games_patched': 0
    }
    
    try:
        res = supabase.table('profiles').insert(user_data).execute()
        new_user = res.data[0]
        del new_user['password_hash']
        return jsonify({'success': True, 'user': new_user}), 201
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/user/login', methods=['POST'])
def login_user():
    if not supabase: return jsonify({'error': 'Database unavailable'}), 503
    
    data = request.get_json()
    username = data.get('username', '').strip()
    password = data.get('password', '')
    
    res = supabase.table('profiles').select('*').eq('username', username).execute()
    if not res.data:
        return jsonify({'error': 'User not found'}), 404
    
    user = res.data[0]
    if not check_password_hash(user['password_hash'], password):
        return jsonify({'error': 'Invalid password'}), 401
    
    del user['password_hash']
    return jsonify({'success': True, 'user': user})

@app.route('/api/user/profile', methods=['GET', 'POST'])
def handle_profile():
    if not supabase: return jsonify({'error': 'Database unavailable'}), 503
    
    username = request.args.get('username')
    if not username: return jsonify({'error': 'Username required'}), 400
    
    if request.method == 'POST':
        updates = request.get_json()
        # Allowed fields to update
        valid_updates = {k: v for k, v in updates.items() if k in ['level', 'xp', 'games_patched', 'avatar_url', 'total_playtime']}
        supabase.table('profiles').update(valid_updates).eq('username', username).execute()
        
    res = supabase.table('profiles').select('*').eq('username', username).execute()
    if not res.data: return jsonify({'error': 'User not found'}), 404
    
    user = res.data[0]
    if 'password_hash' in user: del user['password_hash']
    return jsonify(user)

# --- NEW Social Endpoints ---

@app.route('/api/social/search')
def social_search():
    query = request.args.get('query', '')
    if len(query) < 2: return jsonify([])
    
    res = supabase.table('profiles').select('username, level, avatar_url').ilike('username', f'%{query}%').limit(10).execute()
    return jsonify(res.data)

@app.route('/api/social/request/send', methods=['POST'])
def send_request():
    data = request.get_json()
    from_user = data.get('from_username')
    to_user = data.get('to_username')
    
    # Get IDs
    u1 = supabase.table('profiles').select('id').eq('username', from_user).execute()
    u2 = supabase.table('profiles').select('id').eq('username', to_user).execute()
    
    if not u1.data or not u2.data: return jsonify({'error': 'User not found'}), 404
    
    id1, id2 = u1.data[0]['id'], u2.data[0]['id']
    
    try:
        supabase.table('friendships').insert({'user_id': id1, 'friend_id': id2, 'status': 'pending'}).execute()
        return jsonify({'success': True})
    except Exception as e:
        return jsonify({'error': 'Request already exists or failed'}), 400

@app.route('/api/social/friends')
def get_friends():
    username = request.args.get('username')
    u = supabase.table('profiles').select('id').eq('username', username).execute()
    if not u.data: return jsonify([])
    uid = u.data[0]['id']
    
    # Fetch where user is either side of an 'accepted' friendship
    res = supabase.table('friendships').select('user_id, friend_id').eq('status', 'accepted').execute()
    
    friend_ids = []
    for f in res.data:
        if f['user_id'] == uid: friend_ids.append(f['friend_id'])
        elif f['friend_id'] == uid: friend_ids.append(f['user_id'])
        
    if not friend_ids: return jsonify([])
    
    friends_profiles = supabase.table('profiles').select('username, level, xp, avatar_url').in_('id', friend_ids).execute()
    return jsonify(friends_profiles.data)

@app.route('/api/social/requests/pending')
def get_pending():
    username = request.args.get('username')
    u = supabase.table('profiles').select('id').eq('username', username).execute()
    if not u.data: return jsonify([])
    uid = u.data[0]['id']
    
    # Incoming requests
    res = supabase.table('friendships').select('user_id').eq('friend_id', uid).eq('status', 'pending').execute()
    requester_ids = [r['user_id'] for r in res.data]
    
    if not requester_ids: return jsonify([])
    
    profiles = supabase.table('profiles').select('username, level').in_('id', requester_ids).execute()
    return jsonify(profiles.data)

@app.route('/api/social/request/accept', methods=['POST'])
def accept_request():
    data = request.get_json()
    my_user = data.get('username')
    friend_user = data.get('friend_username')
    
    u1 = supabase.table('profiles').select('id').eq('username', my_user).execute()
    u2 = supabase.table('profiles').select('id').eq('username', friend_user).execute()
    
    if not u1.data or not u2.data: return jsonify({'error': 'User not found'}), 404
    uid, fid = u1.data[0]['id'], u2.data[0]['id']
    
    supabase.table('friendships').update({'status': 'accepted'}).eq('user_id', fid).eq('friend_id', uid).execute()
    return jsonify({'success': True})

if __name__ == '__main__':
    app.run(debug=True, port=5000)
 
