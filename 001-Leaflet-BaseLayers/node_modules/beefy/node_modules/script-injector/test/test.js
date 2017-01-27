var fs             = require('fs')
  , concat         = require('concat-stream')
  , scriptInjector = require('../index')
  , test           = require('tape')
  ;

var testRunner = function (filename, t) {
  var test1 = concat(function (data) {
    var reference = fs.readFileSync(__dirname + '/dat/' + filename + '-output.html');
    t.equal(data.toString(), reference.toString());
    t.end();
  });

  fs.createReadStream(__dirname + '/dat/' + filename + '.html')
    .pipe(scriptInjector(someCode))
    .pipe(test1);
}

test('inject code before the closing body tag', testRunner.bind(null, 'test1'));
test('inject code before the first script tag in the header', testRunner.bind(null, 'test2'));
test('inject code before the first script tag in the body', testRunner.bind(null, 'test3'));

function someCode () {
  console.log("I'm some code to be inserted inline");
}
