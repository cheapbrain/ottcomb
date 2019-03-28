
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
	renderer: {
		container: 'container',
		type: 'canvas',
	},
	settings: {
		edgeColor: '#000000',
		minEdgeSize: 2,
		maxEdgeSize: 10,
		minNodeSize: 0.5,
		maxNodeSize: 20,
	},
});

var obj = {

	S: 10,
	T: 10,
	E: 10,
	generate: function() {
		var data = new URLSearchParams({ action: "random", s: obj.S, t: obj.T, e: obj.E });
		sigma.parsers.json("/api?"+data, s, function() { 
			var edges = s.graph.edges();
			console.log(edges.length / (obj.size_left * obj.size_right));
			for (var i = 0; i < edges.length; i++) {
				edges[i].type = 'curve';
				edges[i].color = "#777";
			}
			s.refresh(); 
		});
	},

};

var gui = new dat.gui.GUI();

gui.add(obj, 'S').name("Left c.").min(1).step(1);
gui.add(obj, 'T').name("Right c.").min(1).step(1);
gui.add(obj, 'E').name("Edge c.").min(0);
gui.add(obj, 'generate');



