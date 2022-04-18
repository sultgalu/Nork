#type vertex
#version 330 core

layout(location = 0) in vec3 vPos;
uniform mat4 VP;

out vec3 v3Pos;

void main(void)
{
    v3Pos = vPos;
    gl_Position = VP * vec4(vPos, 1.0f);
}

uniform mat4 _gl_ModelViewProjectionMatrix;
uniform vec3 _v3CameraPos;     // The camera's current position    
uniform vec3 _v3LightDir;      // Direction vector to the light source    
uniform vec3 _v3InvWavelength; // 1 / pow(wavelength, 4) for RGB   
uniform float _fCameraHeight;    // The camera's current height      
uniform float _fCameraHeight2;   // fCameraHeight^2
uniform float _fOuterRadius;     // The outer (atmosphere) radius    
uniform float _fOuterRadius2;    // fOuterRadius^2
uniform float _fInnerRadius;    // The inner (planetary) radius    
uniform float _fInnerRadius2;    // fInnerRadius^2    
uniform float _fKrESun;			// Kr * ESun      
uniform float _fKmESun;          // Km * ESun    
uniform float _fKr4PI;           // Kr * 4 * PI    
uniform float _fKm4PI;           // Km * 4 * PI      
uniform float _fScale;           // 1 / (fOuterRadius - fInnerRadius)    
uniform float _fScaleOverScaleDepth; // fScale / fScaleDepth  

#type fragment
#version 330 core

uniform vec3 v3CameraPos;
uniform vec3 v3LightPos = vec3(-1.0f, -1.0f, -1.0f);
in vec3 v3Pos;
uniform vec3 v3InvWavelength = vec3(1.0f);
uniform float fCameraHeight = 1.0f;
uniform float fCameraHeight2 = 1.0f;
uniform float fInnerRadius = 1.0f;
uniform float fInnerRadius2 = 1.0f;
uniform float fOuterRadius = 1.0f;
uniform float fOuterRadius2 = 1.0f;
uniform float fKrESun = 1.0f;
uniform float fKmESun = 1.0f;
uniform float fKr4PI = 1.0f;
uniform float fKm4PI = 1.0f;
uniform float fScale = 1.0f;
uniform float fScaleDepth = 1.0f;
uniform float fScaleOverScaleDepth = 1.0f;
uniform float fSamples = 1.0f;
uniform int nSamples = 1;
const float g = -0.95;
const float g2 = g * g;

float scale(float fCos)
{
    float x = 1.0 - fCos;
    return fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

float getNearIntersection(vec3 pos, vec3 ray, float distance2, float radius2)
{
    float B = 2.0 * dot(pos, ray);
    float C = distance2 - radius2;
    float det = max(0.0, B * B - 4.0 * C);
    return 0.5 * (-B - sqrt(det));
}

void main ()
{
    vec3 v3Ray = v3Pos - v3CameraPos;
    float fFar = length(v3Ray);
    v3Ray /= fFar;

    float fStartOffset;
    vec3 v3Start;

    if (length(v3CameraPos) > fOuterRadius)
    {
        float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
        v3Start = v3CameraPos + v3Ray * fNear;
        fFar -= fNear;
        float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
        float fStartDepth = exp(-1.0 / fScaleDepth);
        fStartOffset = fStartDepth * scale(fStartAngle);
    }
    else
    {
        v3Start = v3CameraPos;
        float fHeight = length(v3Start);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
        float fStartAngle = dot(v3Ray, v3Start) / fHeight;
        fStartOffset = fDepth * scale(fStartAngle);
    }

    // Initialize the scattering loop variables
    float fSampleLength = fFar / fSamples;
    float fScaledLength = fSampleLength * fScale;
    vec3 v3SampleRay = v3Ray * fSampleLength;
    vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

    // Now loop through the sample rays
    vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < nSamples; i++)
    {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));
        vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }

    // Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
    vec4 secondaryColor = vec4(v3FrontColor * fKmESun, 1.0);
    vec4 primaryColor = vec4(v3FrontColor * (v3InvWavelength * fKrESun), 1.0);
    vec3 v3Direction = v3CameraPos - v3Pos;

    float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);
    float fRayleighPhase = 0.75 * (1.0 + fCos * fCos);
    float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos * fCos) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);
    gl_FragColor = fRayleighPhase * primaryColor + fMiePhase * secondaryColor;
    gl_FragColor = vec4(secondaryColor.rgb, 1);
}
