layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 4) in ivec4 aBoneIDs;
layout (location = 5) in vec4 aWeights;

layout (location = 6) in vec4 aInstanceMatrixCol0;
layout (location = 7) in vec4 aInstanceMatrixCol1;
layout (location = 8) in vec4 aInstanceMatrixCol2;
layout (location = 9) in vec4 aInstanceMatrixCol3;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

void main() {
    mat4 instanceModelMatrix = mat4(aInstanceMatrixCol0, aInstanceMatrixCol1, aInstanceMatrixCol2, aInstanceMatrixCol3);
    
    mat4 modelMatrix = M_MODEL;
    if (IS_INSTANCE)
        modelMatrix = instanceModelMatrix;

    mat4 skinning = mat4(0.0);
    if (IS_ANIMATION) {
        // Initialize skinning matrix as zero matrix
        skinning = mat4(0.0);
        // Accumulate the transformations from each affecting bone
        int counter = 0;
        for (int i = 0; i < 4; i++) {
            int boneID = aBoneIDs[i];
            float weight = aWeights[i];
            if (boneID >= 0 && boneID < 40) { // Validate bone index and ensure it's within bounds
                skinning += A_M_BONES[boneID] * weight;
                counter++;
            }
        }
        skinning = skinning/counter;
    } else {
        // No animation, just use the identity matrix
        skinning = mat4(1.0);
    }

    vec4 worldPosition = modelMatrix * skinning * vec4(aPos, 1.0);
    vec4 normalTransformed = transpose(inverse(modelMatrix * skinning)) * vec4(aNormal, 0.0);

    // Set the out variables
    vs_out.FragPos = vec3(worldPosition);
    vs_out.Normal = IS_ANIMATION ? vec3(normalTransformed) : aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = M_LIGHT_SPACE * worldPosition;

    // Apply the transformation to the clip space
    gl_Position = M_CAMERA_PROJECTION * M_CAMERA_VIEW * worldPosition;
}
