var trumpet = require('trumpet')
  , duplexer = require('duplexer2')
  , through = require('through2');

function scriptInjector (script) {
  var tr1 = trumpet()
    , tr2 = trumpet()
    , needToAddScript = true;

  script = script ? '<script type=\"text/javascript\">\n;(' + script + ')()\n<\/script>\n'
                  : ';(' + "function () { console.log('You didn\'t provide a script to inject') }" + ')()';

  var firstScriptTag = tr1.createStream('script', { outer: true });
  var bodyTag = tr2.createStream('body');

  firstScriptTag // Inject the new script before the first existing <script>
    .pipe(through(
      function (data, enc, cb) {
        if (needToAddScript) {
          this.push(script);
          needToAddScript = false;
        }
        this.push(data);
        cb();
      }))
    .pipe(firstScriptTag);

  bodyTag // If there were no <script>'s, insert the script right before </body>
    .pipe(through(
      function (data, enc, cb) {
        this.push(data);
        cb();
      },
      function () {
        if (needToAddScript) {
          this.push(script);
        }
        this.push(null);
      }))
    .pipe(bodyTag);

  tr1.pipe(tr2);

  return duplexer(tr1, tr2);
}

module.exports = scriptInjector;
