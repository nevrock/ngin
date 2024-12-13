out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{                        
    if (IS_TEXT) {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
        FragColor = vec4(textColor, 1.0) * sampled;
    } else {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
