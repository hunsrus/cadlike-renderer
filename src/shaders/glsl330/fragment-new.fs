#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 resolution;

uniform vec3 edgeColor;
uniform vec3 backgroundColor;

// Recibe explícitamente qué debe hacer el shader en este pase
uniform int renderMode; 

void main()
{
    // MODO 1: Escribir el mapa de normales sin procesar nada más
    if (renderMode == 1) 
    {
        finalColor = fragColor;
        return;
    }

    // MODO 2: Filtro Sobel Postprocesado (sobre el buffer modelTexture)
    float x = 1.0 / resolution.x;
    float y = 1.0 / resolution.y;

    vec4 centerTex = texture2D(texture0, fragTexCoord);
    
    // Si el píxel central está vacío (fondo transparente), no hay objeto
    if (centerTex.a < 0.1) 
    {
        discard; 
    }

    // Kernel Sobel Horizontal
    vec4 horizEdge = vec4(0.0);
    horizEdge -= texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y - y)) * 1.0;
    horizEdge -= texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y    )) * 2.0;
    horizEdge -= texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y + y)) * 1.0;
    horizEdge += texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y - y)) * 1.0;
    horizEdge += texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y    )) * 2.0;
    horizEdge += texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y + y)) * 1.0;

    // Kernel Sobel Vertical
    vec4 vertEdge = vec4(0.0);
    vertEdge -= texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y - y)) * 1.0;
    vertEdge -= texture2D(texture0, vec2(fragTexCoord.x    , fragTexCoord.y - y)) * 2.0;
    vertEdge -= texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y - y)) * 1.0;
    vertEdge += texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y + y)) * 1.0;
    vertEdge += texture2D(texture0, vec2(fragTexCoord.x    , fragTexCoord.y + y)) * 2.0;
    vertEdge += texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y + y)) * 1.0;

    // Evaluamos la magnitud de cambio basándonos estrictamente en las normales geométricas tridimensionales
    vec3 edge = sqrt((horizEdge.rgb * horizEdge.rgb) + (vertEdge.rgb * vertEdge.rgb));
    float intensity = max(edge.r, max(edge.g, edge.b));

    // Control del umbral para obtener líneas tipo CAD uniformes
    intensity = smoothstep(0.15, 0.25, intensity);

    vec3 finalEdgeColor = mix(backgroundColor, edgeColor, intensity);
    finalColor = vec4(finalEdgeColor, centerTex.a);
}
