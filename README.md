detect-fullscreen - Detect fullscreen windows in node.js
========================================================

This is some code that tries to detect if the user has a fullscreen window open. It should work pretty well in OSX. It works in Windows but needs some better support for computers with multiple screens. Unfortunately it won't work at all in Linux right now, although I would like to change that.

## How To Use It
```
var x = require('detect-fullscreen')
console.log(x.isFullscreen())
=> true/false
```

## How It Works

It works a little differently in OSX and Windows. You can see the code at `Fullscreen.css`

On OSX, the code iterates through all of the elements visible on the screen. One of these elements might be the Menubar which appears at the top of the screen. If that element doesn't exist, we assume that there's a fullscreen window on that display.

On Windows, the code checks the bounds of the active window. If they meet/exceed the resolution of the screen, we treat it as a fullscreen window.

## What Happens With Multiple Displays?

If a computer has multiple displays, and one of them has a fullscreen window, then `isFullscreen()` will return true. 

