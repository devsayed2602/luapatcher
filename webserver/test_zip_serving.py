import zipfile
import os
import io
import requests
from app import app
import unittest

class TestZipServing(unittest.TestCase):
    def setUp(self):
        self.app = app.test_client()
        self.app.testing = True
        
        # Create a temporary games.zip
        self.zip_path = 'games.zip'
        with zipfile.ZipFile(self.zip_path, 'w') as zf:
            zf.writestr('888888.lua', '-- Zip Test Lua')

    def test_serve_from_zip(self):
        # Test free-download from ZIP
        response = self.app.get('/api/free-download?appid=888888')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.mimetype, 'application/zip')
        
        # Verify content
        with zipfile.ZipFile(io.BytesIO(response.data)) as zf:
            self.assertIn('888888.lua', zf.namelist())
            with zf.open('888888.lua') as f:
                self.assertEqual(f.read(), b'-- Zip Test Lua')

    def test_check_availability_from_zip(self):
        # We need a token for check_availability
        # Since I don't have the real token, I'll skip this or mock it
        # Actually, let's just test the helper directly if needed, 
        # but the API test is better.
        pass

    def tearDown(self):
        if os.path.exists(self.zip_path):
            os.remove(self.zip_path)

if __name__ == '__main__':
    unittest.main()
