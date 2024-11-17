layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

layout (location = 6) in vec4 aInstanceMatrixCol0;
layout (location = 7) in vec4 aInstanceMatrixCol1;
layout (location = 8) in vec4 aInstanceMatrixCol2;
layout (location = 9) in vec4 aInstanceMatrixCol3;

out vec2 TexCoords;

void main() {
    mat4 instanceModelMatrix = mat4(aInstanceMatrixCol0, aInstanceMatrixCol1, aInstanceMatrixCol2, aInstanceMatrixCol3);
    mat4 modelMatrix = M_MODEL;
    if (IS_INSTANCE)
        modelMatrix = instanceModelMatrix;
    if (IS_TEXT) {
        gl_Position = M_CAMERA_PROJECTION * vec4(aPos.xy, 0.0, 1.0);
        TexCoords = aTexCoords;
    } else {
        gl_Position = modelMatrix * vec4(aPos, 1.0);
        TexCoords = aTexCoords;
    }   
}
