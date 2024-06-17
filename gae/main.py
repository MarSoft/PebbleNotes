from flask import Flask, redirect, request, jsonify
import json
import random
import urllib.request
from urllib.parse import urlencode
import os

try:
    from google.appengine.api import memcache
except ImportError:
    # Mock memcache for local development
    class SimpleCache:
        def __init__(self):
            self.store = {}

        def set(self, key, value, time=0):
            self.store[key] = value

        def get(self, key):
            return self.store.get(key)

    memcache = SimpleCache()

from secret import client_id, client_secret
import config

app = Flask(__name__)

WORDS = (
    'red green blue white brown violet purple black yellow orange '
    'dog cat cow unicorn animal hedgehog chicken '
    'task computer phone watch android robot apple '
    'rambler rogue warrior king '
    'jeans muffin cake bake cookie oven bread '
).split()

def query_json(url, data):
    if not isinstance(data, str):
        data = urlencode(data).encode()
    try:
        with urllib.request.urlopen(url, data) as response:
            return json.loads(response.read())
    except urllib.error.HTTPError as e:
        return json.loads(e.read())

@app.route('/auth')
def auth_redirect():
    url = 'https://accounts.google.com/o/oauth2/auth?' + urlencode({
        'client_id': client_id,
        'redirect_uri': config.auth_redir_uri,
        'response_type': 'code',
        'scope': 'https://www.googleapis.com/auth/tasks',
        'state': '',
        'access_type': 'offline',
        'approval_prompt': 'force',
        'include_granted_scopes': 'true',
    })
    return redirect(url)

@app.route('/auth/check')
def auth_check():
    lifetime = 10
    passcode = '-'.join(random.sample(WORDS, 4))
    passcode2 = passcode.replace('-', '')

    memcache.set(passcode, request.args.get('state'), time=lifetime*60)
    memcache.set(passcode2, request.args.get('state'), time=lifetime*60)

    html = f'''
    <style>
        tt {{
            border: 1px solid #88c;
            border-radius: 3px;
            background: #ccf;
            font-size: 20pt;
        }}
    </style>
    <h3>Passcode: <tt>{passcode}</tt></h3>
    <h3>Or: <tt>{passcode2}</tt></h3>
    <p><em>It will expire in {lifetime} minutes.</em></p>
    <p>Enter it on application settings page</p>
    '''
    return html

@app.route('/auth/result')
def auth_callback():
    code = request.args.get('code')
    if not code:
        return 'Code parameter missing', 400

    result = query_json('https://accounts.google.com/o/oauth2/token', {
        'code': code,
        'client_id': client_id,
        'client_secret': client_secret,
        'redirect_uri': config.auth_redir_uri,
        'grant_type': 'authorization_code'
    })
    return jsonify(result)

@app.route('/auth/refresh')
def auth_refresh():
    refresh_token = request.args.get('refresh_token')
    if not refresh_token:
        return 'Refresh token parameter missing', 400

    result = query_json('https://accounts.google.com/o/oauth2/token', {
        'refresh_token': refresh_token,
        'client_id': client_id,
        'client_secret': client_secret,
        'grant_type': 'refresh_token'
    })
    return jsonify(result)

if __name__ == '__main__':
    app.run(debug=True)
