#version 330 core

in vec2 TexCoords;
out vec4 FragColour;

uniform sampler2D screenTexture;

const float offset = 1.0f / 300.0f;

void main()
{
    vec3 tintColour = vec3(1.0f, 1.0f, 1.0f);

    vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right
    );

    float kernel[9] = float[](

        -1.0f, -0.5f, 1.0f,
        -0.5f, 2.0f, 0.5f,
        -1.0f, -0.5f, 1.0f
    );

    vec3 col = vec3(0.0f);

    for(int i = 0; i < 9; i++){

        col += vec3(texture(screenTexture, TexCoords + offsets[i])) * kernel[i];
    }

//    vec4 texCol = texture(screenTexture, TexCoords);
//    float average = (texCol.r + texCol.g + texCol.b) / 3.0f;

    FragColour = vec4(vec3(col), 1.0f);
}