'use strict'

var test = require('tape-catch')
var browserify = require('browserify')
var path = require('path')
var stream = require('stream')
var cssify = require('../../lib')

// writable stream that just ignores everything written to it
// similar to /dev/null in unix-like systems
var devNull = new stream.Writable({
  write: function (chunk, enc, next) {
    next()
  }
})

test('main module', function (t) {
  browserify(path.join(__dirname, '..', 'browser', 'index.js'))
    .transform(cssify)
    .bundle()
    .pipe(devNull)
    .on('finish', t.end)
    .on('error', t.fail)
})
