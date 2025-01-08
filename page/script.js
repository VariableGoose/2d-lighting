const canvas = document.querySelector('canvas')
function resize() {
    canvas.height = window.innerHeight;
    canvas.width = window.innerWidth;
    // This notifies emscripten about the resize.
    canvas.dispatchEvent(new Event("resize"));
}

window.onload = resize;
window.onresize = resize;
