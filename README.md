# Title

## Building
### Web
CMake and Emscipten needs to be installed.
```bash
emcmake cmake -B build
cmake --build build
```

## Using web builds in websites
When integrating the project into a website you need to do a bit of setup.
A single canvas element with the tag 'canvas' is required. A piece of javascript
is also needed to give the canvas to the compiled program.

This code is within the `body` of the HTML file.
```html
<canvas id="canvas"></canvas>
<script type="text/javascript">
    var Module = {
        canvas: () => { return document.getElementById('canvas'); }(),
    };
</script>
<script src="your-program.js" type="text/javasript"></script>
```

It might be a good idea to add an event listener to the canvas to be notified
when the WebGL context is lost.
```html
<canvas id="canvas"></canvas>
<script type="text/javascript">
    var Module = {
        canvas: () => {
            let canvas = document.getElementById('canvas');

            // As a default initial behavior, pop up an alert when webgl context is lost. To make your
            // application robust, you may want to override this behavior before shipping!
            // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
            canvas.addEventListener("webglcontextlost", (e) => { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

            return canvas;
        }(),
    };
</script>
<script src="your-program.js" type="text/javasript"></script>
```
