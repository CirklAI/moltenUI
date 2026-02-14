@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> @builtin(position) vec4<f32> {
    var pos = array<vec2<f32>, 6>(
        vec2<f32>(0.0, 0.0),
        vec2<f32>(1.0, 0.0),
        vec2<f32>(0.0, 1.0),
        vec2<f32>(1.0, 0.0),
        vec2<f32>(1.0, 1.0),
        vec2<f32>(0.0, 1.0)
    );
    return vec4<f32>(pos[vertex_index] * 2.0 - 1.0, 0.0, 1.0);
}

struct Uniforms {
    pos: vec2<f32>,
    size: vec2<f32>,
    color: vec4<f32>,
    radius: f32,
    border_width: f32,
    border_color: vec4<f32>,
    resolution: vec2<f32>,
}

@group(0) @binding(0)
var<uniform> ubo: Uniforms;

fn sd_rounded_box(p: vec2<f32>, b: vec2<f32>, r: f32) -> f32 {
    var q = abs(p) - b + vec2<f32>(r);
    return length(max(q, vec2<f32>(0.0))) + min(max(q.x, q.y), 0.0) - r;
}

@fragment
fn fs_main(@builtin(position) frag_coord: vec4<f32>) -> @location(0) vec4<f32> {
    let uv = frag_coord.xy;
    let center = ubo.pos + ubo.size * 0.5;
    let half_size = ubo.size * 0.5;
    let p = uv - center;
    let d = sd_rounded_box(p, half_size, ubo.radius);
    let alpha = 1.0 - smoothstep(0.0, 1.0, d);

    var col = ubo.color;

    if ubo.border_width > 0.0 {
        let d_border = sd_rounded_box(p, half_size - vec2<f32>(ubo.border_width), ubo.radius - ubo.border_width);
        // border region: d < 0 (inside outer) AND d_border >= 0 (outside inner)
        if (d < 0.0 && d_border >= 0.0) {
            col = vec4<f32>(0.1, 0.1, 0.1, 1.0);
        }
    }
    
    return vec4<f32>(col.rgb, col.a * alpha);
}
