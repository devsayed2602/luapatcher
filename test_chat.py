import os
from dotenv import load_dotenv
from supabase import create_client, Client

# Load dotenv from webserver
load_dotenv('d:\\webserver\\.env')

SUPABASE_URL = os.environ.get('SUPABASE_URL')
SUPABASE_KEY = os.environ.get('SUPABASE_KEY')

print(f"URL: {SUPABASE_URL}")
if not SUPABASE_URL or not SUPABASE_KEY:
    print("Missing credentials!")
    exit(1)

supabase: Client = create_client(SUPABASE_URL, SUPABASE_KEY)

try:
    print("Testing chat_messages table...")
    # Try to select
    res = supabase.table('chat_messages').select('*').limit(1).execute()
    print("Select successful!", res.data)
except Exception as e:
    print(f"Error selecting: {e}")
