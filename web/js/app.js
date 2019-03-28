
function httpRequest(url, callback) {
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() { 
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
			callback(xmlHttp.responseText);
		}
	}
	xmlHttp.open("GET", url, true);
	xmlHttp.send(null);
}

var s = new sigma({
	container: 'container',
	settings: {
		edgeColor: '#000000',
		nodeSize: 2,
		edgeSize: 2,
	},
});

var obj = {

	size_left: 10,
	size_right: 10,
	edge_prob: 0.1,
	generate: function() {
		var data = new URLSearchParams({ action: "random", s: obj.size_left, t: obj.size_right, p: obj.edge_prob });
		sigma.parsers.json("/api?"+data, s, function() { s.refresh(); });
	},

};

var gui = new dat.gui.GUI();

gui.add(obj, 'size_left').min(0).step(1);
gui.add(obj, 'size_right').min(0).step(1);
gui.add(obj, 'edge_prob').min(0).max(1);
gui.add(obj, 'generate');



