#version 330

// Input vertex attributes from vertex shader
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

// Input uniform variables
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// Add custom variables here

#define MAX_LIGHTS 1
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1

struct Light {
	int enabled;
	int type;
	vec3 position;
	vec3 target;
	vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform vec4 ambient;

// portal variables
uniform int screenWidth;
uniform int screenHeight;
uniform int isPortal;
uniform float fovy;

// uses either picture view (portal disabled) or main camera (portal enabled)
// will run into texture wrapping issues if picture view is ever larger than whole screen,
// but that should never happen because game design.
uniform mat4 lookAtMatrix;

void main()
{
	// supercedes fragTexCoord
	vec2 texCoords;
	vec4 cameraView = lookAtMatrix * vec4(fragPosition, 1.0);
	// apply perspective projection and scale for UV, using magic projection formula
	float d = screenHeight * 0.5 / tan(fovy * 3.1415926535897932 / 180 / 2);
	texCoords.x = 1.0 - (cameraView.x * d / cameraView.z + screenWidth * 0.5) / screenWidth;
	texCoords.y = 1.0 - (cameraView.y * d / cameraView.z + screenHeight * 0.5) / screenHeight;
	
	vec4 texelColor = texture(texture0, texCoords);
	vec4 lightDot = vec4(0.0);
	vec3 normal = normalize(fragNormal);
	vec3 viewDirection = normalize(viewPos - fragPosition);
	
	// Implement your fragment shader code here
	
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights[i].enabled == 1) {
			vec3 lightDirection = vec3(0.0);
			
			if (lights[i].type == LIGHT_DIRECTIONAL)
			{
				lightDirection = normalize(lights[i].target - lights[i].position);
			}
			if (lights[i].type == LIGHT_POINT)
			{
				lightDirection = normalize(fragPosition - lights[i].position);
			}
			float NdotL = max(dot(normal, -1 * lightDirection), 0.0);
			lightDot = vec4(lights[i].color.rgb * NdotL, 1.0);
		}
	}
	float ambientScale = 0.3;
	vec4 ambientDot = ambient * ambientScale;
	if (dot(normal, vec3(0.0, 1.0, 0.0)) <= 0.0) {
			ambientDot *= sqrt(normal.x * normal.x + normal.z * normal.z);
	}
	finalColor = (colDiffuse + texelColor) * (lightDot + ambientDot);
	
	// Gamma correction
    // finalColor = pow(finalColor, vec4(1.0/2.2));
}
