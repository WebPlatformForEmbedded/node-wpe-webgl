var gles2 = require('./build/Release/gles2');

var init = function(options) {
    options = options || {};

    var width = (typeof options.width == "number" ? options.width : 1280);
    var height = (typeof options.height == "number" ? options.height : 720);
    var fullscreen = !!options.fullscreen;
    var title = options.title || "";

    gles2.init(width, height, fullscreen, title);

    return require('./lib/webgl').instance;
};

var blit = function() {
    gles2.blit();
};

module.exports = {
    init: init,
    blit: blit
};


