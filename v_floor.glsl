#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 lightRotation;

in vec4 vertex;
in vec4 normal;
in vec2 texCoord;
in vec4 c1;
in vec4 c2;
in vec4 c3;


out vec4 l1;
out vec4 l2;
out vec4 v;
out vec4 n;
out vec2 tc;


void main(void) {
    mat4 invTBN = mat4(vec4(1,0,0,0), vec4(0,0,1,0), vec4(0,1,0,0), vec4(0,0,0,1));

    //mat4 invTBN = mat4(c1, c2, c3, vec4(0,0,0,1));

    vec4 lp1 = lightRotation * vec4(0, 9, -15, 1); //light position in world space
    vec4 lp2 = lightRotation * vec4(0, 9, 15, 1);

    l1 = normalize(invTBN*inverse(M)*(lp1-M*vertex)); //vector towards light in eye space
    l2 = normalize(invTBN*inverse(M)*(lp2-M*vertex));
    v = normalize(invTBN*inverse(V*M)*vec4(0,0,0,1) - (invTBN*vertex));
    n = vec4(0,0,1,0);
    
    

    tc = texCoord;
    gl_Position = P * V * M * vertex;
}