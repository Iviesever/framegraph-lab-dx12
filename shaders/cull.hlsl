cbuffer Root : register(b0) {
    float4x4 view_projection;
    float4 camera;
    float4 viewport;
    float4 pass_data;
    float4 extra;
};
RWByteAddressBuffer visible_instances : register(u0);
RWByteAddressBuffer indirect_args : register(u1);
static const uint pillar_count = 160;
uint hash_u32(uint x) {
    x ^= x >> 16; x *= 0x7feb352d; x ^= x >> 15; x *= 0x846ca68b; x ^= x >> 16; return x;
}
float3 center_for(uint instance_id) {
    uint seed = asuint(camera.y);
    uint a = hash_u32(instance_id * 17 + seed);
    uint b = hash_u32(instance_id * 43 + seed * 3);
    float x = (int(a % 31) - 15) * 1.35;
    float z = (int(b % 37) - 18) * 1.15;
    float height = 1.4 + float((a >> 8) % 100) * .055;
    return float3(x, height * .5, z);
}
bool is_visible(uint instance_id) {
    float4 clip = mul(float4(center_for(instance_id), 1), view_projection);
    float margin = .75;
    return clip.w > 0 && abs(clip.x) <= clip.w + margin && abs(clip.y) <= clip.w + margin
        && clip.z >= -margin && clip.z <= clip.w + margin;
}
[numthreads(1, 1, 1)] void InitCullCS() {
    visible_instances.Store(0, 0);
    indirect_args.Store(0, 36);
    indirect_args.Store(4, 0);
    indirect_args.Store(8, 0);
    indirect_args.Store(12, 0);
}
[numthreads(256, 1, 1)] void FrustumCullCS(uint3 id : SV_DispatchThreadID) {
    uint index = id.x;
    if (index >= pillar_count) return;
    uint original = index + 1;
    if (index == pillar_count - 1) {
        uint total = 0;
        for (uint candidate = 1; candidate <= pillar_count; ++candidate) total += is_visible(candidate) ? 1 : 0;
        indirect_args.Store(4, total);
    }
    if (!is_visible(original)) return;
    uint slot = 0;
    for (uint previous = 1; previous < original; ++previous) slot += is_visible(previous) ? 1 : 0;
    visible_instances.Store(slot * 4, original);
}
