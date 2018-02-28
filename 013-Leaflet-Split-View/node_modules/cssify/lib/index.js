'use strict'

var through = require('through2')
var processCss = require('./process-css')
var options = require('./options')

module.exports = function (fileName, opts) {
  opts = options.normalize(opts)

  if (options.skipIt(fileName, opts)) return through()

  var chunks = []

  return through(
    function (chunk, enc, next) {
      chunks.push(chunk)
      next()
    },
    function (done) {
      var buffer = Buffer.concat(chunks)
      var source = buffer.toString('utf-8')

      processCss(fileName, source, opts).then(function (moduleSource) {
        this.push(moduleSource)
        done()
      }.bind(this))
      .catch(done)
    }
  )
}
