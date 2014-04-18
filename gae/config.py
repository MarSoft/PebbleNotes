import os

# get our version id
version = os.environ['CURRENT_VERSION_ID'].split('.')[0]

# construct hostname for current version
host = "https://%s.pebble-notes.appspot.com" % version

# where user will be redirected after logging in with Google
auth_redir_uri = host+"/auth/result"

# uri for settings page (to be used in redirects)
app_config_page = "/notes-config.html"
# where to redirect after successful or failed auth
# (details will be added as a #hashtag)
auth_success_page = "/msg-auth-success.html"
auth_failure_page = "/msg-auth-failed.html"
