extern crate cc;
extern crate bindgen;

use std::{env, fs, io};
use std::path::{Path, PathBuf};
use cc::Build;

fn visit_dirs(dir: &Path, builder: &mut Build) -> io::Result<()> {
    if dir.is_dir() {
        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let path = entry.path();
            if path.is_dir() {
                visit_dirs(&path, builder)?;
            } else {
                let path = path.to_str().unwrap();
                //头文件
                if path.ends_with(".h") || path.ends_with(".hpp")
                    //源文件
                    || path.ends_with(".c") || path.ends_with(".cpp")
                    //汇编源文件
                    //|| path.ends_with(".S") || path.ends_with(".asm")
                {
                    println!("{}", path);
                    builder.file(path);
                }
            }
        }
    }
    Ok(())
}

fn main() {
    // Tell cargo to invalidate the built crate whenever the wrapper changes
    println!("cargo:rerun-if-changed=cpp_src/sorting.h");
    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("cpp_src/sorting.h")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");
    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("sorting.rs"))
        .expect("Couldn't write bindings!");

    // 相当于执行
    // 1. `g++ -Wall -std=c++14 -c sorting.cpp`，使用g++编译sorting.cpp文件
    // 2. `ar rc libsorting.a sorting.o`，通过ar制作一份静态库libsorting.a
    let mut builder = cc::Build::new();
    // 也可以使用Command::new("g++")来组装命令，但是cc更方便。

    visit_dirs(Path::new("lib_fiber"),
               builder.cpp(true)
                   .warnings(true)
                   .flag("-Wall")
                   .flag("-std=c++14")
                   .flag("-c")
                   .file("cpp_src/sorting.cpp"),
    ).unwrap();
    builder.compile("sorting");
}