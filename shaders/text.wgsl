struct Constants {
    pos: vec2<f32>,
    size: vec2<f32>,
    color: vec4<f32>,
    uv_rect: vec4<f32>,
    resolution: vec2<f32>,
}

@group(0) @binding(0) var<uniform> pc: Constants;
@group(0) @binding(1) var font_atlas: texture_2d<f32>;
@group(0) @binding(2) var font_sampler: sampler;

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) uv: vec2<f32>,
    @location(1) color: vec4<f32>,
}

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> VertexOutput {
    var positions = array<vec2<f32>, 6>(
        vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 0.0), vec2<f32>(1.0, 1.0),
        vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 1.0), vec2<f32>(0.0, 1.0)
    );
    
    let p = positions[vertex_index];
    let pixel_pos = pc.pos + (p * pc.size);
    let ndc = (pixel_pos / pc.resolution) * 2.0 - 1.0;
    
    var out: VertexOutput;
    out.position = vec4<f32>(ndc.x, -ndc.y, 0.0, 1.0);
    out.uv = pc.uv_rect.xy + (p * pc.uv_rect.zw);
    out.color = pc.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    let alpha = textureSample(font_atlas, font_sampler, in.uv).r;
    if (alpha < 0.01) {
        discard;
    }
    return vec4<f32>(in.color.rgb, in.color.a * alpha);
}
