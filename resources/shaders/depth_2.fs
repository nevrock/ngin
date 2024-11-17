in vec4 FragPos;

void main()
{
    float lightDistance = length(FragPos.xyz - LIGHT_POS);
    
    // map to [0;1] range by dividing by LIGHT_FAR_PLANE
    lightDistance = lightDistance / LIGHT_FAR_PLANE;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}