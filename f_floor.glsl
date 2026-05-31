#version 330

uniform sampler2D diffTex;
uniform sampler2D displTex;
uniform sampler2D normalTex;

uniform float parallaxScale = 0.05; 
uniform float parallaxBias  = 0;

out vec4 pixelColor;

in vec3 l1_tangent;
in vec3 l2_tangent;
in vec3 v_tangent;
in vec2 tc;

vec4 phong(vec3 mn, vec3 ml, vec3 mv, vec4 kd, vec4 ks) {
    vec3 mr = reflect(-ml, mn);
    float nl = clamp(dot(mn, ml), 0.0, 1.0);
    float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 30.0);
    return vec4(kd.rgb * nl, kd.a) + vec4(ks.rgb * rv, 0.0);
}

void main(void) {
    // Sample height (red channel) – ensure your displacement map is a true height map
    float height = texture(displTex, tc).r;

    // Parallax offset using tangent‑space view direction
    vec2 offset = (v_tangent.xy / v_tangent.z) * (height * parallaxScale + parallaxBias);
    vec2 newTC = tc + offset;

    // (Optional) Clamp to avoid ugly border artifacts
    // newTC = clamp(newTC, 0.0, 6.0);  // because your floor has 6x6 texture repeats

    vec4 texColor = texture(diffTex, newTC);
    vec3 normalTS = texture(normalTex, newTC).rgb * 2.0 - 1.0;
    vec3 mn = normalize(normalTS);

    vec3 mv = normalize(v_tangent);
    vec3 ml1 = normalize(l1_tangent);
    vec3 ml2 = normalize(l2_tangent);

    vec4 kd = texColor;
    vec4 ks = texColor / 2.0;

    vec4 contrib1 = phong(mn, ml1, mv, kd, ks);
    vec4 contrib2 = phong(mn, ml2, mv, kd, ks);

    pixelColor = clamp(contrib1 + contrib2, 0.0, 1.0);
}