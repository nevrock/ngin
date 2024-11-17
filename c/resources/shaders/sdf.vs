layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 4) in ivec4 aBoneIDs;
layout (location = 5) in vec4 aWeights;

layout (location = 6) in vec4 aInstanceMatrixCol0;
layout (location = 7) in vec4 aInstanceMatrixCol1;
layout (location = 8) in vec4 aInstanceMatrixCol2;
layout (location = 9) in vec4 aInstanceMatrixCol3;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec2 ScreenUV; // Added to pass screen UV coordinates
    vec4 FragPosLightSpace;
    vec3 RayOrigin;
    vec3 RayDirection;
} vs_out;

vec3 vertBase(vec3 vertex)
{
    bool isOrtho = M_CAMERA_PROJECTION[3][3] == 1.0;
    vec3 worldSpaceViewerPos = vec3(M_CAMERA_I_VIEW[3][0], M_CAMERA_I_VIEW[3][1], M_CAMERA_I_VIEW[3][2]);
    vec3 worldSpaceViewForward = vec3(M_CAMERA_I_VIEW[2][0], M_CAMERA_I_VIEW[2][1], M_CAMERA_I_VIEW[2][2]);

    // pivot position
    vec3 worldSpacePivotPos = vec3(M_MODEL[3][0], M_MODEL[3][1], M_MODEL[3][2]);

    // offset between pivot and camera
    vec3 worldSpacePivotToView = worldSpaceViewerPos - worldSpacePivotPos;
    
    // get the max object scale
    vec3 scale = vec3(
        length(vec3(M_MODEL[0][0],M_MODEL[0][1],M_MODEL[0][2])),
        length(vec3(M_MODEL[1][0],M_MODEL[1][1],M_MODEL[1][2])),
        length(vec3(M_MODEL[2][0],M_MODEL[2][1],M_MODEL[2][2]))
    );
    float maxScale = max(abs(scale.x), max(abs(scale.y), abs(scale.z)));

    // calculate a camera facing rotation matrix
    vec3 up = vec3(M_CAMERA_I_VIEW[1][0],M_CAMERA_I_VIEW[1][1],M_CAMERA_I_VIEW[1][2]);
    vec3 forward = isOrtho ? -worldSpaceViewForward : normalize(worldSpacePivotToView);
    vec3 right = normalize(cross(forward, up));
    up = cross(right, forward);
    mat3 quadOrientationMatrix = mat3(right, up, forward);

    // use the max scale to figure out how big the quad needs to be to cover the entire sphere
    // we're using a hardcoded object space radius of 0.5 in the fragment shader
    float maxRadius = maxScale * 0.5;

    // find the radius of a cone that contains the sphere with the point at the camera and the base at the pivot of the sphere
    // this means the quad is always scaled to perfectly cover only the area the sphere is visible within
    float quadScale = maxScale;
    if (!isOrtho)
    {
        // get the sine of the right triangle with the hyp of the sphere pivot distance and the opp of the sphere radius
        float sinAngle = maxRadius / length(worldSpacePivotToView);
        // convert to cosine
        float cosAngle = sqrt(1.0 - sinAngle * sinAngle);
        // convert to tangent
        float tanAngle = sinAngle / cosAngle;

        // get the opp of the right triangle with the 90 degree at the sphere pivot * 2
        quadScale = tanAngle * length(worldSpacePivotToView) * 2.0;
    }

    // flatten mesh, in case it's a cube or sloped quad mesh
    vertex.z = 0.0;

    // calculate world space position for the camera facing quad
    vec3 worldPos = ((vertex.xyz * quadScale) * quadOrientationMatrix) + worldSpacePivotPos;

    // calculate world space view ray direction and origin for perspective or orthographic
    vec3 worldSpaceRayOrigin = worldSpaceViewerPos;
    vec3 worldSpaceRayDir = worldPos - worldSpaceRayOrigin;
    if (isOrtho)
    {
        worldSpaceRayDir = worldSpaceViewForward * -dot(worldSpacePivotToView, worldSpaceViewForward);
        worldSpaceRayOrigin = worldPos - worldSpaceRayDir;
    }
    
    vec3 rayDir = worldSpaceRayDir;
    vec3 rayOrigin = worldSpaceRayOrigin;

    return vertex;
}

void main() {
    mat4 instanceModelMatrix = mat4(aInstanceMatrixCol0, aInstanceMatrixCol1, aInstanceMatrixCol2, aInstanceMatrixCol3);
    
    mat4 modelMatrix = M_MODEL;
    if (IS_INSTANCE)
        modelMatrix = instanceModelMatrix;

    mat4 skinning = mat4(0.0);
    if (IS_ANIMATION) {
        skinning = mat4(0.0);
        int counter = 0;
        for (int i = 0; i < 4; i++) {
            int boneID = aBoneIDs[i];
            float weight = aWeights[i];
            if (boneID >= 0 && boneID < 40) {
                skinning += A_M_BONES[boneID] * weight;
                counter++;
            }
        }
        skinning = skinning / counter;
    } else {
        skinning = mat4(1.0);
    }

    vec4 worldPosition = modelMatrix * skinning * vec4(aPos, 1.0);
    vec4 normalTransformed = transpose(inverse(modelMatrix * skinning)) * vec4(aNormal, 0.0);

    vs_out.FragPos = vec3(worldPosition);
    vs_out.Normal = IS_ANIMATION ? vec3(normalTransformed) : aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = M_LIGHT_SPACE * worldPosition;

    bool isOrtho = M_CAMERA_PROJECTION[3][3] == 1.0;
    vec3 worldSpaceViewerPos = vec3(M_CAMERA_I_VIEW[3][0], M_CAMERA_I_VIEW[3][1], M_CAMERA_I_VIEW[3][2]);
    vec3 worldSpaceViewForward = -vec3(M_CAMERA_I_VIEW[2][0], M_CAMERA_I_VIEW[2][1], M_CAMERA_I_VIEW[2][2]);

    // pivot position
    vec3 worldSpacePivotPos = vec3(M_MODEL[3][0], M_MODEL[3][1], M_MODEL[3][2]);

    // offset between pivot and camera
    vec3 worldSpacePivotToView = worldSpaceViewerPos - worldSpacePivotPos;
    
    // get the max object scale
    vec3 scale = vec3(
        length(vec3(M_MODEL[0][0],M_MODEL[0][1],M_MODEL[0][2])),
        length(vec3(M_MODEL[1][0],M_MODEL[1][1],M_MODEL[1][2])),
        length(vec3(M_MODEL[2][0],M_MODEL[2][1],M_MODEL[2][2]))
    );
    float maxScale = max(abs(scale.x), max(abs(scale.y), abs(scale.z)));

    // calculate a camera facing rotation matrix
    vec3 up = vec3(M_CAMERA_I_VIEW[1][0],M_CAMERA_I_VIEW[1][1],M_CAMERA_I_VIEW[1][2]);
    vec3 forward = isOrtho ? -worldSpaceViewForward : normalize(worldSpacePivotToView);
    vec3 right = normalize(cross(forward, up));
    up = cross(right, forward);
    mat3 quadOrientationMatrix = mat3(right, up, forward);

    // use the max scale to figure out how big the quad needs to be to cover the entire sphere
    // we're using a hardcoded object space radius of 0.5 in the fragment shader
    float maxRadius = maxScale * 0.5;

    // find the radius of a cone that contains the sphere with the point at the camera and the base at the pivot of the sphere
    // this means the quad is always scaled to perfectly cover only the area the sphere is visible within
    float quadScale = maxScale;
    if (!isOrtho)
    {
        // get the sine of the right triangle with the hyp of the sphere pivot distance and the opp of the sphere radius
        float sinAngle = maxRadius / length(worldSpacePivotToView);
        // convert to cosine
        float cosAngle = sqrt(1.0 - sinAngle * sinAngle);
        // convert to tangent
        float tanAngle = sinAngle / cosAngle;

        // get the opp of the right triangle with the 90 degree at the sphere pivot * 2
        quadScale = tanAngle * length(worldSpacePivotToView) * 2.0;
    }

    vec3 vertexOut = aPos;

    // flatten mesh, in case it's a cube or sloped quad mesh
    //vertexOut.z = 0.0;

    // calculate world space position for the camera facing quad
    vec3 worldPos = ((vertexOut.xyz * quadScale) * quadOrientationMatrix) + worldSpacePivotPos;

    // calculate world space view ray direction and origin for perspective or orthographic
    vec3 worldSpaceRayOrigin = worldSpaceViewerPos;
    vec3 worldSpaceRayDir = worldPos - worldSpaceRayOrigin;
    if (isOrtho)
    {
        worldSpaceRayDir = worldSpaceViewForward * -dot(worldSpacePivotToView, worldSpaceViewForward);
        worldSpaceRayOrigin = worldPos - worldSpaceRayDir;
    }
    
    vec3 rayDir = worldSpaceRayDir;
    vec3 rayOrigin = worldSpaceRayOrigin;

    vs_out.FragPos = worldPos;

    vs_out.RayOrigin = rayOrigin;
    vs_out.RayDirection = rayDir;

    gl_Position = M_CAMERA_PROJECTION * M_CAMERA_VIEW * vec4(worldPos,1.0);
}
