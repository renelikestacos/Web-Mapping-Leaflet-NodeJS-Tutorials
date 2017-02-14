// Initialize leaflet.js
var L = require('leaflet');
var lT = require('./layerTree');

var layerControlOptions = {
	container_width 	: "300px",
	group_maxHeight     : "80px",
	container_maxHeight : "350px", 
	exclusive       	: false,
	collapsed : true, 
	position: 'topright'
};


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

var districts = L.tileLayer.wms("http://webportal.conabio.gob.mx:8085/geoserver/wms?",
	{layers:"leafletTutorial:at_districts", format:'image/png',transparent:true})//.addTo(map)

var at_states = L.geoJson(states).addTo(map)

var baseMaps = [
                { 
					groupName : "ESRI Base Layers",
					expanded : false,
					layers    : {
						"ESRI World Imagery" :  esri_WorldImagery,
						"ESRI World Terrain": esri_WorldTerrain,
						"ESRI National Geographic": esri_NatGeoWorldMap
					}
		        }							
];

var overlays = [
				 {
					groupName : "WMS Layers",
					expanded : false,
					layers    : { 
						"Austrian Districts": districts
					}	
                 },
                 {
                 	groupName: "GeoJSON Layers",
                 	expanded: false,
                 	layers:{
                 		"Austrian States": at_states
                 	}
                 }					 
];
			
var control = L.Control.styledLayerControl(baseMaps, overlays, layerControlOptions);
map.addControl(control);

map.on('overlayadd', onOverlayAdd);
function onOverlayAdd(e){
    console.log(e)
}

map.on('overlayremove', onOverlayRemove);
function onOverlayRemove(e){
	console.log(e)
}