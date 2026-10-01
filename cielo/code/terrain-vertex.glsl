#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 InTex;
layout(location = 2) in vec3 InNormal;

uniform mat4 gVP;
uniform float gMinHeight;
uniform float gMaxHeight;

out vec4 Color;
out vec2 Tex;
out vec3 WorldPos;
out vec3 Normal;

void main()
{
    gl_Position = gVP * vec4(Position, 1.0);

    // Make higher places brighter
    float DeltaHeight = gMaxHeight - gMinHeight;
    float HeightRatio = (Position.y - gMinHeight) / DeltaHeight;
    float c = HeightRatio * 0.8 + 0.2;
    Color = vec4(c, c, c, 1.0);

    Tex = InTex;

    WorldPos = Position;

    Normal = InNormal;
}

