# cssify #

A simple Browserify v2 transform for adding required styles to the browser.

# Example

If you have a file `entry.js` that you want to require some css from `style.css`:

style.css:
``` css
body {
  background: pink;
}
```

entry.js:
``` js
var styleNode = require('./style.css');

console.log('The background is pink!')
```

Install cssify into your app:

```
$ npm install cssify
```

When you compile your app, just pass `-t cssify` to browserify:

```
$ browserify -t cssify entry.js > bundle.js
```


# Install

With [npm](https://npmjs.org):

```
npm install cssify
```

# Bonus

To add a stylesheet from a url:

``` js

var cssify = require('cssify')

cssify.byUrl('//netdna.bootstrapcdn.com/bootstrap/3.0.0/css/bootstrap.min.css')

// Bootstrap styles!

```

# License

BSD

# Misc

Thanks to substack's insert-css and domenic's simple-jadeify for helping me figure out how to actually test this thing.