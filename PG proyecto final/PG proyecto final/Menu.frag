#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D menuTexture;

void main() {
    // 1. Verificar coordenadas de textura
    if(TexCoord.x < 0.0 || TexCoord.x > 1.0 || TexCoord.y < 0.0 || TexCoord.y > 1.0) {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Magenta si coordenadas son inválidas
        return;
    }
    
    // 2. Muestrear textura
    vec4 texColor = texture(menuTexture, TexCoord);
    
    // 3. Versión alternativa sin usar mod()
    float gridSize = 20.0;
    float xCoord = TexCoord.x * gridSize;
    float yCoord = TexCoord.y * gridSize;
    
    // Implementación manual de módulo usando floor
    float xMod = xCoord - floor(xCoord);
    float yMod = yCoord - floor(yCoord);
    bool chess = (xMod + yMod) < 1.0;
    
    // 4. Mostrar textura o patrón de debug
    if(texColor.a < 0.1) {
        FragColor = chess ? vec4(1.0, 1.0, 0.0, 1.0) : vec4(0.0, 0.0, 1.0, 1.0);
    } else {
        FragColor = texColor;
    }
}