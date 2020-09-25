R""(
attribute vec4 pos;
varying vec2 uv;
uniform mat4 mat;

void main()
{
    uv = pos.xy;
    gl_Position = mat*pos;
}
)"";