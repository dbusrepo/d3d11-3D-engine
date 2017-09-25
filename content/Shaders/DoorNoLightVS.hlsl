cbuffer glData : register(b0)
{
    float4x4 wvp;
};

struct VertexInputType
{
    float3 position : POSITION;
    float3 texCoords : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 texCoords : TEXCOORD0;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    output.position = mul(float4(input.position, 1.0), wvp);
    output.texCoords = input.texCoords;

    //float vz = output.position.w; // view space z
    // Note: output.position.z linear in [0, far]
    //output.depth = output.position.z / 3000.0f; //  or output.depth = (vz - 1) / (3000 - 1); 

    return output;
}