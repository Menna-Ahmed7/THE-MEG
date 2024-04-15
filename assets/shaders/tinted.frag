#version 330 core

in Varyings {
    vec4 color;
} fs_in;

out vec4 frag_color;

uniform vec4 tint;

void main(){
    //TODO: (Req 7) Modify the following line to compute the fragment color
    // by multiplying the tint with the vertex color
    //This line  multiplies the tint uniform (which represents a color) 
    //with the fs_in.color (which is the vertex color interpolated for the fragment).
    frag_color = tint * fs_in.color;
}