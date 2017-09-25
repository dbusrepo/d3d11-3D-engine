cbuffer MatrixBuffer
{
    float4x4 gWorldViewProj;
};

struct VertexIn
{
    float4 PosL : POSITION;
};

struct PixelInputType
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

PixelInputType main(VertexIn vin)
{
    PixelInputType vout;
	
	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    vout.PosH = mul(vin.PosL, gWorldViewProj).xyww;
	
	// Use local vertex position as cubemap lookup vector.
    vout.PosL = vin.PosL.xyz;
	
    return vout;
}