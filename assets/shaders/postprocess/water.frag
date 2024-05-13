#version 330 core

in vec2 tex_coord;

// Sample colors of the object
uniform sampler2D tex;
// Make effect look like being under water
uniform sampler2D additional_sampler; 

out vec4 frag_color;

void main() {
    // get how much to distort from the water texture
    vec4 underwaterColor = texture(additional_sampler, tex_coord);
    vec2 distortion = (underwaterColor.xy - 0.5f) * 0.05;
    // take the color of the random pixel near current pixel
    frag_color = texture(tex, tex_coord + distortion);
}
