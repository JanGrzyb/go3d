#version 330

uniform sampler2D diffTex;
uniform sampler2D displTex;
uniform sampler2D normalTex;

out vec4 pixelColor;

in vec4 l1;
in vec4 l2;
in vec4 v;
in vec2 tc;
in vec4 n;

vec2 parallax(vec4 v, vec2 t, float h, float s){
    vec2 ti = -v.xy/s;
    float hi = -v.z/s;

    vec2 tc = t;
    float hc=h;
    float ht = texture(displTex, tc).r*h;

    if (v.z<=0) discard;

    if(v.z>0){
        while(hc>ht){
            tc = tc+ti;
            hc = hc+hi;
            ht = texture(displTex, tc).r*h;
        }
    }
    return tc;
}

vec4 phong(vec4 mn, vec4 ml, vec4 mv, vec4 kd, vec4 ks) {
    vec4  mr = reflect(-ml, mn);
    float nl = clamp(dot(mn, ml), 0.0, 1.0);
    float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 30.0);
    return vec4(kd.rgb * nl, kd.a) + vec4(ks.rgb * rv, 0.0);
}

void main(void) {
    vec4 mv = normalize(v);
    vec2 tn = parallax(mv, tc, 1.0, 120.0f);

    vec4 ml1 = normalize(l1);
    vec4 ml2 = normalize(l2);
    vec4 mn = normalize(vec4(texture(normalTex, tn).rgb*2-1, 0));
    

    vec4 kd = texture(diffTex, tn);
    vec4 ks = vec4(0.3f,0.3f,0.3f,1);

    vec4 contrib1 = phong(mn, ml1, mv, kd, ks);
    vec4 contrib2 = phong(mn, ml2, mv, kd, ks);

    pixelColor = clamp(contrib1 + contrib2, 0.0, 1.0);
    
}