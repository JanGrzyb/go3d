#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 lightRotation;


in vec4 vertex; 
in vec4 color;
in vec4 normal;
in vec2 texCoord;

out vec4 ic;
out vec4 l1;
out vec4 l2;
out vec4 n;
out vec4 v;
out vec4 worldPos;
out vec2 tc;

void main(void){
    vec4 lp1 = lightRotation * vec4(0, 9, -15, 1); //light position in world space
    vec4 lp2 = lightRotation * vec4(0, 9, 15, 1);
    l1 = normalize(V*lp1-V*M*vertex); //vector towards light in eye space
    l2 = normalize(V*lp2-V*M*vertex);
    v = normalize(vec4(0,0,0,1)-V*M*vertex); //vector towards viewer in eye space
    n = normalize(V*M*normal); //normal vector in eye space

    
    ic = color;
    worldPos = vertex;
    tc = texCoord;
    gl_Position=P*V*M*vertex;
}