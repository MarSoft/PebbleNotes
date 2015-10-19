function getHashParams() {
	var hashParams = {};
	var e,
		a = /\+/g,  // Regex for replacing addition symbol with a space
		r = /([^&;=]+)=?([^&;]*)/g,
		d = function (s) { return decodeURIComponent(s.replace(a, " ")); },
		q = window.location.hash.substring(1);
	while (e = r.exec(q))
		hashParams[d(e[1])] = d(e[2]);
	return hashParams;
}

function getRetUrl() {
	var query = location.search.substring(1);
	var vars = query.split('&');
	for(var i=0; i<vars.length; i++) {
		var pair = vars[i].split('=');
		if(pair[0] == 'return_to')
			return decodeURIComponent(pair[1]);
	}
	return 'pebblejs://close#';
}
function closeConfig(data) {
	data = data || '';
	if(typeof data == 'object')
		data = JSON.stringify(data);
	var url = getRetUrl() + encodeURIComponent(data);
	window.location = url;
}
