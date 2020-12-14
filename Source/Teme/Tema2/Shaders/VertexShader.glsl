#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 object_color;

uniform float autistic;
// Output value to fragment shader
out vec3 color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	color = object_color;
	if (autistic > 0) {
		gl_Position = Projection * View * Model * vec4(v_position.x + rand(v_position.yz) * 0.4f, v_position.y + rand(v_position.xz)  * 0.4f, v_position.z + rand(v_position.xy), 1.0)  * 0.4f;
	} else { 
		gl_Position = Projection * View * Model * vec4(v_position, 1.0);	
	}
}
