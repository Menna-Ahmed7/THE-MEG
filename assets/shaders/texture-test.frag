#version 330 core

in Varyings {
    vec3 position;
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
} fs_in;

out vec4 frag_color;

uniform sampler2D tex;

void main(){
    //TODO: (Req 5) Change the following line to read the fragment color
    // from the texture at the received texture coordinates
    vec4 tex_color = texture(tex, fs_in.tex_coord);
    frag_color = tex_color;
}