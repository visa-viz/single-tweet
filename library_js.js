mergeInto(LibraryManager.library, {
  uploadFlipped: function(img) {
    GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, true);
    GLctx.texImage2D(0xDE1/*GLctx.TEXTURE_2D*/, 0, 0x1908/*GLctx.RGBA*/, 0x1908/*GLctx.RGBA*/, 0x1401/*GLctx.UNSIGNED_BYTE*/, img);
    GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, false);
  },
  upload_unicode_char_to_texture__deps: ['uploadFlipped'],
  upload_unicode_char_to_texture: function(unicodeChar, charSize, bold, r, g, b, outCharWidth, outCharBaseline, outTextureWidth, outTextureHeight) {
    var canvas = document.createElement('canvas');
    canvas.width = charSize;
    canvas.height = charSize * 2;
    // document.body.appendChild(canvas); // Enable for debugging
    var ctx = canvas.getContext('2d');
    ctx.fillStyle = 'black';
    ctx['globalCompositeOperator'] = 'copy';
    ctx.globalAlpha = 0;
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.globalAlpha = 1;
    ctx.fillStyle = `rgb(${r * 255} ${g * 255} ${b * 255})`;
    ctx.font = (bold ? 'bold ' : '') + charSize + 'px Segoe UI';
    ctx.textBaseline = 'bottom';
    var text = String.fromCharCode(unicodeChar);
    ctx.fillText(text, 0, charSize);
    _uploadFlipped(canvas);
    textMetrics = ctx.measureText(text);
    HEAPF32[outCharWidth >> 2] = textMetrics.width;
    HEAPF32[outCharBaseline >> 2] = charSize;
    HEAPF32[outTextureWidth >> 2] = canvas.width;
    HEAPF32[outTextureHeight >> 2] = canvas.height;
  },
  load_texture_from_url__deps: ['uploadFlipped'],
  load_texture_from_url: function(glTexture, url, outW, outH) {
    var img = new Image();
    img.onload = function() {
      HEAPU32[outW>>2] = img.width;
      HEAPU32[outH>>2] = img.height;
      GLctx.bindTexture(0xDE1/*GLctx.TEXTURE_2D*/, GL.textures[glTexture]);
      _uploadFlipped(img);
    };
    img.onerror = function(event, source, lineno, colno, error) {
      debugger;
    }
    img.src = UTF8ToString(url);
  }
});
