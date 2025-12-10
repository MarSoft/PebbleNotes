# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PebbleNotes is a Google Tasks client for Pebble smartwatch. This `gae/` directory contains the Google App Engine backend that handles OAuth2 authentication with Google and serves the web configuration UI.

## Technology Stack

- **Runtime**: Python 3.12 on Google App Engine
- **Framework**: Flask + Gunicorn
- **Storage**: Cloud Datastore (for passcode storage)
- **Frontend**: Static HTML with jQuery Mobile

## Common Commands

**Deploy to App Engine:**
```bash
./deploy.sh prod      # Deploy to production (pebble-notes)
./deploy.sh staging   # Deploy to staging (pebble-notes-staging)
```

Requires `secret-prod.env` and/or `secret-staging.env` with OAuth credentials (see `secret-example.env`).

**Local development:**
```bash
pip install -r requirements.txt
python main.py
```

## Architecture

### Authentication Flow

1. User clicks login on `static/index.html`
2. `/auth` redirects to Google OAuth
3. `/auth/result` receives OAuth callback, generates a 4-word passcode (or 5-digit PIN)
4. Passcode stored in Cloud Datastore for 10 minutes
5. User enters passcode in Pebble app settings
6. App calls `/auth/check` to exchange passcode for tokens
7. `/auth/refresh` handles token refresh (access tokens expire after 1 hour)

### Key Files

- `main.py` - Flask app with OAuth endpoints
- `config.py` - Environment-specific configuration (auto-detects project)
- `secret.py` - OAuth credentials (reads from env vars, not in repo)
- `app.yaml` - GAE routing and runtime config
- `deploy.sh` - Deployment script for prod/staging
- `secret-*.env` - OAuth credentials per environment (not in repo)

### API Endpoints

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/auth` | GET | Redirect to Google OAuth |
| `/auth/result` | GET | OAuth callback, generates passcode |
| `/auth/check` | POST | Exchange passcode for tokens |
| `/auth/refresh` | GET | Refresh expired access token |

## Related Components

The parent directory contains the Pebble smartwatch app:
- `src/` - C source code for Pebble app
- `src/js/main.js` - PebbleKit JavaScript (handles API calls, uses tokens from this backend)
- Build with `pebble build` (requires Pebble SDK)
