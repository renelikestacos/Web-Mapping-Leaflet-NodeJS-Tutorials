'use strict'

// FIXME: probably not good to use "private" interface
require('tape-catch').getHarness()._results.once('done', function () {
  console.log('# coverage:', JSON.stringify(global.__coverage__))
  window.close()
})

require('./basic')
require('./external-url')
