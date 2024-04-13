#version 330 core

out vec4 frag_color;

// In this shader, we want to draw a checkboard where the size of each tile is (size x size).
// The color of the top-left most tile should be "colors[0]" and the 2 tiles adjacent to it
// should have the color "colors[1]".

//TODO: (Req 1) Finish this shader.

uniform int size = 32;
uniform vec3 colors[2];

void main(){
    // Calculate the indices of the current tile
    int x = int(gl_FragCoord.x) / size;
    int y = int(gl_FragCoord.y) / size;

    // Determine the color of the current tile based on its position
    vec3 tileColor = colors[((x + y) % 2 == 0) ? 0 : 1];

    frag_color = vec4(tileColor, 1.0);
}