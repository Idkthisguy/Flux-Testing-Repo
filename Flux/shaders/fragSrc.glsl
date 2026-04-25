#version 330 core
out vec4 FragColor;
in vec3 FragPos; in vec3 Normal; in vec2 TexCoords;
uniform vec3 viewPos;
uniform bool hasTexture;
uniform sampler2D albedo;
uniform vec3 matColor;
uniform float alpha;
#define MAX_POINT 8
#define MAX_SPOT  4
uniform vec3  dirLightDir; uniform vec3 dirLightColor; uniform float dirLightIntensity; uniform bool hasDirLight;
uniform int   numPointLights;
uniform vec3  pointPos[MAX_POINT]; uniform vec3 pointColor[MAX_POINT];
uniform float pointIntensity[MAX_POINT]; uniform float pointRange[MAX_POINT];
uniform int   numSpotLights;
uniform vec3  spotPos[MAX_SPOT]; uniform vec3 spotDir[MAX_SPOT];
uniform vec3  spotColor[MAX_SPOT]; uniform float spotIntensity[MAX_SPOT];
uniform float spotRange[MAX_SPOT]; uniform float spotInner[MAX_SPOT]; uniform float spotOuter[MAX_SPOT];
uniform vec3 surfaceColor; uniform float surfaceIntensity; uniform bool hasSurface;
vec3 base(){return hasTexture?texture(albedo,TexCoords).rgb:matColor;}
vec3 calcDir(vec3 n,vec3 vd){
    if(!hasDirLight)return vec3(0);
    vec3 ld=normalize(-dirLightDir);
    float d=max(dot(n,ld),0.0);
    vec3 ref=reflect(-ld,n);
    float s=pow(max(dot(vd,ref),0.0),32.0);
    return dirLightColor*dirLightIntensity*(0.2+d+0.5*s);
}
vec3 calcPoint(int i,vec3 n,vec3 vd){
    vec3 ld=pointPos[i]-FragPos; float dist=length(ld);
    if(dist>pointRange[i])return vec3(0);
    ld=normalize(ld);
    float d=max(dot(n,ld),0.0);
    vec3 ref=reflect(-ld,n); float s=pow(max(dot(vd,ref),0.0),32.0);
    float att=1.0/(1.0+0.09*dist+0.032*dist*dist);
    return pointColor[i]*pointIntensity[i]*att*(d+0.5*s);
}
vec3 calcSpot(int i,vec3 n,vec3 vd){
    vec3 ld=spotPos[i]-FragPos; float dist=length(ld);
    if(dist>spotRange[i])return vec3(0);
    ld=normalize(ld);
    float theta=dot(ld,normalize(-spotDir[i]));
    float eps=cos(radians(spotInner[i]))-cos(radians(spotOuter[i]));
    float fact=clamp((theta-cos(radians(spotOuter[i])))/eps,0.0,1.0);
    float d=max(dot(n,ld),0.0);
    vec3 ref=reflect(-ld,n); float s=pow(max(dot(vd,ref),0.0),32.0);
    float att=1.0/(1.0+0.09*dist+0.032*dist*dist);
    return spotColor[i]*spotIntensity[i]*att*fact*(d+0.5*s);
}
void main(){
    vec3 n=normalize(Normal); vec3 vd=normalize(viewPos-FragPos);
    vec3 b=base();
    vec3 light=vec3(0.05);
    light+=calcDir(n,vd);
    for(int i=0;i<numPointLights;i++) light+=calcPoint(i,n,vd);
    for(int i=0;i<numSpotLights;i++)  light+=calcSpot(i,n,vd);
    if(hasSurface) light+=surfaceColor*surfaceIntensity;
    FragColor=vec4(b*light,alpha);
}