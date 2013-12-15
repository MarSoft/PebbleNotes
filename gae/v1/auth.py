import webapp2
from urllib import urlencode
import json, urllib2

from secret import client_id, client_secret
import config

class AuthRedirector(webapp2.RequestHandler):
    def get(self):
        args = self.request.GET
        args["client_id"] = client_id
        args["redirect_uri"] = config.auth_redir_uri
        url = "https://accounts.google.com/o/oauth2/auth?"+urlencode(args)
        self.response.location = url
        self.response.status_int = 302

class AuthCallback(webapp2.RequestHandler):
    def get(self):
        # in case of error

        state = self.request.get("state")
        code = self.request.get("code")
        error = self.request.get("error")

        q = {
            "code": code,
            "client_id": client_id,
            "client_secret": client_secret,
            "redirect_uri": config.auth_redir_uri,
            "grant_type": "authorization_code",
        }
        url = "https://accounts.google.com/o/oauth2/token"
        print urlencode(q)
        try:
            result = json.loads(urllib2.urlopen(url, urlencode(q)).read())
            self.response.headers['Content-Type'] = 'application/json; charset=UTF-8'
            self.response.write(result)
        except urllib2.HTTPError as e:
            self.response.write("Error: " + str(e) + "\n" + e.read())
            self.response.status_int = 500

application = webapp2.WSGIApplication([
    ('/v1/auth', AuthRedirector),
    ('/v1/auth/result', AuthCallback),
], debug=True)
