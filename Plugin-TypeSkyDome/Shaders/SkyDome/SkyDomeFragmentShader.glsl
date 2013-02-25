

// vec4 getColorFromCubeMap( vec3 worldDirection )
// {
//     return texture( g_Texture0, worldDirection );
// }

uniform float uUpperCutOff;   // Missing upper part of the panorama in degree
uniform float uHorizontalFOV; // Horizontal FOV of the panorama in degree
uniform float uVerticalFOV;   // Vertical   FOV of the panorama in degree

uniform float uVP_width;  // viewport width
uniform float uVP_height; // viewport height

vec4 getColorFromPanorama( vec3 _worldDirection, float _verticalFOV, float _horizontalFOV, float _upperCutOff)
{
    float PI = 3.1415926;
    float lat = asin( _worldDirection.y );
    float lon = atan( _worldDirection.z, _worldDirection.x );

    lat /= 0.5*PI;
    lat += 1.0;
    lat /= 2.0;

    lon += PI;
    lon /= 2*PI;

    // Lower Cut Off
    // The reminder that is left when reoving upper cutoff and the vertical fov
    float lowerCutOff = (180 - _verticalFOV - _upperCutOff) /180;
    
    // bottom cutoff:
    vec2 coord = vec2( lon, lat );
    coord.y *= (1.0 + lowerCutOff);
    coord.y -= lowerCutOff;
    if (coord.y < 0.0) return vec4(0);

    float upperCutoffValue = _upperCutOff/180;
    
    // upper cutoff
    coord.y *= (1.0 + upperCutoffValue);
    if (coord.y > 1.0) return vec4(0);

    // limit horizontal FOV
    coord.x *= (360.0/_horizontalFOV);
    if (coord.x > 1.0) return vec4(0);

    return texture( g_Texture0, coord );
}



void main()
{

    mat4 invView = inverse( g_mWV);
    mat4 invProj = inverse( g_mP );

    vec2 relativePositionOnScreen = gl_FragCoord.xy / vec2( uVP_width,uVP_height );
    
    relativePositionOnScreen *= 2.0;
    relativePositionOnScreen -= 1.0; // -1..1

    vec4 eyeVector = invProj * vec4( relativePositionOnScreen, -1, 1 );
    eyeVector *= eyeVector.w;

    vec4 eyeRayDirection = vec4(eyeVector.xyz, 0.0);
    vec4 worldRayDirection = (invView * eyeRayDirection);

    worldRayDirection = normalize( worldRayDirection );

    vec4 color;

//    if (uSampleFromCubeMap) {
//        worldRayDirection.x *= -1; // WHY?
//
//        color = getColorFromCubeMap( worldRayDirection.xyz );
//    } else {
        // non-cubemap:
        //worldRayDirection *= -1;
        //color = getColorFromPanorama( worldRayDirection.xyz, uLowerCutOff, uUpperCutOff, uPanoFOV );
        color = getColorFromPanorama( worldRayDirection.xyz, uVerticalFOV, uHorizontalFOV, uUpperCutOff  );
//    }

//    color.rgb *= uHDRScale;

//    if (uPerformGammaCorrection) {
//        color.rgb = linearToGamma( color.rgb, uGamma );
//    }
    
    // Output color of the fragment
    outFragment = color;

}

