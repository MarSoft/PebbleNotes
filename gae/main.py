import json
import os
import random
import time
from urllib.parse import urlencode
from urllib.request import urlopen, Request
from urllib.error import HTTPError

from flask import Flask, request, redirect, send_from_directory
from google.cloud import datastore

from secret import client_id, client_secret
import config

app = Flask(__name__)
ds_client = datastore.Client()

# These words are used to generate word-based passcode.
# Here are 35 words, so there are 35*34*33*32=1256640 possible 4-word codes.
WORDS = (
    'red green blue white brown violet purple black yellow orange '
    'dog cat cow unicorn animal hedgehog chicken '
    'task computer phone watch android robot apple '
    'rambler rogue warrior king '
    'jeans muffin cake bake cookie oven bread '
).split()

PASSCODE_LIFETIME_MINUTES = 10


def query_json(url, data):
    """
    Query JSON data from Google server using POST request.
    Returns only data and ignores result code.
    """
    if not isinstance(data, (str, bytes)):
        data = urlencode(data).encode('utf-8')
    elif isinstance(data, str):
        data = data.encode('utf-8')
    try:
        return json.loads(urlopen(url, data).read().decode('utf-8'))
    except HTTPError as e:
        return json.loads(e.read().decode('utf-8'))


def json_compactify(data):
    return json.dumps(data, separators=(',', ':'))


def store_passcode(passcode, token_data):
    """Store passcode in Datastore with expiration time."""
    key = ds_client.key('Passcode', passcode)
    entity = datastore.Entity(key=key)
    entity['token_data'] = token_data
    entity['expires_at'] = time.time() + (PASSCODE_LIFETIME_MINUTES * 60)
    ds_client.put(entity)


def get_and_delete_passcode(passcode):
    """Retrieve and delete passcode from Datastore. Returns None if expired or not found."""
    key = ds_client.key('Passcode', passcode)
    entity = ds_client.get(key)
    if entity:
        ds_client.delete(key)
        if entity.get('expires_at', 0) > time.time():
            return entity.get('token_data')
    return None


# Serve static files at root
@app.route('/')
def index():
    return send_from_directory('static', 'index.html')


@app.route('/<path:filename>')
def static_files(filename):
    return send_from_directory('static', filename)


@app.route('/auth')
def auth_redirect():
    """Redirect to Google OAuth."""
    url = 'https://accounts.google.com/o/oauth2/auth?' + urlencode(dict(
        client_id=client_id,
        redirect_uri=config.auth_redir_uri,
        response_type='code',
        scope='https://www.googleapis.com/auth/tasks',
        state='',
        access_type='offline',
        approval_prompt='force',
        include_granted_scopes='true',
    ))
    return redirect(url, code=302)


@app.route('/auth/check', methods=['POST'])
def auth_check():
    """Validate passcode and return tokens."""
    passcode = request.form.get('passcode', '')
    # normalize case and spaces
    passcode = ' '.join(passcode.lower().split())
    token_data = get_and_delete_passcode(passcode)

    response_data = token_data or '{"error": "Incorrect token"}'
    return response_data, 200, {'Content-Type': 'application/json; charset=UTF-8'}


@app.route('/auth/result')
def auth_callback():
    """
    Called by Google when user finished auth process.
    Receives code (if success) or error (if failure).
    Queries Google for access and refresh tokens,
    generates passcode and stores tokens.
    """
    code = request.args.get('code', '')
    q = {
        'code': code,
        'client_id': client_id,
        'client_secret': client_secret,
        'redirect_uri': config.auth_redir_uri,
        'grant_type': 'authorization_code',
    }
    result = query_json('https://accounts.google.com/o/oauth2/token', q)
    if 'access_token' not in result:
        return f'ERROR: {result}', 400

    passcode = ' '.join(random.sample(WORDS, 4))
    passcode2 = str(random.randrange(10**4, 10**5))
    data = json_compactify(result)

    for code in (passcode, passcode2):
        store_passcode(code, data)

    return f'''
    <style>tt {{
      border: 1px solid #88c;
      border-radius: 3px;
      background: #ccf;
      font-size: 20pt;
    }} </style>
    <h3>Passcode: <tt>{passcode}</tt></h3>
    <h3>Or: <tt>{passcode2}</tt></h3>
    <p><em>It will expire in {PASSCODE_LIFETIME_MINUTES} minutes.</em></p>
    <p>Enter it on application settings page</p>
    ''', 200, {'Content-Type': 'text/html'}


@app.route('/auth/refresh')
def auth_refresh():
    """
    Refresh access tokens.
    Access tokens have lifetime of 1 hour, after that they need to be refreshed.
    Receives refresh_token as parameter and returns new access_token.
    """
    refresh_token = request.args.get('refresh_token', '')
    if not refresh_token:
        return '', 400
    q = {
        'refresh_token': refresh_token,
        'client_id': client_id,
        'client_secret': client_secret,
        'grant_type': 'refresh_token',
    }
    result = query_json('https://accounts.google.com/o/oauth2/token', q)
    return json_compactify(result), 200, {'Content-Type': 'application/json; charset=UTF-8'}


if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8080, debug=True)
