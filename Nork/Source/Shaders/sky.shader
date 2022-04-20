#type vertex
#version 330 core

layout(location = 0) in vec3 vPos;
uniform mat4 VP;

out vec3 worldPos2;

void main(void)
{
    worldPos2 = vPos;
    gl_Position = VP * vec4(vPos, 1.0f);
    gl_Position = gl_Position.xyww;
}

#type fragment
#version 330 core

uniform vec3 camPos;
uniform vec3 lightPos = vec3(0.58f, 0.36f, -0.26f);

in vec3 worldPos2;


uniform float sunSize = 5.0f;
uniform float sunRad = 0.058f;
uniform float atmThick = 0.05f;
uniform float density = 20.33f;
uniform float commonArea = 0.2f;
uniform vec3 _color = vec3(2, 1.8f, 1.25f);
uniform vec3 _colorSky = vec3(1, 1.36f, 2.0f);
uniform float atmosphereH = 0.377f;
uniform vec3 atmFilter = vec3(8.0f, 0.767f, -1.0f);
uniform float atmOffs = 0.00f;
uniform float skyMin = -0.284f;
uniform float skyMax = 0.417f;
uniform float poww = 1.619;
uniform float correct = 1.0f;

float calcRedness(float lightH)
{
    float height = abs(lightH - atmOffs);
    if (height < atmThick)
        return 1.0f;
    height -= atmThick;
    height = min(height, atmosphereH);
    float redness = (atmosphereH - height) * (1 / atmosphereH);
    if (redness < 1.0f)
        redness = pow(redness, poww);
    return redness;
}

void main ()
{
    vec3 worldPos = normalize(worldPos2);
    vec3 lightPos = normalize(lightPos);
    float lightH = lightPos.y;
    float redness = calcRedness(lightH);
    float rednessSky = calcRedness(worldPos.y) * redness;
    float skyBrightness = (clamp(lightH, skyMin, skyMax) + -skyMin) * (1 / (skyMax - skyMin));

    vec3 col = vec3(0);
    vec3 color = _color * (1 - redness) + _color * redness * atmFilter;

    float dist = distance(lightPos, worldPos);

    dist = max(dist - sunRad, 0);

    float asd = sunSize / 10;
    dist += asd;
    float strength = max(1 - dist, 0);
    if (strength > 0)
    {
        strength *= 2.0f;
        vec3 sunCol = 1 * pow(strength, density) * color;
        col = max(sunCol, col);
        col += sunCol;
    }

    // --------------------------------

    col += skyBrightness * (_colorSky * (1 - rednessSky) + _colorSky * rednessSky * atmFilter);
    col *= correct;

    gl_FragColor = vec4(col, 1);
}
