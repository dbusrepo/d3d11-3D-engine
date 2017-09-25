cbuffer ConstantsBuffer
{
    float4x4 wvp;
    float4 color;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
};


float4 main(PixelInputType input) : SV_TARGET
{
    return color; //float4(0.0f, 1.0f, 0.0f, 1.0f);
}