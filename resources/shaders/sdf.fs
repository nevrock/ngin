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
    vec3 Normal;
    vec2 TexCoords;
    vec2 ScreenUV; // Added to pass screen UV coordinates
    vec4 FragPosLightSpace;
    vec3 RayOrigin;
    vec3 RayDirection;
} fs_in;

struct Ray {
    vec3 origin;
    vec3 direction;
};

#define SURF_DIST 0.01
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
struct Surface {
    float distanceToSurface;
    vec4 diffuse;
    float outline;
};

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

mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}
mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0),
        vec3(-s, 0, c)
    );
}
mat3 rotateZ(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, -s, 0),
        vec3(s, c, 0),
        vec3(0, 0, 1)
    );
}
Surface blend(Surface s1, Surface s2, float blendStrength, int blendMode)
{
    if (blendMode == 0)
    {
        s1.distanceToSurface = opUnion(s1.distanceToSurface, s2.distanceToSurface);
        if (s1.distanceToSurface >= s2.distanceToSurface)
        {
            s1.diffuse = s2.diffuse;
        }
    }
    if (blendMode == 1)
    {
        s1.distanceToSurface = opSmoothUnion(s1.distanceToSurface, s2.distanceToSurface, blendStrength);
        if (s1.distanceToSurface >= s2.distanceToSurface)
        {
            s1.diffuse = s2.diffuse;
        }
    }
    return s1;
}
Surface getSurface(vec3 position) {
    Surface surface;
    surface.distanceToSurface = MAX_DIST;
    surface.diffuse = vec4(1,1,1,1);
    for (int i = 0; i < NUM_SURFACES; i++)
    {
        SurfaceData shape = surfaces[i];
        Surface shapeSurf;
        shapeSurf.diffuse = shape.diffuse;
        mat3 rotation = rotateX(-shape.rotation.x) * rotateY(-shape.rotation.y) * rotateZ(-shape.rotation.z);
        vec3 pos = position - shape.position.xyz;
        vec3 transformedPosition = rotation * (position - shape.position.xyz);

        shapeSurf.distanceToSurface = getSurfaceDistance(transformedPosition, shape);
        surface = blend(surface, shapeSurf, shape.blendStrength, shape.blendType);
    }
    return surface;
}

Surface rayMarch(vec3 ro, vec3 rd, float start, float end) {
    Surface surfaceOut;
    surfaceOut.distanceToSurface = MAX_DIST;
    surfaceOut.diffuse = vec4(0,0,0,0);

    float depth = start;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 pos = ro + depth * rd;
        float minDist = MAX_DIST;

        Surface surface = getSurface(pos);
        minDist = surface.distanceToSurface;

        surfaceOut.diffuse = surface.diffuse;

        if (minDist < SURF_DIST) {
            surfaceOut.distanceToSurface = depth;
            return surfaceOut;
        }
        depth += minDist;
        if (depth >= end) {
            break;
        }
    }
    surfaceOut.distanceToSurface = -1.0;
    return surfaceOut; // No hit
}
vec3 getNormal(vec3 surfPoint)
{
    float centerDistance = getSurface(surfPoint).distanceToSurface;
    float xDistance = getSurface(surfPoint + vec3(EPSILON, 0, 0)).distanceToSurface;
    float yDistance = getSurface(surfPoint + vec3(0, EPSILON, 0)).distanceToSurface;
    float zDistance = getSurface(surfPoint + vec3(0, 0, EPSILON)).distanceToSurface;
    vec3 normal = normalize(vec3(xDistance, yDistance, zDistance) - centerDistance);
    return normal;
}


float getShadowsPointSoft(vec3 fragPos, vec3 normalIn)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - POINT_LIGHT_POS;
    
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.55;
    int samples = 20;
    float viewDistance = length(CAMERA_POS - fragPos);
    float diskRadius = (1.0 + (viewDistance / LIGHT_FAR_PLANE)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(POINT_SHADOW_CUBE_MAP, fragToLight + SHADOW_GRID_SAMPLING_DISK[i] * diskRadius).r;
        closestDepth *= LIGHT_FAR_PLANE;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);  
        
    return shadow;
}
float getShadowsDirectionalSoft(vec4 fragPosLightSpace, vec3 normalIn) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(DIRECTIONAL_SHADOW_MAP, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(normalIn);
    float bias = max(0.05 * (1.0 - dot(normal, -LIGHT_DIRECTION)), 0.005);
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    // PCF
    vec2 texelSize = 1.0 / textureSize(DIRECTIONAL_SHADOW_MAP, 0);
    int size = 2;
    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(DIRECTIONAL_SHADOW_MAP, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 16.0;
    
    // keep the shadow at 0.0 when outside the CAMERA_FAR_PLANE region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}
vec3 getColor(vec3 color, vec3 normalIn) {
    vec3 normal = normalize(normalIn);
    vec3 lightColor = vec3(0.8);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDirFromPos = normalize(POINT_LIGHT_POS - fs_in.FragPos);
    float diff = max(dot(lightDirFromPos, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(CAMERA_POS - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDirFromPos, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDirFromPos + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = 0.0;                
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    return lighting;
}
vec3 getColorDirectional(vec3 color, vec3 normalIn) {
    vec3 normal = normalize(normalIn);
    vec3 lightColor = vec3(0.8);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    float diff = max(dot(-LIGHT_DIRECTION, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(CAMERA_POS - fs_in.FragPos);
    vec3 halfwayDir = normalize(-LIGHT_DIRECTION + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    return lighting;
}
vec3 getColorFromLights(vec3 colorIn, vec3 normal) {
    vec3 color = getColor(colorIn, normal);    
    vec3 colorDirectional = getColorDirectional(colorIn, normal);

    // Blend the two lighting results
    vec3 blendedColor = (color + colorDirectional) / 2.0;

    if (NUM_LIGHTS == 0) {
        blendedColor = colorDirectional;
    }

    float shadow = getShadowsPointSoft(fs_in.FragPos, normal);    

    float viewDistance = length(CAMERA_POS - fs_in.FragPos);
    shadow*=(1.0-min(1.0, max(0.5,(viewDistance / POINT_SHADOWS_THRESHOLD))));

    float shadowDirectional = getShadowsDirectionalSoft(fs_in.FragPosLightSpace, normal);
    if (NUM_LIGHTS > 0) {
        blendedColor *= ((1.0-min(0.4, max(shadow, shadowDirectional))));
    } else {
        blendedColor *= (1.0-min(0.4, shadowDirectional));
    }

    return blendedColor;
}





void main()
{                   
    vec3 rayOrigin = fs_in.RayOrigin;
    vec3 rayDir = normalize(fs_in.RayDirection);

    Surface surface = rayMarch(rayOrigin, rayDir, 0.0, MAX_DIST);
    if (surface.distanceToSurface < 0.0) {
        discard;
    }
    vec3 worldSpaceNew = rayDir * surface.distanceToSurface + rayOrigin;

    vec3 colorOut = getColorFromLights(surface.diffuse.rgb, getNormal(worldSpaceNew));

    FragColor = vec4(colorOut.rgb, 1.0);

    vec4 clipPos = M_CAMERA_PROJECTION * M_CAMERA_VIEW * vec4(worldSpaceNew, 1.0);
    float outDepth = clipPos.z / clipPos.w;
    outDepth = outDepth * 0.5 + 0.5;

    gl_FragDepth = outDepth;
}
