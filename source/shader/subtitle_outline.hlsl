struct VertexShaderInput
{
    float4 Position : POSITION0;	// model xyzw
    float2 uv0: TEXCOORD0;			// texture 0 uv
};

struct VertexShaderOutput
{
    float4 Position : POSITION0;	// model xyzw
    float2 uv0: TEXCOORD0;			// texture 0 uv
};

VertexShaderOutput vs_main(VertexShaderInput input)
{
    VertexShaderOutput output;
	output.Position = input.Position;
	output.uv0 = input.uv0;	
    return output;
}

sampler2D YTexture : register(s0); 
float4 color : register(c0);
float4 outline : register(c1);

struct PixelShaderInput
{
	float2 uv0: TEXCOORD0;			// texture 0 uv
	float4 color: COLOR;
};

float4 ps_osd(PixelShaderInput input): COLOR0 {
    float4 px = tex2D(YTexture, input.uv0) * color;
    const float d = 0.25;

    float cnt = 0;
    float a = 0;
    a += tex2D(YTexture,  input.uv0 + float2(0.002, 0)).a;
    a += tex2D(YTexture,  input.uv0 + float2(-0.002, 0)).a;
    a += tex2D(YTexture,  input.uv0 + float2(0, 0.002)).a;
    a += tex2D(YTexture,  input.uv0 + float2(0, -0.003)).a;
    a += tex2D(YTexture,  input.uv0 + 0.001).a;
    cnt += 0.1;
    a /= cnt;

    float4 s = a;
	s.rgb = outline.rgb;
    float4 r = float4(px.rgb*px.a + s.rgb*(1-px.a), max(px.a, a));
    return px + r;
}