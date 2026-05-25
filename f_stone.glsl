#version 330

out vec4 pixelColor;

in vec4 ic;
in vec4 n;
in vec4 l1;
in vec4 l2;
in vec4 v;

vec4 phong(vec4 mn, vec4 ml, vec4 mv, vec4 kd, vec4 ks) {
    vec4  mr = reflect(-ml, mn);
    float nl = clamp(dot(mn, ml), 0.0, 1.0);
    float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 10.0);
    return vec4(kd.rgb * nl, kd.a) + vec4(ks.rgb * rv, 0.0);
}

void main(void){

    vec4 mn = normalize(n);
    vec4 mv = normalize(v);

    vec4 kd = ic;
    vec4 ks = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    vec4 contrib1 = phong(mn, normalize(l1), mv, kd, ks);
    vec4 contrib2 = phong(mn, normalize(l2), mv, kd, ks);

    pixelColor = clamp(contrib1 + contrib2, 0.0, 1.0);

}