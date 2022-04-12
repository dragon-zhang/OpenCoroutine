use std::{fs, io};
use std::path::Path;

fn visit_dirs(dir: &Path) -> io::Result<()> {
    if dir.is_dir() {
        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let path = entry.path();
            if path.is_dir() {
                visit_dirs(&path)?;
            } else {
                let path = path.to_str().unwrap();
                //头文件
                if path.ends_with(".h") || path.ends_with(".hpp")
                    //源文件
                    || path.ends_with(".c") || path.ends_with(".cpp")
                    //汇编源文件
                    || path.ends_with(".S") || path.ends_with(".asm") {
                    println!("{}", path);
                }
            }
        }
    }
    Ok(())
}

fn main() {
    visit_dirs(Path::new("acl")).unwrap();
}