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

uniform vec3 sunColor = vec3(10, 10, 3);
uniform vec3 skyColor = vec3(3, 3, 3);
uniform float scatterPow = 0.65f;

uniform float sunRad = 0.08f;

uniform float bounceWeight = 3.8f;
uniform float atmDistWeight = 0.7f;
uniform float atmPow = 10.00f;
uniform float atmSunWeight = 0.10f;

uniform float miaScatter = 0.0f;
uniform float rayleighScatter = 0.382f;
uniform vec3 rayleighColor = vec3(0.005f, 0.441f, 0.991f);
//uniform vec3 rayleighColor2 = vec3(0.9f, 0.182f, 0.126f);

float calcBounces(vec3 vertexPos, vec3 sunPos)
{
    float _cos = dot(vertexPos, sunPos);
    //_cos = clamp(_cos, 0, 1);
    _cos = (_cos + 1) / 2;
    return pow(1 - _cos, scatterPow) * bounceWeight;
}

float calcDistanceSpentInAtmosphere(vec3 vertexPos) 
{
    float _cos = dot(vertexPos, vec3(0, 1, 0));
    return pow((1 - _cos), atmPow) * atmDistWeight;
}

float calcMiaScatter(float bounces)
{
    return bounces * miaScatter;
}

vec3 calcRayleighScatter(float bounces)
{
    return pow(rayleighColor, vec3(rayleighScatter * bounces));
}

vec3 calcRayleighFilter(float distInAtm, float distFromLight)
{
    return pow(vec3(1) - rayleighColor, vec3(distInAtm) * (1 / (distFromLight + atmSunWeight))) ;
}

vec3 applyMiaScatter(vec3 col, float mia)
{
    return col - col * mia / 10.0f;
}

void main ()
{
    vec3 worldPos = normalize(worldPos2);
    vec3 lightPos = normalize(lightPos);
    float lightVertDistance = distance(lightPos, worldPos);
    lightVertDistance = max(lightVertDistance - sunRad * 2, 0);

    float bounces = calcBounces(worldPos, lightPos);
    float distInAtm = calcDistanceSpentInAtmosphere(worldPos);
    float miaScatter = calcMiaScatter(bounces);
    vec3 rayleighScatter = calcRayleighScatter(bounces);
    vec3 rayleighFilter = calcRayleighFilter(distInAtm, lightVertDistance);

    vec3 color = vec3(0);
    if (lightVertDistance == 0)
    {
        color = sunColor * rayleighFilter;
    }
    else
    {
        color = skyColor * rayleighFilter * rayleighScatter;
        color = applyMiaScatter(color, miaScatter);
    }
    gl_FragColor = vec4(color, 1);

    // ********************************
   
    // dist += sunSize;
    // float strength = max(1 - dist, 0);
    // if (strength > 0)
    // {
    //     //strength *= 2.0f;
    //     vec3 sunCol = 1 * pow(strength, density) * color;
    //     col = max(sunCol, col);
    //     col += sunCol;
    // }

    // --------------------------------
    //dist2 = (clamp(dist2, sunRad, sunRad2) - sunRad) * (1.0f / (sunRad2 - sunRad));

    //// dist += sunSize;
    //float strength2 = max(1 - dist2, 0) * sunBorderStr * skyBrightness;
    //if (strength2 > 0)
    //{
    //    //strength *= 2.0f;
    //    vec3 sunCol = 1 * pow(strength2, density2) * color;
    //    // col = max(sunCol, col);
    //    col += sunCol; // * strength2 / 10;
    //}
    //// --------------------------------

    //col += skyBrightness * (_colorSky * (1 - rednessSky) + _colorSky * rednessSky * atmFilter);
    //col *= correct;

}
