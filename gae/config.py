import os

# In Python 3 GAE, use GAE_VERSION instead of CURRENT_VERSION_ID
# For OAuth redirects, use the main service URL for stability
version = os.environ.get('GAE_VERSION', 'dev')

# Detect project and set host accordingly
project = os.environ.get('GOOGLE_CLOUD_PROJECT', 'pebble-notes')
host = f"https://{project}.appspot.com"

# where user will be redirected after logging in with Google
auth_redir_uri = host + "/auth/result"

# uri for settings page (to be used in redirects)
app_config_page = "/notes-config.html"
# where to redirect after successful or failed auth
# (details will be added as a #hashtag)
auth_success_page = "/msg-auth-success.html"
auth_failure_page = "/msg-auth-failed.html"
