struct i2v
{
    float3 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
};

struct v2f
{
    float4 clipPos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float4 viewPos : TEXCOORD3;
};

cbuffer Camera : register(b0)
{
    row_major float4x4 ProjectionMatrix;
    row_major float4x4 ModelViewMatrix;
    row_major float4x4 NormalMatrix;
};

v2f main(in i2v vertex)
{
    v2f o;
    
    o.viewPos = mul(float4(vertex.position, 1.0f), ModelViewMatrix);
    o.clipPos = mul(o.viewPos, ProjectionMatrix);
    o.uv = vertex.texCoord;
    o.color = vertex.color;
    o.normal = normalize(mul(vertex.normal, (float3x3) NormalMatrix));

    return o;
}