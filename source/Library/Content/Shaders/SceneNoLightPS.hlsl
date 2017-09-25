Texture2DArray diffuseMapsArray : register(t0);
SamplerState texFilter;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 texCoords : TEXCOORD0;
    //float depth : TEXTURE0;
};

[earlydepthstencil]
float4 main(PixelInputType input) : SV_TARGET
{
    float3 base = diffuseMapsArray.Sample(texFilter, input.texCoords).rgb;

    return float4(base, 1.0f);

    //float4 color = diffuseMapsArray.Sample(texFilter, input.texCoords);
    // render depth values
    //color = float4(input.depth, input.depth, input.depth, 1.0f);
    //return color;
}