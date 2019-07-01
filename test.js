// run a simple burnin script. this doesn't really do anything
// other than do a bad job of checking if calling isFullscreen
// a lot will cause a segfault at some point

let count = 25000;

console.log(`Running ${count} iterations!`)
for ( let i = 0; i < count; i++ ) {
  var fs = require('./index.js');
  fs.isFullscreen()
}
console.log("Done!");