#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
out vec2 UV;
uniform vec3  worldPos;
uniform float size;
uniform mat4  view;
uniform mat4  projection;
void main(){
    vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camUp    = vec3(view[0][1], view[1][1], view[2][1]);
    vec3 vPos = worldPos
    + camRight * aPos.x * size
    + camUp    * aPos.y * size;
    UV = aUV;
    gl_Position = projection * view * vec4(vPos, 1.0);
}