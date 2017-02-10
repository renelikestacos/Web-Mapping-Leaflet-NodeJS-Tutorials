# Leaflet-Nodejs-Tutorial

This is a series of tutorials how to use [Leaflet](http://leafletjs.com/) with [Nodejs](https://nodejs.org/en/). To install one of the tutorials just copy them into any folder and execute `npm start` within the folder in your console.
I am using Browserify and Beefy to start the tutorials on the localhost. If you do so as well, you will find it on <http://localhost:9966/> in your machine.

Note: Make sure you have already installed Nodejs and NPM. If you haven't, here is a [link](https://nodejs.org/en/download/) to download and install it.

#### 001-Leaflet-Base-Layers:
This first tutorial shows a few possible base layers you can use with Leaflet.

#### 002-Leaflet-Custom-Markers:
Shows how to integrate custom style markers. Markers are placed on the nine capitals of the Austrian states. By clicking on them, a popup appears and gives you information about the location.

#### 003-Leaflet-Interactive-Geojson:
This tutorial shows how to integrate an interactive Geojson layer. The example data is a geojson of the Austrian states. If you hover over a state, the name, population density (people/km2) appears in a small info control on the right bottom. Furthermore, the population density classified into groups including a legend panel.

Optional: In case you are familiar with MongoDB you can also find the Geojson in my MongoDB on mLAB. It can be find here `mongodb://guest:guest@ds159328.mlab.com:59328/mongo_project40`, the collection is called `austria`.

#### 004-Leaflet-WMS-GetFeatureInfo:
Demonstrates a custom-built function to retrieve information (attribute values) from a WMS layer for a pixel location on the map. OpenLayers has this function, Leaflet doesn't. For now it only works if one WMS layer is active.

#### 005-Leaflet-Custom-LayerGroup-Panel:
This tutorial shows how to include a customized layergroup panel to represent the displayed layers in a ordered way. I have included layers from different layer sources, such as a few base layers, WMS and Geojson. You can customize the styling of the layergroup panel like you want, you only need to change the `style.css` file.

#### 006-Leaflet-WFS-Request:
Shows the simple approach of making a WFS (Web Feature Service) Request on Leaflet. The result is a JSON structure of each object in the ESRI Shapefile which is stored in a Geoserver. From here, you could create a fancy looking table structure with JQuery or whatever and display them in a nice way.

#### 007-Leaflet-Custom-Opacity-Slider:
Simple example of an opacity slider for Leaflet.


#### 008-Leaflet-Search-Function-Geojson:
In this tutorial, you can find a search function example for Geojson data. The example data is a geojson of the Austrian states. If you type three or more letters into the search field, it will automatically give you state suggestions. By clicking on one of the suggestions, it will zoom into that state on the map.

#### 009-Leaflet-Custom-LayerGroup-Panel-2:
Like tutorial 005, this tutorial shows how to include a customized layergroup panel but in a more modern way. Layers from different data sources were included. You can customize the appearance, you only need to change the `layer_control.css` file. This custom styled layer control is based on the flawless work of Davi Custodio's [Leaflet.StyledLayerControl](https://github.com/davicustodio/Leaflet.StyledLayerControl).

#### 010-Leaflet-WMS-Common-Query-Language:
This tutorial shows how to use CQL (Common Query Language) on WMS layers. In the example, below the map it will be shown a screenshot of the attributes of the WMS layer. The example shows how to use CQL on all three attribute categories.
