cbuffer Root : register(b0) {
    float4x4 unused_matrix;
    float4 camera;
    float4 viewport;
    float4 pass_data;
    float4 extra;
};
Texture2D source0 : register(t0);
Texture2D source1 : register(t1);
SamplerState linear_clamp : register(s0);
struct FullOutput { float4 position : SV_Position; float2 uv : TEXCOORD0; };
FullOutput FullscreenVS(uint id : SV_VertexID) {
    float2 position = id == 0 ? float2(-1,-1) : id == 1 ? float2(-1,3) : float2(3,-1);
    FullOutput output; output.position = float4(position, 0, 1); output.uv = position * float2(.5,-.5) + .5; return output;
}
float4 BloomExtractPS(FullOutput input) : SV_Target {
    float3 color = source0.Sample(linear_clamp, input.uv).rgb;
    float brightness = max(color.r, max(color.g, color.b));
    return float4(color * saturate((brightness - 1) / max(brightness, .0001)), 1);
}
float4 BloomBlurPS(FullOutput input) : SV_Target {
    float2 direction = pass_data.yz * viewport.zw;
    float3 color = source0.Sample(linear_clamp, input.uv).rgb * .227027;
    color += source0.Sample(linear_clamp, input.uv + direction).rgb * .1945946;
    color += source0.Sample(linear_clamp, input.uv - direction).rgb * .1945946;
    color += source0.Sample(linear_clamp, input.uv + direction * 2).rgb * .1216216;
    color += source0.Sample(linear_clamp, input.uv - direction * 2).rgb * .1216216;
    color += source0.Sample(linear_clamp, input.uv + direction * 3).rgb * .054054;
    color += source0.Sample(linear_clamp, input.uv - direction * 3).rgb * .054054;
    color += source0.Sample(linear_clamp, input.uv + direction * 4).rgb * .016216;
    color += source0.Sample(linear_clamp, input.uv - direction * 4).rgb * .016216;
    return float4(color, 1);
}
float3 tone_map(float3 color) {
    color = saturate((color * (2.51 * color + .03)) / (color * (2.43 * color + .59) + .14));
    return pow(color, 1.0 / 2.2);
}
float4 ToneMapPS(FullOutput input) : SV_Target {
    float3 hdr = source0.Sample(linear_clamp, input.uv).rgb;
    float3 bloom = source1.Sample(linear_clamp, input.uv).rgb;
    float3 color = pass_data.x < .5 ? tone_map(hdr + bloom * .82) : pass_data.x < 1.5 ? tone_map(hdr) : tone_map(bloom * 2.0);
    return float4(color, 1);
}
