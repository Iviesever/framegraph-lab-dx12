cbuffer Root : register(b0) {
    float4x4 view_projection;
    float4 camera;
    float4 viewport;
    float4 pass_data;
    float4 extra;
};

static const float3 floor_vertices[6] = {
    float3(-24,0,-24), float3(-24,0,24), float3(24,0,24),
    float3(-24,0,-24), float3(24,0,24), float3(24,0,-24)
};
static const float3 cube_vertices[36] = {
    float3(-.5,-.5,-.5),float3(-.5,.5,-.5),float3(.5,.5,-.5),float3(-.5,-.5,-.5),float3(.5,.5,-.5),float3(.5,-.5,-.5),
    float3(.5,-.5,.5),float3(.5,.5,.5),float3(-.5,.5,.5),float3(.5,-.5,.5),float3(-.5,.5,.5),float3(-.5,-.5,.5),
    float3(-.5,-.5,.5),float3(-.5,.5,.5),float3(-.5,.5,-.5),float3(-.5,-.5,.5),float3(-.5,.5,-.5),float3(-.5,-.5,-.5),
    float3(.5,-.5,-.5),float3(.5,.5,-.5),float3(.5,.5,.5),float3(.5,-.5,-.5),float3(.5,.5,.5),float3(.5,-.5,.5),
    float3(-.5,.5,-.5),float3(-.5,.5,.5),float3(.5,.5,.5),float3(-.5,.5,-.5),float3(.5,.5,.5),float3(.5,.5,-.5),
    float3(-.5,-.5,.5),float3(-.5,-.5,-.5),float3(.5,-.5,-.5),float3(-.5,-.5,.5),float3(.5,-.5,-.5),float3(.5,-.5,.5)
};
static const float3 cube_normals[6] = { float3(0,0,-1),float3(0,0,1),float3(-1,0,0),float3(1,0,0),float3(0,1,0),float3(0,-1,0) };

uint hash_u32(uint x) {
    x ^= x >> 16; x *= 0x7feb352d; x ^= x >> 15; x *= 0x846ca68b; x ^= x >> 16; return x;
}
void world_vertex(uint vertex_id, uint instance_id, out float3 world, out float3 normal, out float height_fraction) {
    if (instance_id == 0) { world = floor_vertices[vertex_id]; normal = float3(0,1,0); height_fraction = 0; return; }
    uint seed = asuint(camera.y);
    uint a = hash_u32(instance_id * 17 + seed);
    uint b = hash_u32(instance_id * 43 + seed * 3);
    float x = (int(a % 31) - 15) * 1.35;
    float z = (int(b % 37) - 18) * 1.15;
    float height = 1.4 + float((a >> 8) % 100) * 0.055;
    float3 local = cube_vertices[vertex_id];
    world = local * float3(.62, height, .62) + float3(x, height * .5, z);
    normal = cube_normals[vertex_id / 6]; height_fraction = local.y + .5;
}
struct DepthOutput { float4 position : SV_Position; };
DepthOutput DepthVS(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) {
    float3 world, normal; float height_fraction; world_vertex(vertex_id, instance_id, world, normal, height_fraction);
    DepthOutput output; output.position = mul(float4(world, 1), view_projection); return output;
}
struct SceneOutput {
    float4 position : SV_Position;
    float3 world : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float height_fraction : TEXCOORD2;
    nointerpolation uint instance_id : TEXCOORD3;
};
SceneOutput SceneVS(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) {
    SceneOutput output; world_vertex(vertex_id, instance_id, output.world, output.normal, output.height_fraction);
    output.position = mul(float4(output.world, 1), view_projection); output.instance_id = instance_id; return output;
}
float4 ScenePS(SceneOutput input) : SV_Target {
    float time = camera.x;
    if (input.instance_id == 0) {
        float2 cell = abs(frac(input.world.xz * .25) - .5);
        float grid_line = pow(1 - saturate(min(cell.x, cell.y) * 18), 5);
        float pulse = .65 + .35 * sin(time * 1.8 + (input.world.x + input.world.z) * .18);
        return float4(float3(.006,.018,.045) + grid_line * pulse * float3(.05,1.1,2.5), 1);
    }
    uint seed = asuint(camera.y); uint h = hash_u32(input.instance_id + seed);
    float3 neon = (h & 1) ? float3(.05,2.5,5.8) : float3(5.2,.12,2.1);
    float3 light_position = float3(sin(time) * 14, 13, cos(time) * 14);
    float diffuse = .18 + .82 * saturate(dot(normalize(input.normal), normalize(light_position - input.world)));
    float top_glow = smoothstep(.55, 1, input.height_fraction);
    float pulse = .72 + .28 * sin(time * 2.3 + input.instance_id * .37);
    return float4(neon * (.10 * diffuse + top_glow * pulse) + float3(.02,.025,.05), 1);
}
