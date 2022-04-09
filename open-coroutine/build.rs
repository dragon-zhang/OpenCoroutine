extern crate cc;
extern crate bindgen;

use std::env;
use std::path::PathBuf;

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
    cc::Build::new()
        .cpp(true)
        .warnings(true)
        .flag("-Wall")
        .flag("-std=c++14")
        .flag("-c")
        .file("cpp_src/sorting.cpp")
        .compile("sorting");
    // 也可以使用Command::new("g++")来组装命令，但是cc更方便。
}