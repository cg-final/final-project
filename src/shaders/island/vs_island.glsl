#version 330 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_texcoord;

out vec3 Positions;
out vec3 Normals;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 oceanPlane;
uniform mat4 lightSpaceMatrix;

void main()
{
    Positions = v_position;
	Normals = v_normal;
    TexCoords = v_texcoord.xy;

	vec4 worldPos =  model * vec4(v_position, 1.0);
	gl_ClipDistance[0] = dot(oceanPlane, worldPos);

	gl_Position = projection * view * worldPos;

	FragPosLightSpace = lightSpaceMatrix * model * vec4(v_position, 1.0);
}
