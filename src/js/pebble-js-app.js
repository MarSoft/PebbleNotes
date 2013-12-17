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
				console.log("xhr:success");
				if(o.success)
					o.success(text, e, req);
			} else {
				console.log("xhr:error "+req.status+"\n"+text);
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
 *
 * success(data, event, xhr)
 * failure(err_code, data, event, xhf)
 */
function getJson(url, success, failure, headers, method, data) {
	ask({
			url: url,
			headers: headers,
			method: method,
			data: data,
			success: function(text, e, xhr) {
				if(success)
					success(JSON.parse(text), e, xhr);
			},
			failure: function(code, text, e, xhr) {
				if(failure === true) {
					if(success)
						success(JSON.parse(text), e, xhr);
				} else if(failure) // function
					failure(code, JSON.parse(text), e, xhr);
			}
	});
}

var g_access_token = "";
var g_refresh_token = "";

/**
 * send query to googleTasks api.
 * Will automatically send error message to Pebble
 * if anything goes wrong.
 *
 * endpoint: e.g. users/@me/lists or lists/.../tasks
 * params: {} with all args
 * success: callback(json)
 * method: get or post or something else (defaults to Get)
 * data: n/a for get method
 */
function queryTasks(endpoint, params, success, method, data) {
	url = "https://www.googleapis.com/tasks/v1/" + endpoint;
	sep = "?";
	if(params) {
		for(p in params)
			url += sep + encodeURIComponent(p) + "=" + encodeURIComponent(params[p]);
		sep = "&";
	}
	headers = {"Authorization": "Bearer "+g_access_token};
	getJson(url, success, function(code, data) {
		if(code == 401) { // Invalid Credentials
			console.log("Renewing token and retrying...");
			renewToken(function() { // renew, and on success -
				getJson(url, success, function(code, data) {
					console.log("Renewal didn't help! Code "+code);
					displayError(data.error.message, code);
				}, headers, method, data);
			});
		} else {
			displayError(data.error.message, code);
		}
	}, headers, method, data);
}

/**
 * Checks current g_access_token for validity (TODO)
 * and requests new if neccessary,
 * after which calls Success callback.
 * [Now just requests new token and saves it]
 * In case of error it will call displayError.
 */
function renewToken(success) {
	refresh_token = localStorage["refresh_token"];
	if(!refresh_token) {
		displayError("No refresh token; please log in!", 401);
		return;
	}
	getJson("https://pebble-notes.appspot.com/v1/auth/refresh?refresh_token="+encodeURIComponent(refresh_token),
		function(data) { // success
			if("access_token" in data) {
				g_access_token = localStorage["access_token"] = data.access_token;
				success(g_access_token);
			} else if("error" in data) {
				displayError(data.error);
			} else {
				displayError("No access token!");
			}
		},
		function(code, data) { // failure
			displayError(data.error.message, code);
		});
}

/**
 * Sends an error packet to Pebble
 * code may be null
 */
function displayError(text, code) {
	if(code) text += " (" + code + ")";
	console.log("Sending error msg to Pebble (Not implemented): " + text);
	// TODO
}
function assert(val, message) {
	if(!val) {
		console.log("assertion failed");
		displayError(message);
	}
}

/**
 * Sends appMessage to pebble; logs errors.
 */
function sendMessage(data) {
	Pebble.sendAppMessage(data,
		function(e) {
			console.log("Message sent: " + e.data);
		},
	   	function(e) {
			console.log("Failed to send message: transactionId=" + e.data.transactionId + ", error is "+e.error.message);
		});
}

var g_tasklists = [];

/* Main logic */
function doGetAllLists() {
	console.log("Querying all tasklists");
	queryTasks("users/@me/lists", null, function(d) {
		console.log("sending " + d.items.length + " items");
		sendMessage({
				code: 20, // array start/size
				scope: 0,
				count: d.items.length});
		g_tasklists = []; // TODO: use it for caching
		for(var i=0; i<d.items.length; i++) {
			var l = d.items[i];
			var lobj = {
				id: l.id,
				title: l.title,
				size: -1
			};
			var id = g_tasklists.push(lobj);
			sendMessage({
					code: 21, // array item
					scope: 0,
					item: i,
					listId: id,
					title: lobj.title,
					size: lobj.size}); // TODO
		}
		sendMessage({
				code: 22, // array end
				scope: 0,
				count: g_tasklists.length}); // send resulting list length, just for any
		console.log("sending finished");
	});
}
function doGetOneList(listId) {
	assert(false, "Not implemented yet");
	queryTasks("lists/"+l.id+"/tasks", null, function(d) {
		console.log("result: "+d);
	});
}
function doGetTaskDetails(taskId) {
	assert(false, "Not implemented yet");
}
function doChangeTaskStatus(taskId, isDone) {
	assert(false, "Not implemented yet");
}

/* Initialization */
Pebble.addEventListener("ready", function(e) {
	console.log("JS is running. Okay.");
	g_access_token = localStorage["access_token"];
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
		if(result.access_token) {
			localStorage["access_token"] = result.access_token;
			console.log("Access token: " + localStorage["access_token"]);
		}
		if(result.refresh_token) {
			localStorage["refresh_token"] = result.refresh_token;
			console.log("Refresh token saved: " + localStorage.refresh_token);
		}
		// todo: maybe save expire time for later checks? (now + value)
		/*console.log("Received tokens. Testing...");
		getJson("https://www.googleapis.com/oauth2/v1/tokeninfo?id_token="+result.access_token, function(r) {
			console.log(r);
		}, true);*/
	}
});

/* Messages */
Pebble.addEventListener("appmessage", function(e) {
	console.log("Received message: " + JSON.stringify(e.payload));
	switch(e.payload.code) {
	case 10: // get info
		switch(e.payload.scope) {
		case 0: // all lists
			doGetAllLists();
			break;
		case 1: // one list
			assert(e.payload.listId, "List ID was not provided for GetOneList query");
			doGetOneList(e.payload.listId);
			break;
		case 2: // one task
			assert(e.payload.taskId, "Task ID was not provided for GetTaskDetails query");
			doGetTaskDetails(e.payload.taskId);
			break;
		default:
			console.log("Unknown message scope "+e.payload.scope);
			break;
		}
		break;
	case 11: // change info
		switch(e.payload.scope) {
		case 2: // one task (here - "done" status)
			assert(e.payload.taskId, "Task ID was not provided for ChangeTaskStatus query");
			assert(e.payload.isDone, "New task status was not provided for ChangeTaskStatus query");
			doChangeTaskStatus(e.payload.taskId, e.payload.isDone);
			break;
		case 0: // all lists
		case 1: // one list
			console.log("Cannot 'change' info for scope "+e.payload.scope);
			break;
		default:
			console.log("Unknown message scope "+e.payload.scope);
			break;
		}
		break;
	default:
		console.log("Unknown message code "+e.payload.code);
		break;
	}
});
