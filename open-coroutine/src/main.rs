use std::os::raw::c_int;
use open_coroutine::interop_sort;

// 对interop_sort函数的安全抽象
pub fn sort_from_cpp(arr: &mut [c_int; 10], n: c_int) {
    unsafe {
        interop_sort(arr as *mut i32, n);
    }
}

fn main() {
    let mut my_arr: [i32; 10] = [10, 42, -9, 12, 8, 25, 7, 13, 55, -1];
    println!("Before sorting...");
    println!("{:?}\n", my_arr);
    sort_from_cpp(&mut my_arr, 10);
    println!("After sorting...");
    println!("{:?}", my_arr);
}