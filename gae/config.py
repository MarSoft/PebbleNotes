import os

# get our version id
# version = os.environ['CURRENT_VERSION_ID'].split('.')[0]

environment = os.getenv('ENVIRONMENT', 'local')

# Set the host based on the environment
if environment == 'production':
    host = "https://pebble-notes-426618.uc.r.appspot.com"
else:
    host = "http://127.0.0.1:5000"
# where user will be redirected after logging in with Google
auth_redir_uri = host+"/auth/result"

# uri for settings page (to be used in redirects)
app_config_page = "/notes-config.html"
# where to redirect after successful or failed auth
# (details will be added as a #hashtag)
auth_success_page = "/msg-auth-success.html"
auth_failure_page = "/msg-auth-failed.html"
