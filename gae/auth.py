import webapp2
from urllib import urlencode
import json
import urllib2

from secret import client_id, client_secret
import config

def query_json(url, data):
    """
    Query JSON data from Google server using POST request.
    Returns only data and ignores result code.
    """
    if not isinstance(data, str):
        data = urlencode(data)
    try:
        return json.loads(urllib2.urlopen(url, data).read())
    except urllib2.HTTPError as e:  # exception is a file-like object
        return json.loads(e.read())

AUTHPAGE = """
    <!DOCTYPE html>
    <html>
    <head>
        <meta http-equiv="refresh" content="{interval}{url}">
    </head>
    <body>
        <h1>Authentication</h1>
        <p>Please go to <tt>{verification_url}</tt> in your browser
        and enter this user code: <strong>{user_code}</strong>.</p>
    </body>
    </html>
"""

class AuthCodeHandler(webapp2.RequestHandler):
    def get(self):
        if not self.request.GET.get('user_code'):
            # first request
            ask = query_json(
                'https://accounts.google.com/o/oauth2/device/code',
                dict(
                    client_id=client_id,
                    scope='email profile https://www.googleapis.com/auth/tasks',
                )
            )
            if 'user_code' not in ask:
                self.response.write('ERROR: %s' % ask)
                return
            # first set it to empty
            ask['url'] = ''
            # now set it to meaningful, but with empty url value
            ask['url'] = '; /auth?' + urlencode(ask)
        else:
            # poll request
            ask = self.request.GET
            result = query_json(
                'https://www.googleapis.com/oauth2/v4/token',
                dict(
                    client_id=client_id,
                    client_secret=client_secret,
                    code=ask['device_code'],
                    grant_type='http://oauth.net/grant_type/device/1.0',
                ),
            )
            if result.get('error') != 'authorization_pending':
                # done! Redirect to resulting page
                url = (config.auth_success_page if "access_token" in result
                       else config.auth_failure_page) + "#" + urlencode(result)
                self.response.location = url
                self.response.status_int = 302
                return

        self.response.headers['Content-Type'] = 'text/html; charset=UTF-8'
        self.response.write(AUTHPAGE.format(**ask))

class AuthPoller(webapp2.RequestHandler):
    def get(self):
        args = self.request.GET
        args["client_id"] = client_id
        args["redirect_uri"] = config.auth_redir_uri
        url = "https://accounts.google.com/o/oauth2/auth?"+urlencode(args)
        self.response.location = url
        self.response.status_int = 302

def json_compactify(data):
    return json.dumps(data, separators=(',', ':'))  # compact encoding

class AuthCallback(webapp2.RequestHandler):
    """
    This page is called by Google when user finished auth process.
    It receives state (currently unused), code (if success)
    or error (if failure).
    Then it queries Google for access and refresh tokens
    and passes them in urlencode form
    to intermediate static page, which will show status
    and pass data to js code.
    """
    def get(self):
        #state = self.request.get("state")
        code = self.request.get("code")
        #error = self.request.get("error")
        q = {
            "code": code,
            "client_id": client_id,
            "client_secret": client_secret,
            "redirect_uri": config.auth_redir_uri,
            "grant_type": "authorization_code",
        }
        result = query_json("https://accounts.google.com/o/oauth2/token", q)
        url = (config.auth_success_page if "access_token" in result
               else config.auth_failure_page) + "#" + urlencode(result)
        self.response.location = url
        self.response.status_int = 302
        # return result as redirect to static page

class AuthRefresh(webapp2.RequestHandler):
    """
    This page is used by client to refresh their access tokens.
    An access token has lifetime of 1 hour, after that it becomes invalid and
    needs to be refreshed.
    So we receive refresh_token as a parameter
    and return a new access_token with its lifetime as a json result.
    """
    def get(self):
        refresh_token = self.request.get("refresh_token")
        if not refresh_token:
            self.response.status_int = 400
            return
        q = {
            "refresh_token": refresh_token,
            "client_id": client_id,
            "client_secret": client_secret,
            "grant_type": "refresh_token",
        }
        result = query_json("https://accounts.google.com/o/oauth2/token", q)
        self.response.headers['Content-Type'] = \
            "application/json; charset=UTF-8"
        self.response.write(json_compactify(result))
        # return result as JSON

application = webapp2.WSGIApplication([
    ('/auth', AuthCodeHandler),
    ('/auth/result', AuthCallback),
    ('/auth/refresh', AuthRefresh),
], debug=True)
