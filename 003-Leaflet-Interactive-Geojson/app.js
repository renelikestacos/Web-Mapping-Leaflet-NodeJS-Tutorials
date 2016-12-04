// Initialize leaflet.js
var L = require('leaflet');

// Initialize the map
var map = L.map('map', {
  scrollWheelZoom: false
});

// Set the position and zoom level of the map
map.setView([47.70, 13.35], 7);

/* Base Layers */
var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
}).addTo(map);

var esri_WorldTerrain = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Terrain_Base/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: USGS, Esri, TANA, DeLorme, and NPS',
	maxZoom: 13
});

var esri_NatGeoWorldMap = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/NatGeo_World_Map/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; National Geographic, Esri, DeLorme, NAVTEQ, UNEP-WCMC, USGS, NASA, ESA, METI, NRCAN, GEBCO, NOAA, iPC',
	maxZoom: 16
});

// Create base layers group object
var baseLayers = {
	"ESRI World Imagery": esri_WorldImagery,
	"ESRI World Terrain": esri_WorldTerrain,
	"ESRI National Geographic": esri_NatGeoWorldMap
};

// Add baseLayers to the map
geojson = L.geoJson(states, {
	style: style,
	onEachFeature: onEachFeature
}).addTo(map);

var overLayers = {
	"Austrian States":geojson
}

L.control.layers(baseLayers, overLayers).addTo(map);

// Create control that shows information on hover
var info = L.control({position:'topright'});

info.onAdd = function (map) {
	this._div = L.DomUtil.create('div', 'info');
	this.update();
	return this._div;
};

info.update = function (props) {
		this._div.innerHTML = '<p><b>Population Density</b></p>' +  (props ?
			'<b>' + props.name + '</b><br />' + props.density + ' people / km<sup>2</sup>'
			: 'Hover over a state');
};
info.addTo(map);

function getColor(d) {
	return d > 1000 ? '#0868ac' :
			d > 130  ? '#2f8ec0' :
			d > 100  ? '#55b0c8' :
			d > 80   ? '#7bccc4' :
			d > 70   ? '#a5dcbe' :
			d > 50   ? '#ccebca' :
						'#ccebca';
}

/* Set of function for the hover over the geojson layer */
function style(feature) {
	return {
		weight: 2,
		opacity: 0.7,
		color: 'white',
		dashArray: '2',
		fillOpacity: 0.7,
		fillColor: getColor(feature.properties.density)

	};
}

function highlightFeature(e) {
	var layer = e.target;

	layer.setStyle({
		weight: 5,
		color: '#277FCA',
		dashArray: '',
		fillOpacity: 0.7
	});

	if (!L.Browser.ie && !L.Browser.opera && !L.Browser.edge) {
		layer.bringToFront();
	}

	info.update(layer.feature.properties);
}

var geojson;

function resetHighlight(e) {
	geojson.resetStyle(e.target);
	info.update();
}

function zoomToFeature(e) {
	map.fitBounds(e.target.getBounds());
}

function onEachFeature(feature, layer) {
	layer.on({
		mouseover: highlightFeature,
		mouseout: resetHighlight,
		click: zoomToFeature
	});
}


var legend = L.control({position: 'bottomright'});

legend.onAdd = function (map) {

	var div = L.DomUtil.create('div', 'info legend'),
		grades = [1, 70, 80, 100, 130, 1000],
		labels = [],
		from, to;

	for (var i = 0; i < grades.length; i++) {
		from = grades[i];
		to = grades[i + 1];

		labels.push(
			'<i style="background:' + getColor(from + 1) + '"></i> ' +
			from + (to ? '&ndash;' + to : '+'));
	}

	div.innerHTML = labels.join('<br>');
	return div;
};

legend.addTo(map);
