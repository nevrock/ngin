struct SurfaceData {
    vec4 position;
    vec4 rotation;
    vec4 scale;
    vec4 diffuse;
    int shapeType;
    int blendType;
    float blendStrength;
    int outline;
};

layout(std430, binding = 12) buffer Surfaces {
    SurfaceData surfaces[];
};

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 RayOrigin;
    vec3 RayDirection;
} fs_in;

struct Ray {
    vec3 origin;
    vec3 direction;
};

#define SURF_DIST 0.1
#define MAX_STEPS 150
#define MAX_DIST 300.0
#define EPSILON 0.001
#define SHADOW_BIAS 0.05

Ray createRay(vec3 origin, vec3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    return ray;
}

// Basic SDF (Signed Distance Function) for a sphere for demonstration
float sphereSDF(vec3 p, vec4 sph) {
    return length(p - sph.xyz) - sph.w;
}

float getSurfaceDistance(vec3 p, SurfaceData surface) {
    if (surface.shapeType == 0) {
        return sdEllipsoid(p, surface.scale.xyz);
    } else if (surface.shapeType == 1) {
        return sdBox(p, surface.scale.xyz/2.0);
    } else if (surface.shapeType == 2) {
        return sdPlane(p, vec3(0,1,0), 0);
    }
    float d1 = sdSphere(p, 0.5);
    return d1;
}

float getDistance(vec3 p, SurfaceData d) {
    p = p - d.position.xyz;
    return getSurfaceDistance(p, d);
}


// Raymarching function
float rayMarch(vec3 ro, vec3 rd, float start, float end) {
    float depth = start;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 pos = ro + depth * rd;
        float minDist = MAX_DIST;
        for (int j = 0; j < NUM_SURFACES; j++) {
            SurfaceData surface = surfaces[j];
            float dist = getDistance(pos, surface);
            minDist = min(minDist, dist);
        }
        if (minDist < EPSILON) {
            return depth;
        }
        depth += minDist;
        if (depth >= end) {
            break;
        }
    }
    return -1.0; // No hit
}

void main()
{             
    gl_FragDepth = gl_FragCoord.z;

    if (IS_SDF) {
        vec3 rayOrigin = fs_in.RayOrigin;
        vec3 rayDir = normalize(fs_in.RayDirection);

        vec3 spherePos = vec3(M_MODEL[3][0], M_MODEL[3][1], M_MODEL[3][2]);

        float rayHit = rayMarch(rayOrigin, rayDir, 0.0, MAX_DIST);

        if (rayHit < 0.0) {
            discard;
        }

        vec3 worldSpaceNew = rayDir * rayHit + rayOrigin;

        vec4 clipPos = M_LIGHT_SPACE * vec4(worldSpaceNew, 1.0);
        float outDepth = clipPos.z / clipPos.w;
        outDepth = outDepth * 0.5 + 0.5;

        gl_FragDepth = outDepth;
    }
}

