Pebble.addEventListener("ready", function(e) {
	console.log("JS is running. Okay.");
});

Pebble.addEventListener("showConfiguration", function(e) {
	console.log("Showing config window... new url");
	Pebble.openURL("http://pebble-notes.appspot.com/v1/notes-config.html?option1=off");
});
Pebble.addEventListener("webviewclosed", function(e) {
	console.log("webview closed: "+e.response);
});
