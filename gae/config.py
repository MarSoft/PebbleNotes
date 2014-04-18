import os

# get our version id
version = os.environ['CURRENT_VERSION_ID'].split('.')[0]
# construct hostname for current version
host = "https://%s.pebble-notes.appspot.com" % version

auth_redir_uri = host+"/auth/result"
app_config_uri = host+"/notes-config.html"
auth_success_page = host+"/msg-auth-success.html"
auth_failure_page = host+"/msg-auth-failed.html"
