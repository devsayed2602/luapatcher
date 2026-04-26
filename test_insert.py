import os
from dotenv import load_dotenv
from supabase import create_client, Client

load_dotenv('d:\\webserver\\.env')

SUPABASE_URL = os.environ.get('SUPABASE_URL')
SUPABASE_KEY = os.environ.get('SUPABASE_KEY')

if not SUPABASE_URL or not SUPABASE_KEY:
    print("Missing credentials!")
    exit(1)

supabase: Client = create_client(SUPABASE_URL, SUPABASE_KEY)

try:
    print("Testing chat_messages table insert...")
    res = supabase.table('chat_messages').insert({
        'sender_username': 'empower',
        'receiver_username': 'leVi',
        'message_text': 'test from script'
    }).execute()
    print("Insert successful!", res.data)
except Exception as e:
    print(f"Error inserting: {e}")
