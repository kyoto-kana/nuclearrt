#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform sampler2D bkd;

uniform float lensBase;
uniform float lensCoeff;

uniform vec4 tintColor;

uniform float tintPow;
uniform float tintOrigPow;

void main()
{
    vec2 uv = TexCoord;
    vec4 lens = texture(uTexture, uv);

    float height = lensBase + lens.r * lensCoeff;

    uv += (height - 1.0) / 2.0;

    vec4 col = texture(bkd, uv / height);

    col.rgb = col.rgb * tintOrigPow + col.rgb * tintColor.rgb * tintPow;

    FragColor = vec4(col.rgb, 1.0);
}
