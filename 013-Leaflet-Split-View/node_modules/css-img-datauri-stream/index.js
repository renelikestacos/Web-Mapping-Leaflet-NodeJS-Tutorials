/**
 * Transform image URL references into inline data URIs.
 *
 * Based largely on "Grunt Image Embed" by Eric Hynds.
 *   https://github.com/ehynds/grunt-image-embed
 */

var through = require("through");
var path = require("path");
var fs = require("fs");
var mime = require("mime");
var _ = require("underscore");

module.exports = function(filePath, opts) {
  "use strict";
  // Holds data written to the wrapper through stream.
  var data = "";

  // Cache regex's
  var rImages = /([\s\S]*?)(url\(([^)]+)\))(?![^;]*;\s*\/\*\s*ImageEmbed:skip\s*\*\/)|([\s\S]+)/img;
  var rExternal = /^(http|https|\/\/)/;
  var rData = /^data:/;
  var rQuotes = /['"]/g;
  var rParams = /([?#].*)$/g;
  var cache = {};

  var transformCss = function(cssFilePath, cssContents, options) {
    var result = "";

    // If no max image size is specified in the parameter opts object, default to the IE8 limit.
    options = _.extend({
      maxImageSize: 32768
    }, options);

    for(var group = rImages.exec(cssContents); group !== null; group = rImages.exec(cssContents)) {
      // if there is another url to be processed, then:
      //    group[1] will hold everything up to the url declaration
      //    group[2] will hold the complete url declaration (useful if no encoding will take place)
      //    group[3] will hold the contents of the url declaration
      //    group[4] will be undefined
      // if there is no other url to be processed, then group[1-3] will be undefined
      //    group[4] will hold the entire string
      if(group[4] == null) {
        result += group[1];
        result += generateImageString(cssFilePath, group[2], group[3], options);
      } else {
        result += group[4];
      }
    }
    return result;
  };

/**
 * [generateImageString description]
 * @param  {[type]} cssFilePath [description]
 * @param  {[type]} fullUrl     [description]
 * @param  {[type]} imageUrl    [description]
 * @return {[type]}             [description]
 */
  var generateImageString = function(cssFilePath, fullUrl, imageUrl, options) {
    imageUrl = imageUrl.trim()
      .replace(rQuotes, "")
      .replace(rParams, ""); // remove query string/hash parmams in the filename, like foo.png?bar or foo.png#bar

    // See if this image was already processed before...
    if(cache[imageUrl]) {
      // If we already processed this image, just pull it from the cache.
      return cache[imageUrl];
    } else {
      // If this is a novel image, encode it as a data URI string and then cache it.
      var loc = imageUrl;
      var isLocalFile = !rData.test(imageUrl) && !rExternal.test(imageUrl);

      if(!isLocalFile) {
        // Ignore non-local image references.
        return fullUrl;
      } else {
        // local file system.. fix up the path
        loc = imageUrl.charAt(0) === "/" ?
          loc :
          path.join(path.dirname(cssFilePath), imageUrl);

        // If that didn't work, try finding the image relative to
        // the current file instead.
        if(!fs.existsSync(loc)) {
          loc = path.resolve(__dirname + imageUrl);
        }
      }

      var encodedImage = getDataURI(loc);

      // If the encoded image meets the criteria for maximum image size, return its data URI.
      // Otherwise, just return the original image reference.
      if(encodedImage != null) {
        if(options.maxImageSize === undefined || encodedImage.length <= options.maxImageSize) {
          var url = 'url("' + encodedImage + '")';
          cache[imageUrl] = url;

          return url;
        } else {
          console.log("The image " + imageUrl + " exceeds the specified maximum data URI size of " + options.maxImageSize + ", so will not be converted.");
        }
      }

      // If the image exceeds the maximum data URI size (or otherwise failed to encode),
      // fall back on the original URL reference.
      return fullUrl;
    }
  };

  /**
   * Given the path for an image in the local file system, returns a data URI string
   * representing the data in that image file.
   */
  var getDataURI = function(localImagePath) {
        // Read the file in and convert it.
      var imageFile = fs.readFileSync(localImagePath);
      var mimeType = mime.lookup(localImagePath);
      var ret = "data:";
      ret += mimeType;
      ret += ";base64,";
      ret += imageFile.toString("base64");
      return ret;
  };

  /**
   * Module creation entry point. Aggregates all data from the incoming through stream
   * (which will represent the contents of a file) then processes the result to replace
   * local image references that meet maximum size requirements with data URIs.
   */
  if(filePath !== undefined && [ ".css", ".scss", ".less" ].indexOf( path.extname(filePath) ) === -1 )
    return through();
  else
    return through(write, end);

  /**
   * Implementation of through interface.
   */
  function write(buf) {
    data += buf;
  };

  /**
   * Implementation of through interface.
   */
  function end() {
    try {
      this.queue(transformCss(filePath, data, opts));
    } catch(err) {
      this.emit("error", new Error(err));
    }

    this.queue(null);
  }
};
