layout (location = 0) in vec3 aPos;
layout (location = 4) in ivec4 aBoneIDs;
layout (location = 5) in vec4 aWeights;

layout (location = 6) in vec4 aInstanceMatrixCol0;
layout (location = 7) in vec4 aInstanceMatrixCol1;
layout (location = 8) in vec4 aInstanceMatrixCol2;
layout (location = 9) in vec4 aInstanceMatrixCol3;

out VS_OUT {
    vec3 FragPos;
    vec3 RayOrigin;
    vec3 RayDirection;
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
    // Apply the skinning matrix to the vertex position before transforming it into light space
    vec4 worldPosition = modelMatrix * skinning * vec4(aPos, 1.0);
    gl_Position = M_LIGHT_SPACE * worldPosition;

    vs_out.FragPos = (M_LIGHT_SPACE * worldPosition).xyz;
    //vs_out.FragPos = modelMatrix * skinning * vec4(aPos, 1.0);

    //vs_out.RayOrigin = CAMERA_POS;
    //vec3 worldSpaceViewForward = -vec3(M_CAMERA_I_VIEW[2][0], M_CAMERA_I_VIEW[2][1], M_CAMERA_I_VIEW[2][2]);
    //vec3 worldRayDir = worldPosition.xyz - CAMERA_POS;
    //worldRayDir = worldSpaceViewForward * -dot(worldRayDir, worldSpaceViewForward);
    //vs_out.RayDirection = worldRayDir;

    vs_out.RayOrigin = worldPosition.xyz - LIGHT_DIRECTION * 5.0;  // Offset by a small amount to prevent self-shadowing

    // Set RayDirection to the normalized light direction, since this is a directional light
    vs_out.RayDirection = normalize(LIGHT_DIRECTION);  // Light direction should be normalized and negative if light travels along the direction vector
}
