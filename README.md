# Leaflet-Tutorial

This is series of tutorials how to use [Leaflet](http://leafletjs.com/) with [Nodejs](https://nodejs.org/en/). To install one of the tutorials just copy them into any folder and execute `npm start` within the folder in your console.
I am using Browserify and Beefy to start the tutorials on the localhost. If you do so as well, you will find it on <http://localhost:9966/> in your machine.

Note: Make sure you have already installed Nodejs and NPM. If you haven't, here is a [link](https://nodejs.org/en/download/) to download and install it.

#### 001-Leaflet-Base-Layers:
This first tutorial shows a few possible base layers you can use with Leaflet.

#### 002-Leaflet-Custom-Markers
Shows how to integrate custom style markers. Markers are placed on the nine capitals of the Austrian states. By clicking on them, a popup appears and gives you information about the location.

#### 003-Leaflet-Interactive-Geojson:
This tutorial shows how to integrate an interactive Geojson layer. The example data is a geojson of the Austrian states. If you hover over a state, the name, population density (people/km2) appears in a small info control on the right bottom. Furthermore, the population density classified into groups including a legend panel.

#### 004-Leaflet-WMS-GetFeatureInfo
Demonstrates a custom-built function to retrieve information (attribute values) from a WMS layer for a pixel location on the map. OpenLayers has this function, Leaflet doesn't. For now it only works if one WMS layer is active.

#### 005-Leaflet-Custom-LayerGroup-Panel
This tutorial shows how to include a customized layergroup panel to represent the displayed layers in a ordered way. You can customize the styling like you want, you only need to change the `style.css` file.
