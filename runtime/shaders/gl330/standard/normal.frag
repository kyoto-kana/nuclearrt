#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform int uInkEffect;

void main() {
    vec4 texColor = texture(uTexture, TexCoord);
    if (texColor.a < 0.01) discard;

    if (uInkEffect == 2) // Inverted
    {
        texColor.rgb = 1.0 - texColor.rgb;
    }

    texColor *= uColor;

    if (uInkEffect == 10) // Monochrome
    {
        float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
        texColor.rgb = vec3(gray);
    }

    FragColor = texColor;
}
