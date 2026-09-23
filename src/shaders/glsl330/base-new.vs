#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matNormal;

out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    fragTexCoord = vertexTexCoord;
    
    if (length(vertexNormal) > 0.0) {
        // Pasamos las normales limpias orientadas al espacio de vista de la cámara
        vec3 normalSpace = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
        fragColor = vec4(normalSpace * 0.5 + vec3(0.5), 1.0);
    } else {
        fragColor = vertexColor;
    }

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
