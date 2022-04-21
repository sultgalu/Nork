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

uniform vec3 lightPos = vec3(0.58f, 0.36f, -0.26f);

in vec3 worldPos2;

uniform vec3 sunColor = vec3(10.0f);
uniform vec3 baseColor = vec3(10.0f);
uniform vec3 inscatterColor = vec3(0.06f, 0.28f, 1.0f);
uniform vec3 miaScatterColor = vec3(0.6f);
uniform vec3 extinctionColor = vec3(1.0f, 0.193f, 0.042f);

uniform float sunRad = 0.03;
uniform vec2 atmDistMinMax = vec2(2, 4);

uniform float atmPlainness = 3.0f;

uniform float brightnessPow = 7.0f;
uniform float scatterPow = 0.6f;
uniform vec3 outscatterPtg = vec3(0.01f, 0.05f, 0.18f);

uniform float extVolume = 1.0f;
uniform float atmShadowPow = 1.0f;
uniform float atmShadowBias = 0.5f;

vec3 wavelengths[3] = {
    //vec3(0.4, 0, 0.4),
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1),
    //vec3(0.4, 0, 0)
};

vec3 getInscattering(float distFromSun, float atmDist, vec3 scatterColor)
{
    float brightness = (1 - distFromSun) * 10 + 1; // [1;10]
    float blueFactor = distFromSun * 10 + 1; // [1;10]
    blueFactor = pow(blueFactor, scatterPow);
    vec3 color = baseColor;
    // color *= pow(brightness, brightnessPow);
    color *= pow(atmDist, brightnessPow);
    color *= pow(scatterColor, vec3(blueFactor));
    return color;
    //return (4 - distFromSun) * baseColor * inscatterColor * (distFromSun + 1) * atmDist;
}

vec3 applyExtinction(vec3 color, float distFromSun, float atmDistVert, float atmDistSun, vec3 extColor)
{
    if (extVolume == 0)
        return color;
    return  color * pow(extColor, vec3(atmDistVert - atmDistMinMax.x + atmDistSun - atmDistMinMax.x));
}

void main ()
{
    vec3 worldPos = normalize(worldPos2);
    vec3 lightPos = normalize(lightPos);

    float distanceFromLight = distance(worldPos, lightPos);
    distanceFromLight = (clamp(distanceFromLight - sunRad, 0, 2 - sunRad)) / (2.0f - sunRad);

    float lightAngle = acos(dot(lightPos, vec3(0, 1, 0)));
    float lightAngleN = clamp(lightAngle / (3.15f / 2), 0, 1);
    float vertAngle = acos(dot(worldPos, vec3(0, 1, 0)));
    float vertAngleN = clamp(vertAngle / (3.15f / 2), 0, 1);
    vertAngleN = pow(vertAngleN, atmPlainness); 
    float atmDistVert = atmDistMinMax.x + (atmDistMinMax.y - atmDistMinMax.x) * vertAngleN;
    float atmDistSun = atmDistMinMax.x + (atmDistMinMax.y - atmDistMinMax.x) * lightAngleN;

    //float atmInShadow = pow(distanceFromLight, atmShadowPow);

    float atmInShadow = pow(1 - min(lightPos.y + atmShadowBias, 1), atmShadowPow);

    vec3 color = vec3(0);
    if (distanceFromLight == 0)
    {
        color = sunColor;
    }
    else
    {
        color += getInscattering(distanceFromLight, atmDistVert, inscatterColor) * (1 - atmInShadow);
        
        //color += getInscattering(distanceFromLight, atmDist, miaScatterColor);
    }
    color = applyExtinction(color, distanceFromLight, atmDistVert, atmDistSun, extinctionColor);
    color = applyExtinction(color, distanceFromLight, atmDistVert, atmDistSun, miaScatterColor);
    //color = applyOutscattering(vertAngleN, color, distanceFromLight);

    gl_FragColor = vec4(color, 1);
}
