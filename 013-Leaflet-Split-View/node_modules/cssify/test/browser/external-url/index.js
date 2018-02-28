'use strict'

var test = require('tape-catch')
var cssify = require('../../..')

var externalUrl =
  'https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css'
var link = cssify.byUrl(externalUrl)

test('injecting external url', function (t) {
  t.equal(link.parentElement, document.head, 'link is inserted')
  t.equal(link.rel, 'stylesheet', 'rel is set')
  t.equal(link.href, externalUrl, 'href is set')
  t.end()
})
