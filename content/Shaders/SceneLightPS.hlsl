cbuffer glData : register(b0)
{
    float3 worldCamPos;
    float2 clusterMapSize;
    float3 padding;
}

struct Light
{
    float3 Position;
    float Intensity;
    float3 Color;
    uint index; // level light index used to sample lightMapsArray
    float radius;
};

struct LeafData
{
    float3 Ambient;
    uint ActiveLightsMask;
    Light Lights[16];
};

Texture2DArray lightMapsArray : register(t0);
Texture2DArray<uint> clustersMapsArray : register(t1);
Texture2DArray diffuseMapsArray : register(t2);
Texture2DArray normalMapsArray : register(t3);
StructuredBuffer<LeafData> leafDataBuffer : register(t4);
SamplerState texFilter;
SamplerState lmFilter;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 texCoords : TEXCOORD0;
    centroid float2 lmCoords : TEXCOORD1;
    float3 normal : Normal;
    float3 tangent : Tangent;
    float3 bitangent : Bitangent;
    nointerpolation uint leafIndex : TEXCOORD2;
    //float depth : TEXTURE0;
};

[earlydepthstencil]
float4 main(PixelInputType input) : SV_TARGET
{
    float3 base = diffuseMapsArray.Sample(texFilter, input.texCoords).rgb;
    float3 bump = normalMapsArray.Sample(texFilter, input.texCoords).xyz;

    // Uncompress each component from [0,1] to [-1,1].
    bump = normalize(2.f * bump - 1.0f);

    // Compute world-space normal
    float3x3 tangentToWorldSpace = float3x3(input.tangent,
                                             input.bitangent,
                                             input.normal);
    
    //float3 normal = normalize(input.tangent * bump.x + input.bitangent * bump.y + input.normal * bump.z);
    float3 normal = mul(bump, tangentToWorldSpace);
    
    float3 view_vec = normalize(worldCamPos - input.worldPosition);

    uint leafIndex = input.leafIndex;

    // Fetch clustered light mask
    int4 coord = int4(input.lmCoords * clusterMapSize, leafIndex, 0);
    uint light_mask = clustersMapsArray.Load(coord);
    
    light_mask &= leafDataBuffer[leafIndex].ActiveLightsMask;
    
    //float3 total_light = leafDataBuffer[leafIndex].Ambient * base;
    float3 total_light = leafDataBuffer[leafIndex].Ambient * base; //base;

    //[loop]
    while (light_mask)
    {
        // Extract a light from the mask and disable that bit
        uint i = firstbitlow(light_mask);
        light_mask &= ~(1 << i);

        const Light light = leafDataBuffer[leafIndex].Lights[i];

        float shadow = (float)lightMapsArray.SampleLevel(lmFilter, float3(input.lmCoords, light.index), 0);
        shadow *= shadow; // Poor man's gamma. Stored as sqrt() of value. This gives much smoother shadows than storing linear values.

        // Lighting vectors
        float3 lVec = light.Position - input.worldPosition;
        float3 light_vec = normalize(lVec);
        float3 half_vec = normalize(light_vec + view_vec);

        // Lighting
        //float atten = (1.0f + 0.000001f * (dot(lVec, lVec)));
        float lenSqr = dot(lVec, lVec);
        float atten = (1.0f + 0.000001f * lenSqr); //(1.0f / (0.001f*dot(lVec, lVec)));
        atten *= shadow * light.Intensity;

        float diffuse = saturate(dot(light_vec, normal));
        float specular = 0.1f * pow(saturate(dot(half_vec, normal)), 50.0f);
        specular *= shadow;
        //float diffuse = 1.0f;
        //float specular = 0.0;
    
       // base.x = 1.f;
        //base.y = 1.f;
       // base.z = 1.f;
        total_light += atten * (diffuse * base + specular) * light.Color.rgb;
       // total_light += atten; //* (diffuse * base + specular);
       // break;
    }

    // Tonemapping
    //total_light = 1.0f - exp2(-leafDataBuffer[leafIndex].Exposure * total_light);
    total_light = 1.0f - exp2(-5 * total_light);

    return float4(total_light, 1.0f); //0.5f*(normal+1.f)

    //float4 color = diffuseMapsArray.Sample(texFilter, input.texCoords);
    // render depth values
    //color = float4(input.depth, input.depth, input.depth, 1.0f);
    //return color;
}