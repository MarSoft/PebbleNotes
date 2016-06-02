// Timeout for (any) http requests, in milliseconds
var g_xhr_timeout = 10000;
// Timeout for sending appmessage to Pebble, in milliseconds
var g_msg_timeout = 8000;

var g_server_url = "https://1-dot-pebble-notes.appspot.com";

var g_options = {
	"sort_status": false,
	"sort_date": false, // false, "asc", "desc"
	"sort_due": false, // false, "asc", "desc"
	"sort_alpha": false,
	"large_font": false,
	"task_actions_position": 2, // 0=off, 1=top, 2=bottom
};

var g_option_ids = {
	large_font: 1,
	task_actions_position: 2,
};

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
	for(var h in headers)
		req.setRequestHeader(h, headers[h]);
	req.onload = function(e) {
		if(req.readyState == 4) {
			clearTimeout(xhrTimeout); // got response, no more need in timeout
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
	var xhrTimeout = setTimeout(function() {
		req.abort();
		displayError("Request timed out");
	}, g_xhr_timeout);
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
	var o = 
	{
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
	};
	//DEBUG: console.log("asking:"+JSON.stringify(o));
	ask(o);
}

var Timeline = {
	usertoken: null,
	init: function() {
		Pebble.getTimelineToken(function(token) {
			console.log('Timeline token acquired');
			Timeline.usertoken = token;
		}, function(err) {
			console.log('Failed to acquire timeline token: '+err);
			Timeline.usertoken = false;
		});
	},
	act: function(method, type, id, data, success, failure) {
		var headers = {};
		if(data)
			headers['Content-Type'] = 'application/json';
		if(type == 'user') {
			if(!Timeline.usertoken) {
				if(failure)
					failure('no user timeline token');
				return false;
			}
			headers['X-User-Token'] = Timeline.usertoken;
		} else if(type == 'shared') {
			// TODO
			console.warning('shared are not implemented');
		} else {
			console.error('invalid type! '+type);
			return;
		}
		return ask({
			method: method,
			url: 'https://timeline-api.getpebble.com/v1/'+type+'/pins/'+id,
			data: data,
			headers: headers,
			success: success,
			failure: failure,
		});
	},
	user_put: function(pin, success, failure) {
		return this.act('PUT', 'user', pin.id, JSON.stringify(pin),
				success, failure);
	},
	user_delete: function(id, success, failure) {
		return this.act('DELETE', 'user', id, null,
				success, failure);
	},
};

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
function queryTasks(endpoint, params, success, method, send_data) {
	var url = "https://www.googleapis.com/tasks/v1/" + endpoint;
	var sep = "?";
	if(params) {
		for(var p in params) {
			url += sep + encodeURIComponent(p) + "=" + encodeURIComponent(params[p]);
			sep = "&";
		}
	}
	var headers = {"Authorization": "Bearer "+g_access_token,
		"Content-Type": "application/json"};
	getJson(url, success, function(code, data) {
		if(code == 401) { // Invalid Credentials
			console.log("Renewing token and retrying...");
			renewToken(function() { // renew, and on success -
				headers.Authorization = "Bearer "+g_access_token; // the new one
				getJson(url, success, function(code, data) {
					console.log("Renewal didn't help! "+code+": "+data.error.message);
					displayError(data.error.message, code);
				}, headers, method, send_data);
			});
		} else {
			displayError(data.error.message, code);
		}
	}, headers, method, send_data);
}

/**
 * Checks current g_access_token for validity (TODO)
 * and requests new if neccessary,
 * after which calls Success callback.
 * [Now just requests new token and saves it]
 * In case of error it will call displayError.
 */
function renewToken(success) {
	console.log("Renewing token!");
	refresh_token = g_refresh_token;
	if(!refresh_token) {
		displayError("No refresh token; please log in!", 401);
		return;
	}
	getJson(g_server_url+"/auth/refresh?refresh_token="+encodeURIComponent(refresh_token),
		function(data) { // success
			console.log("Renewed. "+JSON.stringify(data));
			if("access_token" in data) {
				localStorage.access_token = data.access_token;
				g_access_token = data.access_token;
				success(g_access_token);
			} else if("error" in data) {
				displayError("Auth error: " + data.error + "\nPlease open Pebble app and log in again!");
			} else {
				displayError("No access token received from Google!"); // unlikely...
			}
		},
		function(code, data) { // failure
			displayError(data.error.message, code);
		});
}

/**
 * Obfuscates token for logging purposes
 */
function hideToken(token) {
	if(typeof token != "string")
		return token;
	return token.substring(0, 5) + "...";
}

/**
 * Sends an error packet to Pebble
 * code may be null
 */
function displayError(text, code) {
	if(code) text += " (" + code + ")";
	console.log("Sending error msg to Pebble: " + text);
	sendMessage({
			code: 50, // Error
			error: text
	});
}
function assert(val, message) {
	if(!val) {
		console.log("assertion failed");
		displayError("assertion failed"+(message?": ":"")+message);
	}
}

var g_msg_buffer = [];
var g_msg_transaction = null;

/**
 * Sends appMessage to pebble; logs errors.
 * failure: may be True to use the same callback as for success.
 */
function sendMessage(data, success, failure) {
	function sendNext() {
		g_msg_transaction = null;
		next = g_msg_buffer.shift();
		if(next) { // have another msg to send
			sendMessage(next);
		}
	}
	if(g_msg_transaction) { // busy
		g_msg_buffer.push(data);
	} else { // free
		g_msg_transaction = Pebble.sendAppMessage(data,
			function(e) {
				console.log("Message sent for transactionId=" + e.data.transactionId);
				clearTimeout(msgTimeout);
				if(g_msg_transaction >= 0 && g_msg_transaction != e.data.transactionId) // -1 if unsupported
					console.log("### Confused! Message sent which is not a current message. "+
							"Current="+g_msg_transaction+", sent="+e.data.transactionId);
				if(success)
					success();
				sendNext();
			},
		   	function(e) {
				console.log("Failed to send message for transactionId=" + e.data.transactionId +
						", error is "+(e.error && "message" in e.error ? e.error.message : "(none)"));
				clearTimeout(msgTimeout);
				if(g_msg_transaction >= 0 && g_msg_transaction != e.data.transactionId)
					console.log("### Confused! Message not sent, but it is not a current message. "+
							"Current="+g_msg_transaction+", unsent="+e.data.transactionId);
				if(failure === true) {
					if(success)
						success();
				} else if(failure)
					failure();
				sendNext();
			}
		);
		if(!g_msg_transaction) { // buggy sendAppMessage: on iOS returns undefined, on emulator returns null
			g_msg_transaction = -1; // just a dummy "non-false" value for sendNext and friends
		}
		var msgTimeout = setTimeout(function() {
			console.log("Message timeout! Sending next.");
			// FIXME: it could be really delivered. Maybe add special handler?
			if(failure === true) {
				if(success)
					success();
			} else if(failure) {
				failure();
			}
			sendNext();
		}, g_msg_timeout);
		console.log("transactionId="+g_msg_transaction+" for msg "+JSON.stringify(data));
	}
}
var g_ready = false;
/**
 * Send "ready" msg to watchapp: we are now ready to return data from google
 */
function ready() {
	if(g_ready) // already sent
		return;
	sendMessage({code: 0});
	// and send indexed options
	// to ensure Pebble knows their correct values
	for(var k in g_option_ids) {
		sendMessage({
			code: 45, // update option
			option_id: g_option_ids[k],
			option_value: g_options[k],
		});
	}
	g_ready = true;
}

var g_tasklists = [];

/**
 * Compare two strings; for array sorting.
 */
function strcmp(a, b) {
	if(a<b) return -1;
	if(a>b) return 1;
	return 0;
}

/* Main logic */
function doGetAllLists() {
	console.log("Querying all tasklists");
	g_tasklists = []; // TODO: use it for caching
	function haveSomeLists(d) {
		// this function receives current page
		// and then either queries for next one
		// or saves all gathered items to the watch.

		console.log("sending " + d.items.length + " items");
		// Add all tasklists from the current page to g_tasklists variable
		for(var i=0; i<d.items.length; i++) {
			var l = d.items[i];
			g_tasklists.push({
					id: l.id,
					title: l.title,
			});
		}
		if(d.nextPageToken) { // have next page?
			// query it!
			queryTasks("users/@me/lists",
				   {
					   pageToken: d.nextPageToken
				   },
				   haveSomeLists); // get next page and pass it to this same function
			// and stop for now
			return;
		}

		g_tasklists.sort(function(a, b) {
			return strcmp(a.title, b.title);
		});
		sendMessage({
				code: 20, // array start/size
				scope: 0,
				count: g_tasklists.length});
		for(i=0; i<g_tasklists.length; i++) {
			console.log("Sending item: " + JSON.stringify(g_tasklists[i]));
			sendMessage({
					code: 21, // array item
					scope: 0,
					item: i,
					listId: i,
					title: g_tasklists[i].title});
		}
		sendMessage({
				code: 22, // array end
				scope: 0,
				count: g_tasklists.length}); // send resulting list length, just for any
		console.log("sending finished");
	}
	queryTasks("users/@me/lists", null, haveSomeLists); // get first page
}
function createTaskObjFromGoogle(t) {
	return {
		id: t.id,
		position: t.position,
		done: t.status == "completed",
		title: t.title,
		hasNotes: "notes" in t,
		notes: t.notes,
		updated: t.updated,
		due: t.due,
	};
}
function manageTaskPin(task) {
	if(!task.due) {
		// For tasks which have no due date
		// we delete pin which was probably created before.
		// FIXME: don't delete unless was previously created
		console.log('deleting pin (if any)');
		Timeline.user_delete(task.id);
		return;
	}
	// FIXME: don't re-create pin if already exists?
	var d = new Date(task.due);
	var time = Date.parse(task.due) + d.getTimezoneOffset()*60000; // minutes to ms
	time = new Date(time).toISOString();
	console.log('time: was '+task.due+', will be '+time);
	console.log('adding pin');
	Timeline.user_put({
		id: task.id,
		time: time,
		layout: {
			type: 'genericPin',
			title: task.title,
			subtitle: task.done ? 'Already done' : 'Due date',
			tinyIcon: 'system://images/SCHEDULED_EVENT',
			body: task.notes, // TODO: maybe more info?
		},
		actions: [
			{
				title: 'Open app',
				type: 'openWatchApp',
				launchCode: 0, // TODO?.. can we pass task id here?..
			},
		],
	});
}
function doGetOneList(listId) {
	assert(listId in g_tasklists, "No such list!");
	var realId = g_tasklists[listId].id;
	queryTasks("lists/"+realId+"/tasks", null, function(d) {
		// FIXME: support more than 100 tasks (by default Google returns only 100)
		if(d.nextPageToken)
			displayError("There are more tasks than we can process");
		console.log("sending " + d.items.length + " items");
		var tasks = g_tasklists[listId].tasks = []; // TODO: use it for caching
		for(var i=0; i<d.items.length; i++) {
			var l = d.items[i];
			var task = createTaskObjFromGoogle(l);
			tasks.push(task);
			manageTaskPin(task);
			// TODO: use cached version to determine deleted tasks
		}
		var comparator = function(a, b) {
			if(g_options.sort_status && a.done != b.done)
				return a.done ? 1 : -1; // move finished tasks to end
			var ret = 0;
			if(g_options.sort_date) {
				ret = strcmp(a.updated, b.updated);
				if(g_options.sort_date == "desc")
					ret *= -1; // reverse order - newest first
				if(ret !== 0)
					return ret;
			}
			if(g_options.sort_due) {
				if(a.due && b.due) {
					ret = strcmp(a.due, b.due);
					if(g_options.sort_due == "desc")
						ret *= -1; // reverse order - newest first
				} else if(a.due || b.due) {
					ret = a.due ? -1 : 1; // move tasks with due available date to top
				}
				if(ret !== 0)
					return ret;
			}
			if(g_options.sort_alpha) {
				ret = strcmp(a.title, b.title);
				if(ret !== 0)
					return ret;
			}
			return strcmp(a.position, b.position);
		};
		tasks.sort(comparator);
		if(tasks[tasks.length-1].title === "" && !tasks[tasks.length-1].done) // if last task is empty and not completed
			tasks.pop(); // don't show it
		sendMessage({
				code: 20, // array start/size
				scope: 1,
				listId: listId,
				count: tasks.length});
		for(i=0; i<tasks.length; i++) {
			console.log("Sending item: " + JSON.stringify(tasks[i]));
			sendMessage({
					code: 21, // array item
					scope: 1,
					item: i,
					taskId: i,
					isDone: tasks[i].done?1:0,
					title: tasks[i].title,
					hasNotes: tasks[i].hasNotes?1:0,
					notes: tasks[i].notes
			});
		}
		sendMessage({
				code: 22, // array end
				scope: 1,
				listId: listId,
				count: tasks.length}); // send resulting list length, just for any
		console.log("sending finished");
	});
}
function doGetTaskDetails(taskId) {
	assert(false, "Not implemented yet");
}
function doUpdateTaskStatus(listId, taskId, isDone) {
	assert(listId in g_tasklists, "No such list!");
	var list = g_tasklists[listId];
	assert(taskId in list.tasks, "No such task!");
	var task = list.tasks[taskId];
	var taskobj = {
		status: (isDone?"completed":"needsAction"),
		completed: null // Google will replace with "now" date if needed
	};
	var taskJson = JSON.stringify(taskobj);
	console.log("New task data: "+taskJson);
	queryTasks("lists/"+list.id+"/tasks/"+task.id, null, function(d) {
		console.log("Received: "+JSON.stringify(d));
		assert(d.id == task.id, "Task ID mismatch!!?");
		task = createTaskObjFromGoogle(d); // TODO: maybe not create new but only update?
		assert(list.tasks[taskId].id == task.id, "Task ID or position mismatch!!?");
		list.tasks[taskId] = task;
		sendMessage({
				code: 23, // item updated
				scope: 2, // task
				listId: listId,
				taskId: taskId,
				isDone: task.done?1:0
		});
	}, "PATCH", taskJson);
}
function doCreateTask(listId, task, parentTask, prevTask) {
	assert(listId in g_tasklists, "No such list!");
	var list = g_tasklists[listId];
	for(var k in task) {
		if(task[k] === null)
			delete task[k];
	}
	assert(task.title, 'Title is required!');
	var taskJson = JSON.stringify(task);
	var params = null;
	if(parentTask || prevTask) {
		params = {};
		if(parentTask)
			params.parent = list.tasks[parentTask].id;
		if(prevTask)
			params.previous = list.tasks[prevTask].id;
	}
	queryTasks('lists/'+list.id+'/tasks', params, function(d) {
		// success
		console.log("Received: "+JSON.stringify(d));
		task = createTaskObjFromGoogle(d);
		list.tasks.push(task);
		var taskId = list.tasks.length-1;
		sendMessage({
				code: 24, // item added
				scope: 1, // tasklist / tasks
				listId: listId,
				taskId: taskId,
				title: task.title,
				notes: task.notes,
				isDone: task.done?1:0,
		});
	}, 'POST', taskJson);
}

/* Initialization */
Pebble.addEventListener("ready", function(e) {
	console.log("JS is running. Okay.");
	Timeline.init();

	g_access_token = localStorage.access_token;
	g_refresh_token = localStorage.refresh_token;
	for(var key in g_options) {
		if(localStorage[key] !== undefined && localStorage[key] !== null) {
			g_options[key] = localStorage[key];
			if(g_options[key] == 'true')
				g_options[key] = true;
			if(g_options[key] == 'false')
				g_options[key] = false;
		}
		console.log(key+": "+g_options[key]);
	}
	console.log("access token (from LS): "+g_access_token);
	console.log("refresh token (from LS): "+hideToken(g_refresh_token));

	if(g_refresh_token) // check on refresh token, as we can restore/renew access token later with it
		ready(); // ready: tell watchapp that we are ready to communicate
	else { // try to retrieve it from watchapp
		console.log("No refresh token, trying to retrieve");
		sendMessage({ code: 41 }, // retrieve token
			false, // on success just wait for reply
			function() { // on sending failure tell user to login; although error message is unlikely to pass
				displayError("No refresh token stored, please log in!", 401); // if no code, tell user to log in
			}
		);
	}
});

/* Configuration window */
Pebble.addEventListener("showConfiguration", function(e) {
	console.log("Showing config window...");
	opts = {"access_token": (g_access_token === undefined ? "" : g_access_token)};
	for(var key in g_options)
		opts[key] = g_options[key];
	var url = g_server_url+"/notes-config.html#"+
		encodeURIComponent(JSON.stringify(opts));
	console.log("URL: "+url);
	var result = Pebble.openURL(url);
	console.log("Okay. "+result);
});
Pebble.addEventListener("webviewclosed", function(e) {
	console.log("webview closed: "+e.response);
	result = {};
	try {
		if(('response' in e) && e.response) // we don't want to parse 'undefined'
			result = JSON.parse(decodeURIComponent(e.response));
	} catch(ex) {
		console.log("Parsing failed: "+ex+"\n");
	}
	if("access_token" in result && "refresh_token" in result) { // assume it was a login session
		console.log("Saving tokens");
		// save tokens
		if(result.access_token) {
			localStorage.access_token = g_access_token = result.access_token;
			console.log("Access token: " + g_access_token);
		}
		if(result.refresh_token) {
			localStorage.refresh_token = g_refresh_token = result.refresh_token;
			console.log("Refresh token saved: " + hideToken(g_refresh_token));
		}
		// TODO: maybe save expire time for later checks? (now + value)
		// now save tokens in watchapp:
		sendMessage({
			code: 40, // save_token
			access_token: g_access_token,
			refresh_token: g_refresh_token
		});
		ready(); // tell watchapp that we are now ready to work
	} else if("logout" in result) {
		console.log("Logging out");
		g_access_token = localStorage.access_token = '';
		g_refresh_token = localStorage.refresh_token = '';
		sendMessage({ code: 40 }); // remove credentials
	} else { // settings saved, update
		console.log("Updating settings");
		for(var key in g_options) {
			if(result[key] !== undefined) {
				if(result[key] == "on")
					result[key] = true;
				else if(result[key] == "off")
					result[key] = false;
				else if(!isNaN(parseInt(result[key])))
					result[key] = parseInt(result[key]);

				if(g_option_ids[key] && result[key] != g_options[key]) {
					console.log('Option '+key+' changed to '+result[key]+', notifying watch');
					sendMessage({
						code: 45, // update option
						option_id: g_option_ids[key],
						option_value: result[key], // TODO adapt?
					});
				}

				localStorage[key] = g_options[key] = result[key];
			}
		}
		console.log(JSON.stringify(g_options));
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
			assert('listId' in e.payload, "List ID was not provided for GetOneList query");
			doGetOneList(e.payload.listId);
			break;
		case 2: // one task
			assert('taskId' in e.payload, "Task ID was not provided for GetTaskDetails query");
			doGetTaskDetails(e.payload.taskId);
			break;
		default:
			console.log("Unknown message scope "+e.payload.scope);
			break;
		}
		break;
	case 11: // update info
		switch(e.payload.scope) {
		case 2: // one task (here - "done" status)
			assert('listId' in e.payload, "List ID was not provided for ChangeTaskStatus query");
			assert('taskId' in e.payload, "Task ID was not provided for ChangeTaskStatus query");
			assert('isDone' in e.payload, "New task status was not provided for ChangeTaskStatus query");
			doUpdateTaskStatus(e.payload.listId, e.payload.taskId, e.payload.isDone);
			break;
		case 0: // all lists
		case 1: // one list
			console.log("Cannot 'update' info for scope "+e.payload.scope+" [yet]");
			break;
		default:
			console.log("Unknown message scope "+e.payload.scope);
			break;
		}
		break;
	case 12: // post info
		switch(e.payload.scope) {
		case 2: // one task (here - create new task)
			assert('listId' in e.payload, 'List ID was not profided for PostTask query');
			assert('title' in e.payload, 'Task title was not provided for PostTask query');
			doCreateTask(e.payload.listId, {
				// TODO: allow specifying prev/parent?
				title: e.payload.title,
				notes: e.payload.notes,
				status: e.payload.isDone ? 'completed' : (
						e.payload.isDone === false ? 'needsAction' : null),
			});
			break;
		default:
			console.log('Unknown message scope '+e.payload.scope);
			break;
		}
		break;
	case 41: // retrieve token - reply received
		if("access_token" in e.payload)
			g_access_token = e.payload.access_token;
		if("refresh_token" in e.payload)
			g_refresh_token = e.payload.refresh_token;
		if(g_refresh_token) { // it's possible to refresh access token if it was not provided
			console.log("Retrieved tokens from watch: "+g_access_token+", "+hideToken(g_refresh_token));
			// save them (again)
			localStorage.access_token = g_access_token;
			localStorage.refresh_token = g_refresh_token;
			ready(); // ready at last
		} else { // no tokens here nor on watch
			displayError("Please open settings and log in!"); // if no code, tell user to log in
		}
		break;
	default:
		console.log("Unknown message code "+e.payload.code);
		break;
	}
});
