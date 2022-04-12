extern crate cc;
extern crate bindgen;

use std::{env, fs, io};
use std::path::{Path, PathBuf};
use cc::Build;

fn visit_sources(dir: &Path, builder: &mut Build) -> io::Result<()> {
    if dir.is_dir() {
        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let path = entry.path();
            if path.is_dir() {
                visit_sources(&path, builder)?;
            } else {
                let path = path.to_str().unwrap();
                //头文件
                if path.ends_with(".h") || path.ends_with(".hpp")
                    //源文件
                    || path.ends_with(".c") || path.ends_with(".cpp") {
                    println!("{}", path);
                    builder.file(path);
                }
            }
        }
    }
    Ok(())
}

fn visit_asm_sources(dir: &Path, builder: &mut Build, suffix: &String) -> io::Result<()> {
    if dir.is_dir() {
        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let path = entry.path();
            if path.is_dir() {
                visit_asm_sources(&path, builder, suffix)?;
            } else {
                let path = path.to_str().unwrap();
                if path.ends_with(suffix) {
                    println!("{}", path);
                    builder.file(path);
                }
            }
        }
    }
    Ok(())
}

fn build_asm_suffix() -> String {
    // for the stable build asm lib
    let target: String = env::var("TARGET").unwrap();
    let is_win_gnu = target.ends_with("windows-gnu");
    let is_win_msvc = target.ends_with("windows-msvc");
    let is_win = is_win_gnu || is_win_msvc;

    let arch = match target.split('-').next().unwrap() {
        "arm" | "armv7" | "armv7s" => "arm",
        "arm64" | "aarch64" => "arm64",
        "x86" | "i386" | "i486" | "i586" | "i686" => "i386",
        "mips" | "mipsel" => "mips32",
        "powerpc" => "ppc32",
        "powerpc64" => "ppc64",
        "x86_64" => "x86_64",
        _ => {
            panic!("Unsupported architecture: {}", target);
        }
    };

    let abi = match arch {
        "arm" | "arm64" => "aapcs",
        "mips32" => "o32",
        _ => {
            if is_win {
                "ms"
            } else {
                "sysv"
            }
        }
    };

    let format = if is_win {
        "pe"
    } else if target.contains("apple") {
        "macho"
    } else if target.ends_with("aix") {
        "xcoff"
    } else {
        "elf"
    };

    let (asm, ext) = if is_win_msvc {
        if arch == "arm" {
            ("armasm", "asm")
        } else {
            ("masm", "asm")
        }
    } else if is_win_gnu {
        ("gas", "asm")
    } else {
        ("gas", "S")
    };
    let file_name: [&str; 10] = ["_", arch, "_", abi, "_", format, "_", asm, ".", ext];
    file_name.concat()
}

fn main() {
    // Tell cargo to invalidate the built crate whenever the wrapper changes
    println!("cargo:rerun-if-changed=lib_fiber/c/include/libfiber.h");
    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("lib_fiber/c/include/libfiber.h")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings")
        .write_to_file(out_path.join("libfiber.rs"))
        .expect("Couldn't write bindings!");

    let suffix = build_asm_suffix();
    let mut asm = cc::Build::new();
    visit_asm_sources(Path::new("lib_fiber/c/src"),
                      asm.warnings(false), &suffix);
    asm.compile("libasm.a");

    // 相当于执行
    // 1. `g++ -Wall -std=c++14 -c sorting.cpp`，使用g++编译sorting.cpp文件
    // 2. `ar rc libsorting.a sorting.o`，通过ar制作一份静态库libsorting.a
    let mut builder = cc::Build::new();
    // 也可以使用Command::new("g++")来组装命令，但是cc更方便。

    visit_sources(Path::new("lib_fiber"),
                  builder.cpp(false)
                      .warnings(false)
                      .file(out_path.join("libasm.a")),
    ).unwrap();
    builder.compile("fiber");
}