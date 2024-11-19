out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

float ShadowCalculationSoft(vec3 fragPos)
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

float ShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - POINT_LIGHT_POS;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(POINT_SHADOW_CUBE_MAP, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= LIGHT_FAR_PLANE;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.25; // we use a much larger bias since depth is now in [near_plane, CAMERA_FAR_PLANE] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / CAMERA_FAR_PLANE), 1.0);    
        
    return shadow;
}

float ShadowCalculationDirectional(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(DIRECTIONAL_SHADOW_MAP, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    float bias = max(0.05 * (1.0 - dot(normal, -LIGHT_DIRECTION)), 0.005);
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    // PCF
    vec2 texelSize = 1.0 / textureSize(DIRECTIONAL_SHADOW_MAP, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(DIRECTIONAL_SHADOW_MAP, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the CAMERA_FAR_PLANE region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float ShadowCalculationDirectionalSoft(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(DIRECTIONAL_SHADOW_MAP, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
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

vec3 getColor() {
    vec3 color;
    #ifdef TEX_DIFFUSE
        color = texture(TEX_DIFFUSE, fs_in.TexCoords).rgb;
    #else
        color = vec3(1.0, 1.0, 1.0); // Default to white
    #endif
    //vec3 color = texture(TEX_DIFFUSE, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
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
    //float shadow = ShadowCalculation(fs_in.FragPos);                      
    //float shadow = ShadowCalculationDirectional(fs_in.FragPosLightSpace); 
    float shadow = 0.0;                
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    return lighting;
}

vec3 getColorDirectional() {
    vec3 color;
    #ifdef TEX_DIFFUSE
        color = texture(TEX_DIFFUSE, fs_in.TexCoords).rgb;
    #else
        color = vec3(1.0, 1.0, 1.0); // Default to white
    #endif
    //vec3 color = texture(TEX_DIFFUSE, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
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
    //float shadow = ShadowCalculationDirectional(fs_in.FragPosLightSpace);
    float shadow = 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    return lighting;
}

vec3 getColorFromLights() {
    vec3 color = getColor();    
    vec3 colorDirectional = getColorDirectional();

    // Blend the two lighting results
    vec3 blendedColor = (color + colorDirectional) / 2.0;

    if (NUM_LIGHTS == 0) {
        blendedColor = colorDirectional;
    }

    float shadow = ShadowCalculationSoft(fs_in.FragPos);    

    float viewDistance = length(CAMERA_POS - fs_in.FragPos);
    shadow*=(1.0-min(1.0, max(0.5,(viewDistance / POINT_SHADOWS_THRESHOLD))));

    float shadowDirectional = ShadowCalculationDirectionalSoft(fs_in.FragPosLightSpace);
    if (NUM_LIGHTS > 0) {
        blendedColor *= ((1.0-min(0.4, max(shadow, shadowDirectional))));
    } else {
        blendedColor *= (1.0-min(0.4, shadowDirectional));
    }

    return blendedColor;
}

void main()
{                        
    vec3 blendedColor = getColorFromLights();
    FragColor = vec4(blendedColor, 1.0);
}
