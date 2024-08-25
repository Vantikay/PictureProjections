// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// Custom variables

#define MAX_PORTALS 8

uniform int screenWidth;
uniform int screenHeight;
uniform int isPortal;
uniform float fovy;

uniform mat4 lookAtMatrix;

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
	
	// Adjusts to allow portal texure to be screen size and maintain proportions
	// Assumes that edges in UV map have same proportions as in world coordinates
    
	if (isPortal == 1) {
		
		vec4 cameraView = lookAtMatrix * matModel * vec4(vertexPosition, 1.0);
		// apply perspective projection and scale for UV, using magic projection formula
		float d = screenHeight * 0.5 / tan(fovy * 3.1415926535897932 / 180 / 2);
		fragTexCoord.x = (cameraView.x * d / cameraView.z + screenWidth * 0.5) / screenWidth;
		fragTexCoord.y = (cameraView.y * d / cameraView.z + screenHeight * 0.5) / screenHeight;
		
	} else {
		fragTexCoord.x = (vertexTexCoord.x * screenHeight + 0.5 * (screenWidth - screenHeight)) / screenWidth;
		fragTexCoord.y = vertexTexCoord.y;
	}
	
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));
	
    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}