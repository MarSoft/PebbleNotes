/**
 * XHR wrapper
 * Usage:
 * ask({
 *   url: ...,
 *   method: null, // default depends on data: GET or POST
 *   data: null, // null => default GET
 *   headers: additional request headers (eg: {'Content-Type': 'text/plain'}
 *   success: function(text, event, xhr){...},
 *   failure: function(code, text, event, xhr){...},
 * });
 */
function ask(o) {
	function p(name, def) {
		return o[name] || def;
	}
	var req = new XMLHttpRequest();
	req.open(p('method', (o.data?'POST':'GET')), o.url, true); // open async
	headers = p('headers', {});
	for(h in headers)
		req.setRequestHeader(h, headers[h]);
	req.onload = function(e) {
		if(req.readyState == 4) {
			text = req.responseText;
			if(req.status == 200) {
				console.log("success\n"+text);
				if(o.success)
					o.success(text, e, req);
			} else {
				console.log("error "+req.status+"\n"+text);
				if(o.failure)
					o.failure(req.status, text, e, req);
			}
		}
	};
	req.send(p('data', null));
}
/**
 * Usage:
 * getJson(url, success_func, failure_func)
 * or
 * getJson(url, success_func, true) to use the same function
 */
function getJson(url, success, failure, headers, method, data) {
	ask({
			url: url,
			headers: headers,
			method: method,
			data: data,
			success: function(text, e, xhr) {
				success(JSON.parse(text), e, xhr);
			},
			failure: function(code, text, e, xhr) {
				if(failure === true)
					success(JSON.parse(text), e, xhr);
				else if(failue) // function
					failure(code, JSON.parse(text), e, xhr);
			}
	});
}

var g_access_token = "";
var g_token_good = false; // TODO

/**
 * send query to googleTasks api
 *
 * endpoint: e.g. users/@me/lists or lists/.../tasks
 * params: {} with all args
 * success: callback(json)
 */
function queryTasks(endpoint, params, success, method, data) {
	url = "https://www.googleapis.com/tasks/v1/" + endpoint;
	sep = "?";
	if(params) { // TODO: urlencoding
		for(p in params)
			url += sep + p + "=" + params[p];
		sep = "&";
	}
	getJson(url, success, null,
		{"Authorization": "Bearer "+g_access_token},
		method, data);
}

/**
 * Checks current g_access_token for validity
 * and requests new if neccessary
 */
function renewToken() {
	// TODO
}

/* Initialization */
Pebble.addEventListener("ready", function(e) {
	console.log("JS is running. Okay.");
	g_access_token = localStorage["access_token"];
	console.log("Access token is "+g_access_token+".\n Testing it!");
	queryTasks("users/@me/lists", null, function(d) {
		console.log("result:"+d);
		l = d.items[2];
		console.log("querying "+l.title);
		queryTasks("lists/"+l.id+"/tasks", null, function(d) {
			console.log("result: "+d);
		});
	});
	console.log("Okay");
});

/* Configuration window */
Pebble.addEventListener("showConfiguration", function(e) {
	console.log("Showing config window... new url");
	Pebble.openURL("http://pebble-notes.appspot.com/v1/notes-config.html?option1=off");
});
Pebble.addEventListener("webviewclosed", function(e) {
	console.log("webview closed: "+e.response);
	result = JSON.parse(e.response);
	if(result.access_token && result.refresh_token) { // assume it was a login session
		console.log("Saving tokens");
		// save tokens
		localStorage["access_token"] = result.access_token;
		localStorate["refresh_token"] = result.refresh_token;
		// todo: maybe save expire time for later checks? (now + value)
		/*console.log("Received tokens. Testing...");
		getJson("https://www.googleapis.com/oauth2/v1/tokeninfo?id_token="+result.access_token, function(r) {
			console.log(r);
		}, true);*/
	}
});

/* Messages */
Pebble.addEventListener("appmessage", function(e) {
	console.log("Received message: " + e.payload);
});
