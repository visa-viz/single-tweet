R""(
precision lowp float;
varying vec2 uv;
uniform vec4 color;
uniform float w;
uniform float h;
uniform float radius;

float aa(float x, float y, float x0, float y0, float radius)
{
    int total_in = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            total_in += int(pow(x + 0.125 + float(i) * 0.25 - x0, 2.0) + pow(y + 0.125 + float(j) * 0.25 - y0, 2.0) <= pow(radius, 2.0));
        }
    }
    return float(total_in) / 16.0;
}

void main()
{
    float x = uv.x * w;
    float y = uv.y * h;
    if (y <= radius)
    {
        if (x <= radius)
        {
            if (pow(radius - (x + 1.0), 2.0) + pow(radius - (y + 1.0), 2.0) > radius * radius)
            {
            	gl_FragColor = vec4(color.rgb, 0.0);
            }
            else if (pow(radius - x, 2.0) + pow(radius - y, 2.0) <= radius * radius)
            {
                gl_FragColor = vec4(color.rgb, 1.0);
            }
            else
            {
                gl_FragColor = vec4(color.rgb, aa(x, y, radius, radius, radius));
            }
        }
        else if (x < w - radius)
        {
            gl_FragColor = vec4(color.rgb, 1.0);
        }
        else
        {
            if (pow(w - radius + 1.0 - x, 2.0) + pow(radius - (y + 1.0), 2.0) > radius * radius)
            {
            	gl_FragColor = vec4(color.rgb, 0.0);
            }
            else if (pow(w - radius + 1.0 - (x + 1.0), 2.0) + pow(radius - y, 2.0) <= radius * radius)
            {
                gl_FragColor = vec4(color.rgb, 1.0);
            }
            else
            {
                gl_FragColor = vec4(color.rgb, aa(x, y, w - radius + 1.0, radius, radius));
            }
        }
    }
    else if (y < h - radius)
    {
        gl_FragColor = vec4(color.rgb, 1.0);
    }
    else
    {
        if (x <= radius)
        {
            if (pow(radius - (x + 1.0), 2.0) + pow(h - radius + 1.0 - y, 2.0) > radius * radius)
            {
            	gl_FragColor = vec4(color.rgb, 0.0);
            }
            else if (pow(radius - x, 2.0) + pow(h - radius + 1.0 - (y + 1.0), 2.0) <= radius * radius)
            {
                gl_FragColor = vec4(color.rgb, 1.0);
            }
            else
            {
                gl_FragColor = vec4(color.rgb, aa(x, y, radius, h - radius + 1.0, radius));
            }
        }
        else if (x < w - radius)
        {
            gl_FragColor = vec4(color.rgb, 1.0);
        }
        else
        {
            if (pow(w - radius + 1.0 - x, 2.0) + pow(h - radius + 1.0 - y, 2.0) > radius * radius)
            {
            	gl_FragColor = vec4(color.rgb, 0.0);
            }
            else if (pow(w - radius + 1.0 - (x + 1.0), 2.0) + pow(h - radius + 1.0 - (y + 1.0), 2.0) <= radius * radius)
            {
                gl_FragColor = vec4(color.rgb, 1.0);
            }
            else
            {
                gl_FragColor = vec4(color.rgb, aa(x, y, w - radius + 1.0, h - radius + 1.0, radius));
            }
        }
    }
}
)"";