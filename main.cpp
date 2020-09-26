#include <emscripten.h>
#include <emscripten/html5.h>
#include <webgl/webgl2.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

// JS bindings
extern "C"
{
    void upload_unicode_char_to_texture(int unicodeChar, int charSize, int bold, float r, float g, float b, float *outCharWidth, float *outCharBaseline, float *outTextureWidth, float *outTextureHeight);
    void load_texture_from_url(GLuint texture, const char *url, int *outW, int *outH);
}

static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE glContext;
static GLuint quad, solidColor;
static GLuint texturedRectColorPos, texturedRectMatPos;
static GLuint roundedRectColorPos, roundedRectMatPos, roundedRectWidthPos, roundedRectHeightPos, roundedRectRadiusPos;
static GLuint textured_rect_program, rounded_rect_program;
static float pixelWidth, pixelHeight;

static GLuint compile_shader(GLenum shaderType, const char *src)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        printf("Shader source: %s\n", src);
        printf("Compile status: %i", compileStatus);
        GLchar infoLog[10000];
        GLsizei infoLogLength;
        glGetShaderInfoLog(shader, 10000, &infoLogLength, infoLog);
        printf("Shader compile log: %s\n", infoLog);
    }
    return shader;
}

static GLuint create_program(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glBindAttribLocation(program, 0, "pos");
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("Program link status: %i\n", status);
        GLchar infoLog[10000];
        GLsizei infoLogLength;
        glGetProgramInfoLog(program, 10000, &infoLogLength, infoLog);
        printf("Program info log: %s\n", infoLog);
    }
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("Program validate status: %i\n", status);
        GLchar infoLog[10000];
        GLsizei infoLogLength;
        glGetProgramInfoLog(program, 10000, &infoLogLength, infoLog);
        printf("Program info log: %s\n", infoLog);
    }
    return program;
}

static GLuint create_texture()
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return texture;
}

void init_webgl(int width, int height)
{
    double dpr = emscripten_get_device_pixel_ratio();
    emscripten_set_element_css_size("canvas", width / dpr, height / dpr);
    emscripten_set_canvas_element_size("canvas", width, height);

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = 0;
    attrs.majorVersion = 2;
    glContext = emscripten_webgl_create_context("canvas", &attrs);
    assert(glContext);
    emscripten_webgl_make_context_current(glContext);

    pixelWidth = 2.f / width;
    pixelHeight = 2.f / height;

    static const char vertex_shader[] =
        #include "vertex_shader.glsl"
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);

    static const char textured_rect_fragment_shader[] =
        #include "texture_fragment_shader.glsl"
    GLuint trfs = compile_shader(GL_FRAGMENT_SHADER, textured_rect_fragment_shader);
    textured_rect_program = create_program(vs, trfs);

    static const char rounded_rect_fragment_shader[] = 
        #include "rounded_rect_fragment_shader.glsl"
    GLuint rrfs = compile_shader(GL_FRAGMENT_SHADER, rounded_rect_fragment_shader);
    rounded_rect_program = create_program(vs, rrfs);

    glUseProgram(textured_rect_program);

    texturedRectColorPos = glGetUniformLocation(textured_rect_program, "color");
    texturedRectMatPos = glGetUniformLocation(textured_rect_program, "mat");
    roundedRectColorPos = glGetUniformLocation(rounded_rect_program, "color");
    roundedRectMatPos = glGetUniformLocation(rounded_rect_program, "mat");
    roundedRectWidthPos = glGetUniformLocation(rounded_rect_program, "w");
    roundedRectHeightPos = glGetUniformLocation(rounded_rect_program, "h");
    roundedRectRadiusPos = glGetUniformLocation(rounded_rect_program, "radius");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenBuffers(1, &quad);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    const float pos[] = {0, 0, 1, 0, 0, 1, 1, 1};
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    solidColor = create_texture();
    unsigned int whitePixel = 0xFFFFFFFFu;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel);
}

struct Color
{
    float r;
    float g;
    float b;
    float a;
};

void clear_screen(Color color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void fill_rounded_rectangle(float x0, float y0, float x1, float y1, float radius, float scaleFactor, Color color)
{
    glUseProgram(rounded_rect_program);
    float mat[16] = {
        (x1 - x0) * scaleFactor * pixelWidth, 0, 0, 0,
        0, (y1 - y0) * scaleFactor * pixelHeight, 0, 0,
        0, 0, 1, 0,
        x0 * scaleFactor * pixelWidth - 1.f, y0 * scaleFactor * pixelHeight - 1.f, 0, 1};
    glUniformMatrix4fv(roundedRectMatPos, 1, 0, mat);
    glUniform4f(roundedRectColorPos, color.r, color.g, color.b, color.a);
    glUniform1f(roundedRectWidthPos, (x1 - x0) * scaleFactor);
    glUniform1f(roundedRectHeightPos, (y1 - y0) * scaleFactor);
    glUniform1f(roundedRectRadiusPos, radius);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void fill_textured_rectangle_raw(float x0, float y0, float x1, float y1, Color color, GLuint texture)
{
    glUseProgram(textured_rect_program);
    float mat[16] = {
        (x1 - x0) * pixelWidth, 0, 0, 0,
        0, (y1 - y0) * pixelHeight, 0, 0,
        0, 0, 1, 0,
        x0 * pixelWidth - 1.f, y0 * pixelHeight - 1.f, 0, 1};
    glUniformMatrix4fv(texturedRectMatPos, 1, 0, mat);
    glUniform4f(texturedRectColorPos, color.r, color.g, color.b, color.a);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void fill_textured_rectangle(float x0, float y0, float x1, float y1, float scaleFactor, Color color, GLuint texture)
{
    fill_textured_rectangle_raw(x0 * scaleFactor, y0 * scaleFactor, x1 * scaleFactor, y1 * scaleFactor, color, texture);
}

typedef struct Texture
{
    // Image
    char *url;
    int w, h;

    GLuint texture;
} Texture;

#define MAX_TEXTURES 256
static Texture textures[MAX_TEXTURES] = {};

static Texture *find_or_cache_url(const char *url)
{
    for (int i = 0; i < MAX_TEXTURES; ++i) // Naive O(n) lookup for tiny code size
        if (!strcmp(textures[i].url, url))
            return textures + i;
        else if (!textures[i].url)
        {
            textures[i].url = strdup(url);
            textures[i].texture = create_texture();
            load_texture_from_url(textures[i].texture, url, &textures[i].w, &textures[i].h);
            return textures + i;
        }
    return 0; // fail
}

void fill_image(float x0, float y0, float scale, float scaleFactor, Color color, const char *url)
{
    Texture *t = find_or_cache_url(url);
    float x1 = x0 + t->w * scale;
    float y1 = y0 + t->h * scale;
    fill_textured_rectangle(x0, y0, x1, y1, scaleFactor, color, t->texture);
}

typedef struct Glyph
{
    // Font
    unsigned int ch;
    int charSize;
    int bold;
    float r;
    float g;
    float b;
    float charWidth;
    float charBaseline;
    float textureWidth;
    float textureHeight;

    GLuint texture;
} Glyph;

#define MAX_GLYPHS 256
static Glyph glyphs[MAX_GLYPHS] = {};
static Glyph *find_or_cache_character(unsigned int ch, int charSize, int bold, float r, float g, float b)
{
    for (int i = 0; i < MAX_TEXTURES; ++i) // Naive O(n) lookup for tiny code size
        if (glyphs[i].ch == ch && glyphs[i].charSize == charSize && glyphs[i].bold == bold && glyphs[i].r == r && glyphs[i].g == g && glyphs[i].b == b)
            return glyphs + i;
        else if (!glyphs[i].ch)
        {
            glyphs[i].ch = ch;
            glyphs[i].charSize = charSize;
            glyphs[i].texture = create_texture();
            float charW, textureW, textureH;
            glyphs[i].bold = bold;
            glyphs[i].r = r;
            glyphs[i].g = g;
            glyphs[i].b = b;
            upload_unicode_char_to_texture(ch, charSize, bold, r, g, b, &glyphs[i].charWidth, &glyphs[i].charBaseline, &glyphs[i].textureWidth, &glyphs[i].textureHeight);

            return glyphs + i;
        }
    return 0; // fail
}

float fill_char_raw(float x0, float y0, Color color, unsigned int ch, int charSize, int bold)
{
    Glyph *glyph = find_or_cache_character(ch, charSize, bold, color.r, color.g, color.b);
    Color textureColor = { 1, 1, 1, color.a };
    // Glyph measurements are multiplied by scale factor but the coordinates are not
    fill_textured_rectangle_raw(x0, y0 - glyph->charBaseline, x0 + glyph->textureWidth, y0 + glyph->textureHeight - glyph->charBaseline, textureColor, glyph->texture);
    return glyph->charWidth;
}

void fill_char(float x0, float y0, float scaleFactor, Color color, unsigned int ch, int charSize, int bold)
{
    fill_char_raw(x0 * scaleFactor, y0 * scaleFactor, color, ch, charSize * scaleFactor, bold);
}

void fill_text(float x0, float y0, float scaleFactor, Color color, const char *str, int charSize, int bold)
{
    float scaledX0 = x0 * scaleFactor;
    while (*str)
    {
        float width = fill_char_raw(scaledX0, y0 * scaleFactor, color, *str++, charSize * scaleFactor, bold);
        scaledX0 += width;
    }
}

#define WIDTH 1024
#define HEIGHT 768
#define SCALE_FACTOR 1.75
#define SCALED_HEIGHT (HEIGHT / SCALE_FACTOR)

const char *line1 = "Request for Netflix show: ethnic aunties travelling and cooking";
const char *line2 = "together. An Indian aunty goes to Mexico and meets the local aunties";
const char *line3 = "there. And compare cooking recipes, and complain about their";
const char *line4 = "children. Repeat with aunties worldwide";
const char *lines[] = {line1, line2, line3, line4};

// Per-frame animation tick.
EM_BOOL draw_frame(double t, void *)
{
    float lineHeightFactor = 1.3125;
    float mainFontSize = 15;
    float counterFontSize = 13;

    Color white = { 1.0, 1.0, 1.0, 1.0 };
    Color outlineColor = { 204.0 / 255, 214.0 / 255, 221.0 / 255, 1.0 };
    Color textColor = { 20.0 / 255, 23.0 / 255, 26.0 / 255, 1.0 };
    Color textAuxColor = { 101.0 / 255, 119.0 / 255, 134.0 / 255, 1.0 };

    float avatarTextureSize = 205;
    float avatarDisplaySize = 49;
    float counterTextureSize = 96;
    float counterDisplaySize = 18.75;

    clear_screen(white);

    // Outline
    fill_rounded_rectangle(15, SCALED_HEIGHT - 164, 565, SCALED_HEIGHT - 15, 15, SCALE_FACTOR, outlineColor);
    fill_rounded_rectangle(16, SCALED_HEIGHT - 163, 564, SCALED_HEIGHT - 16, 14, SCALE_FACTOR, white);

    // Avatar
    fill_image(30, SCALED_HEIGHT - (25 + 49), avatarDisplaySize / avatarTextureSize, SCALE_FACTOR, white, "avatar.png");

    // Header
    fill_text(89, SCALED_HEIGHT - (25 + mainFontSize * lineHeightFactor), SCALE_FACTOR, textColor, "visa is cleaning out his notes", mainFontSize, 1);
    fill_text(295, SCALED_HEIGHT - (25 + mainFontSize * lineHeightFactor), SCALE_FACTOR, textAuxColor, "@visakanv", mainFontSize, 0);
    fill_char(370, SCALED_HEIGHT - (25 + mainFontSize * lineHeightFactor), SCALE_FACTOR, textAuxColor, 0xB7, mainFontSize, 0);
    fill_text(379, SCALED_HEIGHT - (25 + mainFontSize * lineHeightFactor), SCALE_FACTOR, textAuxColor, "Jun 12, 2019", mainFontSize, 0);

    // Body
    for (int i = 0; i < 4; i++)
    {
        fill_text(89, SCALED_HEIGHT - (47 + (1 + i) * mainFontSize * lineHeightFactor), SCALE_FACTOR, textColor, lines[i], mainFontSize, 0);
    }

    // Counters
    fill_image(89, SCALED_HEIGHT - (135 + counterDisplaySize), counterDisplaySize / counterTextureSize, SCALE_FACTOR, white, "reply.png");
    fill_text(118, SCALED_HEIGHT - (136 + counterFontSize * lineHeightFactor), SCALE_FACTOR, textAuxColor, "121", counterFontSize, 0);
    fill_image(220, SCALED_HEIGHT - (135 + counterDisplaySize), counterDisplaySize / counterTextureSize, SCALE_FACTOR, white, "retweet.png");
    fill_text(249, SCALED_HEIGHT - (136 + counterFontSize * lineHeightFactor), SCALE_FACTOR, textAuxColor, "3.6K", counterFontSize, 0);
    fill_image(354, SCALED_HEIGHT - (135 + counterDisplaySize), counterDisplaySize / counterTextureSize, SCALE_FACTOR, white, "fav.png");
    fill_text(383, SCALED_HEIGHT - (136 + counterFontSize * lineHeightFactor), SCALE_FACTOR, textAuxColor, "13.4K", counterFontSize, 0);

    return EM_TRUE;
}

int main()
{
    init_webgl(WIDTH, HEIGHT);
    emscripten_request_animation_frame_loop(&draw_frame, 0);
}
