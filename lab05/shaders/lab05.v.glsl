#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;                 // the precomputed Model-View-Projection Matrix
uniform mat4 modelMatrix;                 // the precomputed Model-View-Projection Matrix
uniform vec3 eyePos;
// TODO #D add our normal matrix
uniform mat3 normalMatrix;
// the normal matrix
// TODO #A add our light properties (direction & color)
uniform vec3 lightDirection, lightPosition, diffuseLightColor,
specularLightColor, ambientLightColor;
uniform vec3 spotLightDirection, spotLightPosition, diffuseSpotLightColor,
specularSpotLightColor, ambientSpotLightColor;
uniform float spotLightTheta;
// the direction the incident ray of light is traveling
// the color of the light
uniform vec3 materialColor;             // the material color for our vertex (& whole object)

// attribute inputs
layout(location = 0) in vec3 vPos;      // the position of this specific vertex in object space
// TODO #C add our vertex normal
in vec3 vertexNormal;
// the normal of this specific vertex in object space

// varying outputs
layout(location = 0) out vec3 color;    // color to apply to this vertex

void main() {
    // transform & output the vertex in clip space
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    // TODO #B convert the light direction to our normalized light vector

    float spotMag = sqrt(pow(spotLightDirection.x, 2.0) + pow(spotLightDirection.y, 2.0) + pow(spotLightDirection.z, 2.0));
    vec3 normSpotLightDir = vec3((-1 * spotLightDirection.x)/spotMag, (-1 * spotLightDirection.y)/spotMag, (-1 * spotLightDirection.z)/spotMag);

    //spot light vector math
    vec3 lightToPoint = spotLightPosition - vPos;
    float lightToPointDist =  sqrt(pow(lightToPoint.x, 2.0) + pow(lightToPoint.y, 2.0) + pow(lightToPoint.z, 2.0));
    float objectTheta = acos(dot(spotLightDirection, lightToPoint)/(spotMag * lightToPointDist));
    vec3 spotColor = vec3(0,0,0);
    // TODO #E transform the vertex normal in to world space
    vec3 normalizedVertexNorm = normalMatrix * vertexNormal;
    //specular reflectance math
    vec4 tempVec = vec4(eyePos,1.0f) - modelMatrix * vec4(vPos, 1.0f);
    float magnitude = sqrt(pow(tempVec.x, 2.0) + pow(tempVec.y, 2.0) + pow(tempVec.z, 2.0));
    vec3 viewVector = vec3(tempVec.x/magnitude, tempVec.y/magnitude, tempVec.z/magnitude);

    vec3 diffuse, specular, ambient;
    if (abs(objectTheta) <= spotLightTheta) {
        // TODO #F compute the diffuse component of the Phong Illumination Model
        diffuse = diffuseSpotLightColor * materialColor * max(dot(normSpotLightDir, normalizedVertexNorm), 0);

        specular = specularSpotLightColor * materialColor * max(dot((normSpotLightDir + viewVector), normalizedVertexNorm), 0);

        ambient = ambientSpotLightColor * materialColor;

        float attenFact = 0.3 + 0.02*lightToPointDist + 0.01*pow(lightToPointDist, 2);
        // TODO #G output the illumination color of this vertex
        color = (diffuse + specular + ambient) * attenFact;              // assign the color for this vertex
    }
//    magnitude = sqrt(pow(lightDirection.x, 2.0) + pow(lightDirection.y, 2.0) + pow(lightDirection.z, 2.0));
//    vec3 normLightDirection = vec3((-1 * lightDirection.x)/magnitude, (-1 * lightDirection.y)/magnitude, (-1 * lightDirection.z)/magnitude);
//    // TODO #F compute the diffuse component of the Phong Illumination Model
//    diffuse = diffuseLightColor * materialColor * max(dot(normLightDirection, normalizedVertexNorm), 0);
//
//    specular = specularLightColor * materialColor * max(dot((normLightDirection + viewVector), normalizedVertexNorm), 0);
//
//    ambient = ambientLightColor * materialColor;
//
//    // TODO #G output the illumination color of this vertex
//    vec3 directionalColor = diffuse + specular + ambient;              // assign the color for this vertex
//    color = spotColor + directionalColor;
}