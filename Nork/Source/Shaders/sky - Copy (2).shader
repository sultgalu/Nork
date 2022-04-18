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
uniform vec3 lightPos;

uniform vec3 colorTop = vec3(0.7f, 0.7f, 1.0f);
uniform vec3 colorTopDark = vec3(0.0f, 0.0f, 0.2f);
uniform vec2 topMinMax = vec2(0.5f, 1.5f);

uniform vec3 colorMiddle = vec3(0.8f, 0.8f, 0.3f);
uniform vec3 colorMiddleDark = vec3(0.0f, 0.0f, 0.0f);
uniform vec2 middleMinMax = vec2(-0.1f, 0.5f);

uniform vec3 colorBottom = vec3(0.9f, 0.2f, 0.2f);
uniform vec3 colorBottomDark = vec3(0.0f, 0.0f, 0.0f);
uniform vec2 bottomMinMax = vec2(-0.4f, 0.2f);


uniform float sunSize = 4.25f;
uniform float border = 0.4f;
uniform float density = 4.0f;
uniform float commonArea = 0.2f;

in vec3 worldPos2;

vec2 calc(float lightH, vec2 minMax, vec3 worldPos)
{
    float middle = (clamp(lightH, minMax.x, minMax.y) - minMax.x);
    //middle /= 2.0f;
    middle -= (minMax.y - minMax.x) / 2.0f;
    middle = abs(middle);
    vec2 ld = vec2((minMax.y - minMax.x) / 2.0f - middle, middle);
    //middle = (minMax.y - minMax.x) / 2.0f - middle;
    ld *= 5 * ((clamp(worldPos.y, minMax.x, minMax.y) - minMax.x));
    return ld;
}

void main ()
{
    vec3 worldPos = normalize(worldPos2);
    vec3 lightPos = normalize(lightPos);
    float lightH = lightPos.y;

    vec3 col = vec3(0);

    vec2 top = calc(lightH, topMinMax, worldPos);
    vec2 middle = calc(lightH, middleMinMax, worldPos);
    vec2 bottom = calc(lightH, bottomMinMax, worldPos);

    //bottom /= 2.0f;

    col += top.x * colorTop;
    col += middle.x * colorMiddle;
    col += bottom.x * colorBottom;
    col += top.y * colorTopDark;
    col += middle.y * colorMiddleDark;
    col += bottom.y * colorBottomDark;
    //col += (commonArea + clamp(lightH, -commonArea, border - commonArea)) * (1.0f / border) * colorTop;
    //col *= (worldPos.y + 1) * 0.5f; 
    //col += clamp(commonArea + (-lightH), -commonArea, border - commonArea) * colorBottom;

    float dist = distance(lightPos, worldPos);
    float asd = sunSize / 10;
    dist += asd;
    float strength = max(1 - dist, 0);
    if (strength > 0)
    {
        strength *= 2.0f;
        vec3 sunCol = 1 * pow(strength, density) * colorTop;
        //col = max(sunCol, col);
        col += sunCol;
    }

    gl_FragColor = vec4(col, 1);
}
