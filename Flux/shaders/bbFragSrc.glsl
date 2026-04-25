#version 330 core
in vec2 UV;
out vec4 FragColor;
uniform sampler2D icon;
void main(){
    vec4 c = texture(icon, UV);
    if(c.a < 0.05) discard;
    FragColor = c;
}