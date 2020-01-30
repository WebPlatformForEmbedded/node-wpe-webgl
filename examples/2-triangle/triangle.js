var gles2 = require('../../gles2');

var WebGLUtils = require('../WebGLUtils.js');
var glMatrix = require('../gl-matrix.js');
var mat4 = glMatrix.mat4;

var options = {width: 1280, height: 720, title: "Single Triangle", fullscreen: false};
var gl = gles2.init(options);

function webGLStart() {
    initShaders()
    initBuffers();

    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.enable(gl.DEPTH_TEST);

    while(true) {
        drawScene();
        //animate();
        gles2.nextFrame();
    }
}


var fs = require('fs');
var shaders = {
    "shader-fs": fs.readFileSync(__dirname + "/frag.shader", {encoding: 'utf8'}),
    "shader-vs": fs.readFileSync(__dirname + "/vert.shader", {encoding: 'utf8'})
};

function getShader(gl, id) {
    var shaderScript = shaders[id];
    if (!shaderScript) {
        return null;
    }

    var shader;
    if (id == "shader-fs") {
        shader = gl.createShader(gl.FRAGMENT_SHADER);
    } else if (id == "shader-vs") {
        shader = gl.createShader(gl.VERTEX_SHADER);
    } else {
        return null;
    }

    gl.shaderSource(shader, shaderScript);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.error('shader error', id, gl.getShaderInfoLog(shader));
        return null;
    }

    return shader;
}


var shaderProgram;

function initShaders() {
    var fragmentShader = getShader(gl, "shader-fs");
    var vertexShader = getShader(gl, "shader-vs");

    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);

    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.error("Could not initialise shaders");
        console.error('gl.VALIDATE_STATUS', gl.getProgramParameter(shaderProgram, gl.VALIDATE_STATUS));
        console.error('gl.getError()', gl.getError());
        if (gl.getProgramInfoLog(shaderProgram) !== '') {
            console.warn('Warning: gl.getProgramInfoLog()', gl.getProgramInfoLog(shaderProgram));
        }
    }

    gl.useProgram(shaderProgram);

    shaderProgram.vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);

    shaderProgram.vertexColorAttribute = gl.getAttribLocation(shaderProgram, "aVertexColor");
    gl.enableVertexAttribArray(shaderProgram.vertexColorAttribute);

    shaderProgram.pMatrixUniform = gl.getUniformLocation(shaderProgram, "uPMatrix");
    shaderProgram.mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
}

var mvMatrix = mat4.create();
var mvMatrixStack = [];
var pMatrix = mat4.create();

function mvPushMatrix() {
    var copy = mat4.create();
    mat4.set(mvMatrix, copy);
    mvMatrixStack.push(copy);
}

function mvPopMatrix() {
    if (mvMatrixStack.length == 0) {
        throw "Invalid popMatrix!";
    }
    mvMatrix = mvMatrixStack.pop();
}


function setMatrixUniforms() {
    gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, pMatrix);
    console.log("setMatrixUniforms: error=" + gl.getError());
    gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
    console.log("setMatrixUniforms: error=" + gl.getError());
}


function degToRad(degrees) {
    return degrees * Math.PI / 180;
}

var pyramidVertexPositionBuffer;
var pyramidVertexColorBuffer;

function initBuffers() {
    pyramidVertexPositionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, pyramidVertexPositionBuffer);
    var vertices = [
        // Front face
        0.0,  1.0,  0.0,
        -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
    pyramidVertexPositionBuffer.itemSize = 3;
    pyramidVertexPositionBuffer.numItems = 3;

    pyramidVertexColorBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, pyramidVertexColorBuffer);
    var colors = [
        // Front face
        1.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 1.0
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    pyramidVertexColorBuffer.itemSize = 4;
    pyramidVertexColorBuffer.numItems = 3;
    console.log("initBuffers: error=" + gl.getError());
}


var rPyramid = 0;

function drawScene() {
    gl.viewport(0, 0, options.width, options.height);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    mat4.perspective(45, options.width / options.height, 0.1, 100.0, pMatrix);

    mat4.identity(mvMatrix);
    mat4.translate(mvMatrix, [0.0, 0.0, -8.0]);
    mat4.rotate(mvMatrix, degToRad(rPyramid), [0, 1, 0]);

    gl.bindBuffer(gl.ARRAY_BUFFER, pyramidVertexPositionBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, pyramidVertexPositionBuffer.itemSize, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, pyramidVertexColorBuffer);
    gl.vertexAttribPointer(shaderProgram.vertexColorAttribute, pyramidVertexColorBuffer.itemSize, gl.FLOAT, false, 0, 0);

    setMatrixUniforms();
    gl.drawArrays(gl.TRIANGLES, 0, pyramidVertexPositionBuffer.numItems);

    gl.bindBuffer(gl.ARRAY_BUFFER, null); // cleanup GL state
}


var lastTime = 0;

function animate() {
    var timeNow = new Date().getTime();
    if (lastTime != 0) {
        var elapsed = timeNow - lastTime;

        rPyramid += (90 * elapsed) / 1000.0;
    }
    lastTime = timeNow;
}


webGLStart();
