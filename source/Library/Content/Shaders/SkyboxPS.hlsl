TextureCube gCubeMap;
SamplerState sampleType;

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

float4 main(VertexOut pin) : SV_TARGET
{
    return gCubeMap.Sample(sampleType, pin.PosL);
}