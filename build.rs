use std::env;
use std::fs;
use std::path::PathBuf;
use std::process::Command;

fn main() {
    println!("cargo:rerun-if-changed=src/wrapper.cpp");
    println!("cargo:rerun-if-changed=include/wrapper.hpp");
    println!("cargo:rerun-if-changed=CMakeLists.txt");
    println!("cargo:rerun-if-changed=shaders/*.wgsl");

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let shaders_dir = manifest_dir.join("shaders");
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    let dxc_path = find_dxc();

    let shaders = ["rect", "rounded_rect", "text"];
    let mut rust_code =
        String::from("// Auto-generated SPIR-V shader data\n#[allow(dead_code)]\n\n");

    for shader_name in &shaders {
        let wgsl_path = shaders_dir.join(format!("{}.wgsl", shader_name));
        let spv_path = out_dir.join(format!("{}.spv", shader_name));

        if wgsl_path.exists() {
            if compile_wgsl_to_spirv(&dxc_path, &wgsl_path, &spv_path) {
                if let Ok(spv_data) = fs::read(&spv_path) {
                    let var_name = format!("{}_spv", shader_name.replace("-", "_"));
                    rust_code.push_str(&format!(
                        "pub static {}: [u8; {}] = [\n",
                        var_name,
                        spv_data.len()
                    ));

                    for (i, chunk) in spv_data.chunks(16).enumerate() {
                        rust_code.push_str("    ");
                        for (j, &byte) in chunk.iter().enumerate() {
                            if j > 0 {
                                rust_code.push_str(", ");
                            }
                            rust_code.push_str(&format!("0x{:02x}", byte));
                        }
                        if i < (spv_data.len() + 15) / 16 - 1 {
                            rust_code.push_str(",");
                        }
                        rust_code.push_str("\n");
                    }
                    rust_code.push_str("];\n\n");

                    println!(
                        "Compiled {} -> {} ({} bytes)",
                        wgsl_path.display(),
                        spv_path.display(),
                        spv_data.len()
                    );
                }
                println!("cargo:rerun-if-changed={}", wgsl_path.display());
            }
        }
    }

    fs::write(out_dir.join("shaders.rs"), &rust_code).expect("Failed to write shaders.rs");

    let dst = cmake::Config::new(".")
        .build_target("molten_wrapper")
        .build();

    println!("cargo:rustc-link-search=native={}/build", dst.display());
    println!(
        "cargo:rustc-link-search=native={}/build/_deps/fetch_vk_bootstrap-build",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=molten_wrapper");
    println!("cargo:rustc-link-lib=static=moltenUI");
    println!("cargo:rustc-link-lib=static=vk-bootstrap");

    if cfg!(target_os = "macos") {
        let home = env::var("HOME").unwrap();
        println!("cargo:rustc-link-search=native=/opt/homebrew/lib");
        println!("cargo:rustc-link-search=native=/usr/local/lib");
        println!(
            "cargo:rustc-link-search=native={}/VulkanSDK/1.4.341.0/macOS/lib",
            home
        );

        println!("cargo:rustc-link-lib=dylib=glfw");
        println!("cargo:rustc-link-lib=dylib=freetype");
        println!("cargo:rustc-link-lib=dylib=vulkan");
        println!("cargo:rustc-link-lib=framework=Cocoa");
        println!("cargo:rustc-link-lib=framework=IOKit");
        println!("cargo:rustc-link-lib=framework=CoreVideo");
        println!("cargo:rustc-link-lib=framework=QuartzCore");
        println!("cargo:rustc-link-lib=framework=Metal");
        println!("cargo:rustc-link-lib=c++");
    } else if cfg!(target_os = "linux") {
        println!("cargo:rustc-link-lib=vulkan");
        println!("cargo:rustc-link-lib=glfw");
        println!("cargo:rustc-link-lib=freetype");
        println!("cargo:rustc-link-lib=stdc++");
    }

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let wrapper_header = manifest_dir.join("include/wrapper.hpp");

    let bindings = bindgen::Builder::default()
        .header(wrapper_header.to_str().unwrap())
        .clang_arg("-xc++")
        .clang_arg("-std=c++17")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .allowlist_function("molten_.*")
        .allowlist_type("MoltenColor")
        .allowlist_type("MoltenVec2")
        .allowlist_type("MoltenThemeColors")
        .disable_header_comment()
        .layout_tests(false)
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}

fn find_dxc() -> PathBuf {
    let home = env::var("HOME").unwrap();
    let sdk_path = PathBuf::from(&home).join("VulkanSDK/1.4.341.0/macOS/bin/dxc");
    if sdk_path.exists() {
        return sdk_path;
    }

    if let Ok(path) = Command::new("which").arg("dxc").output() {
        if path.status.success() {
            let path_str = String::from_utf8_lossy(&path.stdout);
            return PathBuf::from(path_str.trim());
        }
    }

    PathBuf::from("/usr/local/bin/dxc")
}

fn compile_wgsl_to_spirv(dxc_path: &PathBuf, wgsl_path: &PathBuf, spv_path: &PathBuf) -> bool {
    let result = Command::new(dxc_path)
        .args([
            wgsl_path.to_str().unwrap(),
            "-spirv",
            "-fentry-point=vs_main",
            "-fentry-point=fs_main",
            "-o",
            spv_path.to_str().unwrap(),
        ])
        .output();

    match result {
        Ok(output) => {
            if !output.status.success() {
                let stderr = String::from_utf8_lossy(&output.stderr);
                eprintln!(
                    "Warning: Failed to compile {}: {}",
                    wgsl_path.display(),
                    stderr
                );
                false
            } else {
                true
            }
        }
        Err(e) => {
            eprintln!("Warning: Could not run dxc: {}", e);
            false
        }
    }
}
