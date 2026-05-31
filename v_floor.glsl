#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec4 vertex;
in vec4 normal;
in vec2 texCoord;

out vec3 l1_tangent;
out vec3 l2_tangent;
out vec3 v_tangent;
out vec2 tc;

void main(void) {
    // Object‑space tangent basis (the floor is a flat XY plane)
    vec3 objTangent  = vec3(1.0, 0.0, 0.0);
    vec3 objBitangent = vec3(0.0, 0.0, 1.0);
    vec3 objNormal   = vec3(0.0, 1.0, 0.0);

    // Transform to eye space using the normal matrix (rotation+scale)
    mat3 normalMatrix = transpose(inverse(mat3(M)));
    vec3 eyeNormal    = normalize(V * vec4(normalMatrix * objNormal, 0.0)).xyz;
    vec3 eyeTangent   = normalize(V * vec4(normalMatrix * objTangent, 0.0)).xyz;
    vec3 eyeBitangent = normalize(V * vec4(normalMatrix * objBitangent, 0.0)).xyz;

    // TBN matrix: from tangent space to eye space
    mat3 TBN = mat3(eyeTangent, eyeBitangent, eyeNormal);

    // Vertex position in eye space
    vec3 eyePos = (V * M * vertex).xyz;

    // View direction (from vertex to camera) in eye space
    vec3 eyeViewDir = normalize(-eyePos);

    // Light positions in world space (same as in your original shader)
    vec4 lp1_world = vec4(0.0, 9.0, -15.0, 1.0);
    vec4 lp2_world = vec4(0.0, 9.0,  15.0, 1.0);

    // Light directions in eye space
    vec3 lp1_eye = (V * lp1_world).xyz;
    vec3 lp2_eye = (V * lp2_world).xyz;
    vec3 eyeLightDir1 = normalize(lp1_eye - eyePos);
    vec3 eyeLightDir2 = normalize(lp2_eye - eyePos);

    // Transform to tangent space
    mat3 TBNt = transpose(TBN);      // eye space → tangent space
    v_tangent   = TBNt * eyeViewDir;
    l1_tangent  = TBNt * eyeLightDir1;
    l2_tangent  = TBNt * eyeLightDir2;

    tc = texCoord;
    gl_Position = P * V * M * vertex;
}