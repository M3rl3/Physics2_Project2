// Fragment shader
#version 420

// Coming in from the geometry shader
in vec4 fColour;
in vec4 fNormal;
in vec4 fWorldLocation;
in vec4 fUV2;
in vec4 fTangent;
in vec4 fBiNormal;
in vec4 fBoneID;
in vec4 fBoneWeight;

// Out to the screen
out vec4 outputColor;

// Uniforms
uniform vec4 RGBAColour;
uniform bool useRGBAColour;

uniform vec4 diffuseColour;
uniform vec4 specularColour;

uniform vec4 debugColour;
uniform bool doNotLight;

uniform vec4 eyeLocation;

uniform bool bHasTexture;

uniform sampler2D texture0;	
uniform sampler2D texture1;	
uniform sampler2D texture2;	
uniform sampler2D texture3;	
uniform sampler2D texture4;	
uniform sampler2D texture5;	
uniform sampler2D texture6;	
uniform sampler2D texture7;	

uniform vec4 texRatio_0_3;	
uniform vec4 texRatio_4_7;

uniform samplerCube skyboxTexture;

// If true, applies the skybox texture
uniform bool bIsSkyboxObject;

// If it is a terrain mesh
uniform bool bIsTerrainMesh;

// Frame Buffer
uniform bool bIsFullScreenQuad;
uniform sampler2D crosshair_texture;
uniform sampler2D FBO_Texture;		// Color texture from the FBO
uniform vec2 FBO_size;				// x = width, y = height
uniform vec2 screen_size;			// x = width, y = height

// Lighting
struct sLight
{
	vec4 position;			
	vec4 diffuse;	
	vec4 specular;	// rgb = highlight colour, w = power
	vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
	vec4 direction;	// Spot, directional lights
	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
	                // 0 = pointlight
					// 1 = spot light
					// 2 = directional light
	vec4 param2;	// x = 0 for off, 1 for on
};

const int PointLight = 0;
const int SpotLight = 1;
const int DirectionalLight = 2;

uniform int NUMBEROFLIGHTS;
uniform sLight sLightsArray[20];

uniform float ambientLight;

vec4 calculateLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular );

void main()
{
	
	if (bIsFullScreenQuad) 
	{
		float screen_width = screen_size.x;
		float screen_height = screen_size.y;
		
		vec2 screen_coordinates = vec2(gl_FragCoord.x / screen_width, gl_FragCoord.y / screen_height);

		vec3 pixelColour = vec3(0.0f, 0.0f, 0.0f);
		vec3 crosshairColor = vec3(0.0f, 0.0f, 0.0f);

		pixelColour = texture(FBO_Texture, screen_coordinates ).rgb;
		crosshairColor = texture(crosshair_texture, screen_coordinates).rgb;

		outputColor.rgb = (pixelColour.rgb * 0.9f) +
		 				  (crosshairColor.rgb * 0.25f);

		// outputColor = vec4(mix(pixelColour, crosshairColor, 0.15f), 1.f) + vec4(0.135, 0.135, 0.135, 1);

		// Set the alpha transparency based on the colour.
		float RGBcolorSum = outputColor.r + outputColor.g + outputColor.b;
		outputColor.a = max(((RGBcolorSum - 0.1f) / 3.f), 0.f);
		
		return;
 	}

	if (bIsSkyboxObject)
	{
		vec3 cubeMapColour = texture(skyboxTexture, fNormal.xyz).rgb;
		outputColor.rgb = cubeMapColour.rgb;
		outputColor.a = 1.0f;

		// Apply the skybox texture and return without lighting
		return;
	}

	if (bIsTerrainMesh)
	{
		if ( fWorldLocation.y < -25.0f )
		{	// Water
			outputColor.rgb = vec3(0.0f, 0.0f, 1.0f);
		}
		else if ( fWorldLocation.y < -15.0f )
		{	// Sand ( 89.8% red, 66.67% green and 43.92% )
			outputColor.rgb = vec3(0.898f, 0.6667f, 0.4392f);
		}
		else if ( fWorldLocation.y < 30.0f )
		{	// Grass
			outputColor.rgb = vec3(0.0f, 1.0f, 0.0f);
		}
		else
		{	// Snow
			outputColor.rgb = vec3(1.0f, 1.0f, 1.0f);
		}
		outputColor.a = 1.0f;
	
		return;
	}

	vec3 matColour = fColour.rgb;

	float alphaTransparency = RGBAColour.w;

	// If a solid color is applied
	if (useRGBAColour) 
	{
		matColour = RGBAColour.rgb;
	}

	// If a texture is applied
	else if (bHasTexture)
	{
		vec3 textColour0 = texture( texture0, fUV2.st ).rgb;		
		vec3 textColour1 = texture( texture1, fUV2.st ).rgb;	
		vec3 textColour2 = texture( texture2, fUV2.st ).rgb;	
		vec3 textColour3 = texture( texture3, fUV2.st ).rgb;
		
		matColour = (textColour0.rgb * texRatio_0_3.x) 
				  + (textColour1.rgb * texRatio_0_3.y) 
				  + (textColour2.rgb * texRatio_0_3.z) 
				  + (textColour3.rgb * texRatio_0_3.w);

		// matColour *= 10.f;
		//outputColor.rgb = matColour.rgb;
		//outputColor.a = 1.f;
		//vec4 specColour = vec4(0.1f, 0.1f, 0.1f, 1.0f);
		//
		//outputColor = calculateLightContrib( specColour.rgb, fNormal.xyz, 
	    //                                    fWorldLocation.xyz, specColour );
		//
		//return;
	}

	// If neither a texture nor a solid color is applied
	// Apply the colors coming in from the model file
	else {
		//matColour = fColour.rgb;
	}

	if (doNotLight)
	{
		// Set the output colour and exit early
		// (Don't apply the lighting to this)
		outputColor = vec4(matColour.rgb, alphaTransparency);
		return;
	}

	// initialize the lights array 
	// based on the number of lights coming in from "uniform NUMBEROFLIGHTS"
	//sLightsArray[NUMBEROFLIGHTS];

	vec4 specColour = vec4(0.1f, 0.1f, 0.1f, 1.0f);

	//matColour.r = 1.f;

	vec4 litColour = calculateLightContrib( matColour.rgb, normalize(fNormal.xyz), 
	                                        fWorldLocation.xyz, specColour );
	
	// If my blend function is (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) 
	// then it's reading whatever the 4th value of the output is:
	outputColor = vec4(litColour.rgb, alphaTransparency);

    float amountOfAmbientLight = ambientLight;

//	if (bHasTexture) {
//		matColour.rgb += (matColour.rgb * amountOfAmbientLight);
//		outputColor.rgb = matColour;
//		return;
//	}

	outputColor.rgb += (matColour.rgb * ambientLight);

	//outputColor.rgb *= 0.01f;
	//
	//outputColor.rgb += normalize(fNormal.xyz);
	return;
}

vec4 calculateLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular )
{
	vec3 norm = normalize(vertexNormal);

	vec4 finalColour = vec4( 0.0f, 0.0f, 0.0f, 1.0f );

	for ( int index = 0; index < NUMBEROFLIGHTS; index++ )
	{	

		// is light "on"
		if ( sLightsArray[index].param2.x == 0.0f )
		{	// it's off
			continue;
		}

		// Cast to an int (note with c'tor)
		int intLightType = int(sLightsArray[index].param1.x);

		// We will do the directional light here... 
		// (BEFORE the attenuation, since sunlight has no attenuation, really)
		if ( intLightType == DirectionalLight )		// = 2
		{

			vec3 lightContrib = sLightsArray[index].diffuse.rgb;

			// Get the dot product of the light and normalize
			float dotProduct = dot( -sLightsArray[index].direction.xyz,  
									   normalize(norm.xyz) );	// -1 to 1

			dotProduct = max( 0.0f, dotProduct );		// 0 to 1

			lightContrib *= dotProduct;		

			finalColour.rgb += (vertexMaterialColour.rgb * sLightsArray[index].diffuse.rgb * lightContrib); 
									 //+ (materialSpecular.rgb * lightSpecularContrib.rgb);
			// NOTE: There isn't any attenuation, like with sunlight.
			// (This is part of the reason directional lights are fast to calculate)


			return finalColour;
		}

		// Assume it's a point light 
		// intLightType = 0

		// Contribution for this light
		vec3 vLightToVertex = sLightsArray[index].position.xyz - vertexWorldPos.xyz;
		float distanceToLight = length(vLightToVertex);	
		vec3 lightVector = normalize(vLightToVertex);
		float dotProduct = dot(lightVector, vertexNormal.xyz);	 

		dotProduct = max( 0.0f, dotProduct );	

		vec3 lightDiffuseContrib = dotProduct * sLightsArray[index].diffuse.rgb;


		// Specular 
		vec3 lightSpecularContrib = vec3(0.0f);

		vec3 reflectVector = reflect( -lightVector, normalize(norm.xyz) );

		// Get eye or view vector
		// The location of the vertex in the world to your eye
		vec3 eyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);

		// To simplify, we are NOT using the light specular value, just the object�s.
		float objectSpecularPower = vertexSpecular.w; 

		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
			                   * sLightsArray[index].specular.rgb;

		// Attenuation
		float attenuation = 1.0f / 
				( sLightsArray[index].atten.x + 										
				  sLightsArray[index].atten.y * distanceToLight +						
				  sLightsArray[index].atten.z * distanceToLight*distanceToLight );  	

		// total light contribution is Diffuse + Specular
		lightDiffuseContrib *= attenuation;
		lightSpecularContrib *= attenuation;


		// But is it a spot light
		if ( intLightType == SpotLight )		// = 1
		{	
			// Yes, it's a spotlight
			// Calcualate light vector (light to vertex, in world)
			vec3 vertexToLight = vertexWorldPos.xyz - sLightsArray[index].position.xyz;

			vertexToLight = normalize(vertexToLight);

			float currentLightRayAngle
					= dot( vertexToLight.xyz, sLightsArray[index].direction.xyz );

			currentLightRayAngle = max(0.0f, currentLightRayAngle);

			//vec4 param1;	
			// x = lightType, y = inner angle, z = outer angle, w = TBD

			// Is this inside the cone? 
			float outerConeAngleCos = cos(radians(sLightsArray[index].param1.z));
			float innerConeAngleCos = cos(radians(sLightsArray[index].param1.y));

			// Is it completely outside of the spot?
			if ( currentLightRayAngle < outerConeAngleCos )
			{
				// Nope. so it's in the dark
				lightDiffuseContrib = vec3(0.0f, 0.0f, 0.0f);
				lightSpecularContrib = vec3(0.0f, 0.0f, 0.0f);
			}
			else if ( currentLightRayAngle < innerConeAngleCos )
			{
				// Angle is between the inner and outer cone
				// (this is called the penumbra of the spot light, by the way)
				// 
				// This blends the brightness from full brightness, near the inner cone
				//	to black, near the outter cone
				float penumbraRatio = (currentLightRayAngle - outerConeAngleCos) / 
									  (innerConeAngleCos - outerConeAngleCos);

				lightDiffuseContrib *= penumbraRatio;
				lightSpecularContrib *= penumbraRatio;
			}

		}// if ( intLightType == 1 )



		finalColour.rgb += (vertexMaterialColour.rgb * lightDiffuseContrib.rgb)
								  + (vertexSpecular.rgb  * lightSpecularContrib.rgb );

	}//for(intindex=0...

	finalColour.a = 1.0f;

	return finalColour;
}
