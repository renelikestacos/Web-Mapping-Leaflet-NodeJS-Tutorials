CSS Local Image Reference -> Data URI Transform
======================

Accepts a file argument and an (optional) opts argument. For a CSS file passed as input, returns a [through stream](https://github.com/dominictarr/through) with references to local images replaced by inline [data URIs](http://css-tricks.com/data-uris/). All other files return a vanilla through stream.

The opts argument object may contain the following:
* `maxImageSize` : size (in bytes) beyond which local images references will not be converted to data URIs.

Can be used as a [parcelify](https://github.com/rotundasoftware/parcelify) transform.

Draws heavily from [Grunt Image Embed](https://github.com/ehynds/grunt-image-embed/blob/master/README.md) by [Eric Hynds](https://github.com/ehynds).

#Usage
### css-img-datauri-stream(file [, opts])
* `file` - the CSS file to transform
* `opts` - optional object whose 'maxImageSize' member specifies a byte size beyond which local image references will not be converted to data URIs.
