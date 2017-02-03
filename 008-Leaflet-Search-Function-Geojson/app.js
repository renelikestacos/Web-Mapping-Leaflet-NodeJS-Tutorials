// Initialize leaflet.js
var L = require('leaflet');

// Initialize search.js
var search = require('./search');

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


var items = [];
var at_states = L.geoJson(at, {
  onEachFeature: function(data, layer) {
    items.push(layer);
    layer.bindPopup('<b>' + data.properties.name + '</b><br>'
    	+ data.properties.cities + ' Cities'+'<br>'
    	+ data.properties.towns + ' Towns'+'<br>'
    	+ data.properties.density + ' People / km<sup>2</sup>');
  }
}).addTo(map);

var baseLayers = {
	"ESRI World Imagery": esri_WorldImagery,
	"ESRI World Terrain": esri_WorldTerrain
};

var overLayers = {
	"Austrian States": at_states
}

L.control.layers(baseLayers, overLayers).addTo(map);
L.control.search({data: items}).addTo(map);