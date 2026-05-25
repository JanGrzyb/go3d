#version 330

uniform sampler2D myTexture;

out vec4 pixelColor;

in vec4 ic;
in vec4 n;
in vec4 l1;
in vec4 l2;
in vec4 v;
in vec4 worldPos;
in vec2 tc;

vec4 phong(vec4 mn, vec4 ml, vec4 mv, vec4 kd, vec4 ks) {
    vec4  mr = reflect(-ml, mn);
    float nl = clamp(dot(mn, ml), 0.0, 1.0);
    float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 30.0);
    return vec4(kd.rgb * nl, kd.a) + vec4(ks.rgb * rv, 0.0);
}

void main(void){
    float gridSize = 0.6;
    float lineWidth = 0.025;

    float gx = mod(abs(worldPos.x), gridSize);
    float gz = mod(abs(worldPos.z), gridSize);
    //hoshi points at (-3.6,-3.6),(-3.6,0),(-3.6,3.6),(0,-3.6),(0,0),(0,3.6),(3.6,-3.6),(3.6,0),(3.6,3.6)
    bool nearHoshi = 
    length(worldPos.xz - vec2(-3.6, -3.6)) < 0.075 ||
        length(worldPos.xz - vec2(-3.6,  0.0)) < 0.08 ||
        length(worldPos.xz - vec2(-3.6,  3.6)) < 0.08 ||
        length(worldPos.xz - vec2( 0.0, -3.6)) < 0.08 ||
        length(worldPos.xz - vec2( 0.0,  0.0)) < 0.08 ||
        length(worldPos.xz - vec2( 0.0,  3.6)) < 0.08 ||
        length(worldPos.xz - vec2( 3.6, -3.6)) < 0.08 ||
        length(worldPos.xz - vec2( 3.6,  0.0)) < 0.08 ||
        length(worldPos.xz - vec2( 3.6,  3.6)) < 0.08; 
        
    bool onLine =
        gx < lineWidth || gx > gridSize - lineWidth ||
        gz < lineWidth || gz > gridSize - lineWidth;

    bool inGrid = 
        worldPos.x<=5.4+lineWidth && worldPos.x >=-5.4-lineWidth &&
        worldPos.z<=5.4+lineWidth && worldPos.z >=-5.4-lineWidth;
    if(onLine && inGrid || inGrid && nearHoshi){
        pixelColor = vec4(0,0,0,1);
    }
    else{
        vec4 mn = normalize(n);
        vec4 mv = normalize(v);

        vec4 texColor = texture(myTexture, tc);
        vec4 kd = texColor; //ic jeżeli chcę kolory zamiast textury
        vec4 ks = texColor/2;

        vec4 contrib1 = phong(mn, normalize(l1), mv, kd, ks);
        vec4 contrib2 = phong(mn, normalize(l2), mv, kd, ks);

        pixelColor = clamp(contrib1 + contrib2, 0.0, 1.0);
    }
}