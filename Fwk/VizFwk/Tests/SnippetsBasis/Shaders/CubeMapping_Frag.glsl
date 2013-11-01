
uniform mat4 cvfu_viewMatrixInverse;
uniform mat4 cvfu_modelViewMatrix;                                
uniform mat3 cvfu_normalMatrix;                                             
uniform samplerCube u_cubeMap;                                               

// Rename after refactor of phong
uniform float ambientIntensity;
uniform vec3 color;
uniform float reflectivity;

varying vec3 v_ecPosition; 
varying vec3 v_ecNormal;   
//varying vec3 v_wcPosition;                                                   
//varying vec3 v_wcNormal;                                                     
                                                                           

//--------------------------------------------------------------------------------------------------
/// Compute cube map lookup vector using eye space coordinates
//--------------------------------------------------------------------------------------------------
vec3 cubeMapTexCoordsFromEyeCoords()
{
    // Viewing vector (from point to eye)                                  
	vec3 V = normalize(-v_ecPosition);                                       
    vec3 N = normalize(v_ecNormal);                                          
    vec3 R = normalize(reflect(-V, N));                                    
                                                                           
	// Transform the reflection VECTOR from eye coords to world
	vec3 wR = (cvfu_viewMatrixInverse*vec4(R, 0)).xyz;                                                 

	// Slightly more clumsy aternative
	//vec3 wR = (cvfu_viewMatrixInverse*vec4(R, 1)).xyz;                                                 
	//wR -= cvfu_viewMatrixInverse[3].xyz;

	return wR;
}


//--------------------------------------------------------------------------------------------------
/// Compute cube map lookup vector using world space coordinates
//--------------------------------------------------------------------------------------------------
/*
vec3 cubeMapTexCoordsFromWorldCoords()
{
	// Need to compute eye position in world coords
	vec4 ecEye = vec4(0, 0, 0, 1);                                                 
	vec3 wEye = (cvfu_viewMatrixInverse*ecEye).xyz;                                                 

	vec3 wV = normalize(wEye - v_wcPosition);                                       
	vec3 wN = normalize(v_wcNormal);                                          
	vec3 wR = normalize(reflect(-wV, wN));                                    

	return wR;
}
*/


//--------------------------------------------------------------------------------------------------
/// Using these coordinates has the effect of fixing the camera relative to the
/// cube map and then just rotating the model
//--------------------------------------------------------------------------------------------------
vec3 eyeSpaceCubeMapTexCoordsFromEyeCoords()
{
    // Viewing vector (from point to eye)                                  
	vec3 V = normalize(-v_ecPosition);                                       
    vec3 N = normalize(v_ecNormal);                                          
    vec3 R = normalize(reflect(-V, N));                                    

	return R;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
vec3 phongShadeFragment()                                                              
{                                                                         
	const vec3 ecHeadlightDirection = vec3(-0.5, -5.0, -7.0);                 
	const float specularIntensity = 0.5;                                  
	    
    // Light vector (from point to light source)                          
    vec3 L = normalize(-ecHeadlightDirection);                            
                                                                          
    // Viewing vector (from point to eye)                                 
    // Since we are in eye space, the eye pos is at (0, 0, 0)             
    vec3 V = normalize(-v_ecPosition);                                      
                                                                          
    vec3 N = normalize(v_ecNormal);                                         
    vec3 R = normalize(reflect(-L, N));                                   
                                                                          
    vec3 ambient  = color*ambientIntensity;                               
    vec3 diffuse  = color*(1.0 - ambientIntensity)*max(dot(N, L), 0.0);   
    vec3 specular = vec3(specularIntensity*pow(max(dot(R, V), 0.0), 8.0));
                                                                          
    vec3 clr = ambient + diffuse + specular;
	return clr;
}                                                                         

                                                                           
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void main ()                                                               
{                                                                          
	vec3 texCoord = cubeMapTexCoordsFromEyeCoords();
	//vec3 texCoord = cubeMapTexCoordsFromWorldCoords();
    vec3 clrCubeMap = textureCube(u_cubeMap, texCoord).xyz;                             

    vec3 clrPhong = phongShadeFragment();                             

    gl_FragColor = vec4(mix(clrPhong, clrCubeMap, reflectivity), 1.0);
}

