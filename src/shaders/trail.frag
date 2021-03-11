#version 330 core

in float t;

out vec4 frag_color;

void main()
{
    const vec4 color1={0.f,1.f,0.f,1.f};// Green
    const vec4 color2={1.f,.9f,0.f,1.f};// Yellow
    const vec4 color3={1.f,0.f,0.f,1.f};// Red
if(t<=.5f){
    float new_t=t*2.;
    frag_color=(1-new_t)*color1+new_t*color2;
}else{
    float new_t=(t-.5f)*2;
    frag_color=(1-new_t)*color2+new_t*color3;
}
}