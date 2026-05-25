#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform bool translucent;

in vec4 vertex; 
in vec4 color;
in vec4 normal;

out vec4 ic;
out vec4 l1;
out vec4 l2;
out vec4 n;
out vec4 v;

void main(void){
    vec4 lp1 = vec4(0, 9, -15, 1); //light position in world space
    vec4 lp2 = vec4(0, 9, 15, 1);
    l1 = normalize(V*lp1-V*M*vertex); //vector towards light in eye space
    l2 = normalize(V*lp2-V*M*vertex);
    v = normalize(vec4(0,0,0,1)-V*M*vertex); //vector towards viewer in eye space
    mat3 normalMatrix = transpose(inverse(mat3(M)));
    n = normalize(V * vec4(normalMatrix * vec3(normal), 0.0)); //normal vector in eye space

    
    if(translucent){
        ic = vec4(color.rgb, 0.2*color.a);
    }else{
        ic = color;
    }
    gl_Position=P*V*M*vertex;
}