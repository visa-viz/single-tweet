R""(
precision lowp float;
uniform sampler2D tex;
varying vec2 uv;
uniform vec4 color;

void main()
{
    gl_FragColor = color * texture2D(tex, uv);
}
)"";