#version 410 core

uniform sampler2D fbo;

layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 fragColorOut;

void main() {
    vec4 texel = texture( fbo, texCoord );
    
    // TODO #A
    float sum = texel.r + texel.g + texel.b;
    float average = sum/3.0;

    float dist = length(texCoord - vec2(0.5, 0.5));
    texel = vec4(average * 0.1, average * 0.95, average * 0.2, 1.0);
    if (dist > 0.48)
        texel = vec4(0.0, 0.0, 0.0, 1.0);
    else if (texCoord.s >= 0.495 && texCoord.s <= 0.505 && texCoord.t >= 0.405 && texCoord.t <= 0.595)
        texel = vec4(1.0, 0.0, 0.0, 1.0);
    else if (texCoord.t >= 0.495 && texCoord.t <= 0.505 && texCoord.s >= 0.405 && texCoord.s <= 0.595)
        texel = vec4(1.0, 0.0, 0.0, 1.0);

    fragColorOut = clamp(texel, 0.0, 1.0);
}
