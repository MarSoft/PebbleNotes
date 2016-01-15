var config = {
};

// Timeout for (any) http requests, in milliseconds
config.xhr_timeout = 10000;
// Timeout for sending appmessage to Pebble, in milliseconds
config.msg_timeout = 8000;

config.server_url = "https://1-dot-pebble-notes.appspot.com";

// here are default values
config.options = {
	sort_status: false,
	sort_date: false, // false, "asc", "desc"
	sort_due: false, // false, "asc", "desc"
	sort_alpha: false,
};
/**
 * Load options values from localStorage, if present.
 */
config.load_options = function() {
	var options = config.options;
	for(var key in options) {
		if(localStorage[key] !== undefined) {
			options[key] = localStorage[key];
			if(options[key] == 'true')
				options[key] = true;
			if(options[key] == 'false')
				options[key] = false;
		}
		console.log(key+": "+options[key]);
	}
};

config.init = function() {
	config.load_options();
};


module.exports = config;
