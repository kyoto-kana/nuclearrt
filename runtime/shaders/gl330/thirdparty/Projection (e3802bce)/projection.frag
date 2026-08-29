#version 330 core

#define PI   3.14159265359
#define PI_2 1.57079632679
#define PI2  6.28318530718

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D bkd;

uniform float fovVar;
uniform float latitudeVar;
uniform float longitudeVar;

float con1(float long_lat, float deg)
{
    return long_lat / deg;
}

float con2(float num)
{
    return (num / 2.0) - 0.5;
}

vec2 project(vec2 uv, vec2 m, vec2 fov)
{
    vec2 m2 = (m * 2.0 - 1.0) * vec2(PI, PI_2);
    vec2 cuv = (uv * 2.0 - 1.0) * fov * vec2(PI, PI_2);

    float x = cuv.x;
    float y = cuv.y;

    float rou = sqrt(x * x + y * y);
    float c = atan(rou);

    float sin_c = sin(c);
    float cos_c = cos(c);

    float lat = asin(
        cos_c * sin(m2.y) +
        (y * sin_c * cos(m2.y)) / rou
    );

    float lon = m2.x + atan(
        (x * sin_c) /
        (rou * cos(m2.y) * cos_c -
         y * sin(m2.y) * sin_c)
    );

    lat = (lat / PI_2 + 1.0) * 0.5;
    lon = (lon / PI + 1.0) * 0.5;

    return vec2(lon, lat) * vec2(PI2, PI);
}

void main()
{
    vec2 q = TexCoord;

    vec2 fov = vec2(fovVar);
    vec2 m = vec2(0.5, 0.5);

    vec2 dir = project(q, m, fov) / vec2(PI2, PI);

    vec2 ou;

    ou.x = con1(180.0, longitudeVar);
    ou.y = con1(90.0, latitudeVar);

    dir.x *= ou.x;
    dir.y *= ou.y;

    dir.x -= con2(ou.x);
    dir.y -= con2(ou.y);

    FragColor = texture(bkd, dir);
}