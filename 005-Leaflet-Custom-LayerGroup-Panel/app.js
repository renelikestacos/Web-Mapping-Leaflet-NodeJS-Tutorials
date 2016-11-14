// Initialize leaflet.js
var L = require('leaflet');
var GFI = require('./groupLayerPanel');

// Initialize the map
var map = L.map('map', {
  scrollWheelZoom: false
});

// Set the position and zoom level of the map
map.setView([47.70, 13.35], 7);

/* Base Layers */
var osm_bw_mapnik = L.tileLayer('http://{s}.tiles.wmflabs.org/bw-mapnik/{z}/{x}/{y}.png', {
	maxZoom: 18,
	attribution: '&copy; OSM Black and White Mapnik<a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
})

var esri_WorldTerrain = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Terrain_Base/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: USGS, Esri, TANA, DeLorme, and NPS',
	maxZoom: 13
});

var esri_NatGeoWorldMap = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/NatGeo_World_Map/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; National Geographic, Esri, DeLorme, NAVTEQ, UNEP-WCMC, USGS, NASA, ESA, METI, NRCAN, GEBCO, NOAA, iPC',
	maxZoom: 16
});

/* GeoJSON Layer Style */
function style(feature) {
	return {
		weight: 2,
		opacity: 0.7,
		color: 'green',
		dashArray: '5',
		fillOpacity: 0.7
	};
};

/* GeoJSON Layer */
states = L.geoJson(states, {
	style: style
}).addTo(map);

/* WMS Layer */
var districts = L.tileLayer.wms("http://webportal.conabio.gob.mx:8085/geoserver/wms?",
	{layers:"leafletTutorial:at_districts", format:'image/png',transparent:true})

/* Markers */
var klagenfurt = L.marker([46.623997, 14.307812]).bindPopup('<b>Klagenfurt, Kärnten</b>'),
	graz = L.marker([47.070762, 15.438698]).bindPopup('<b>Graz, Steiermark</b>'),
	salzburg = L.marker([47.805109, 13.041151]).bindPopup('<b>Salzburg, Salzburg</b>'),
	eisenstadt = L.marker([47.845993, 16.527337]).bindPopup('<b>Eisenstadt, Burgenland</b>'),
	wien = L.marker([48.208539, 16.372505]).bindPopup('<b>Wien, Wien</b>'),
	stpoelten = L.marker([48.203828, 15.630877]).bindPopup('<b>St.Pölten, Niederösterreich</b>'),
	linz = L.marker([48.307025, 14.284829]).bindPopup('<b>Linz, Oberösterreich</b>'),
	innsbruck = L.marker([47.268896, 11.401791]).bindPopup('<b>Innsbruck, Tirol</b>'),
	bregenz = L.marker([47.500929, 9.740660]).bindPopup('<b>Bregenz, Vorarlberg</b>');

var capitals = L.layerGroup([klagenfurt, 
	graz, eisenstadt, salzburg, wien, 
	stpoelten, linz, innsbruck, bregenz]);


var baselayers = {
	"OpenStreetMap Mapnik":osm_bw_mapnik
};
var layers = {
	"ESRI Maps":{
		"ESRI World Imagery": esri_WorldImagery,
		"ESRI World Terrain": esri_WorldTerrain,
		"ESRI National Geographic": esri_NatGeoWorldMap
	},
	"Borders":{
		"Austrian States": states,
		"Austrian Districts":districts
	},
	'Markers': {
		"Austrian Capitals":capitals
	}
};

L.control.groupedLayers(baselayers, layers).addTo(map);