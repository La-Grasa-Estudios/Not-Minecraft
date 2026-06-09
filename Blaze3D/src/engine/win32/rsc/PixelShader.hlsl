struct v2f
{
    float4 clipPos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float4 viewPos : TEXCOORD3;
};

struct Light
{
    int Type;
    float3 Direction;
    bool Enabled;
    float3 Diffuse;
    float4 Ambient;
    float4 Specular;
};

Texture2D Tex : register(t0);
SamplerState Sampler : register(s0);
cbuffer DrawParams : register(b1)
{
    bool LightingEnabled;
    Light Lights[8];
    bool FogEnabled;
    float FogStart;
    float FogEnd;
    float3 FogColor;
};

float4 main(in v2f i) : SV_TARGET
{
    float4 col = Tex.Sample(Sampler, i.uv);
    clip(col.a - 0.35f);
    
    col *= i.color;

    if (LightingEnabled)
    {
        float3 N = normalize(i.normal);
        
        float3 lightAccum = Lights[0].Ambient.rgb + Lights[1].Ambient.rgb;

        float3 L0 = normalize(Lights[0].Direction.xyz);
        lightAccum += saturate(dot(N, L0)) * Lights[0].Diffuse.rgb;

        float3 L1 = normalize(Lights[1].Direction.xyz);
        lightAccum += saturate(dot(N, L1)) * Lights[1].Diffuse.rgb;

        col.rgb *= saturate(lightAccum);
    }
    
    if (FogEnabled)
    {
        float dist = length(i.viewPos.xyz);

        float fogFactor = (FogEnd - dist) / (FogEnd - FogStart);
        fogFactor = saturate(fogFactor);
        
        col.rgb = lerp(FogColor, col.rgb, fogFactor);
    }
    
    return col;
}