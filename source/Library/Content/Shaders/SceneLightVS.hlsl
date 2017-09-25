cbuffer glData : register(b0)
{
    float4x4 wvp;
};

struct VertexInputType
{
    float3 position : POSITION;
    float3 texCoords : TEXCOORD0;
    float2 lmCoords : TEXCOORD1;
    float3 normal : NORMAL0;
    float4 tangent : TANGENT0;
    uint leafIndex : TEXCOORD2;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 texCoords : TEXCOORD0;
    float2 lmCoords : TEXCOORD1;
    float3 normal : Normal;
    float3 tangent : Tangent;
    float3 bitangent : Bitangent;
    uint leafIndex : TEXCOORD2;
    //float depth : TEXTURE0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    output.position = mul(float4(input.position, 1.0), wvp);
    output.worldPosition = input.position;
    output.texCoords = input.texCoords;
    output.lmCoords = input.lmCoords;
    output.normal = input.normal;
    output.tangent = (float3)input.tangent;
    output.bitangent = input.tangent.w * cross(input.normal, (float3)input.tangent);
    output.leafIndex = input.leafIndex;

    //float vz = output.position.w; // view space z
    // Note: output.position.z linear in [0, far]
    //output.depth = output.position.z / 3000.0f; //  or output.depth = (vz - 1) / (3000 - 1); 

    return output;
}