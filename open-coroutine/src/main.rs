use std::os::raw::{c_int, c_void};
// use open_coroutine::fiber::{acl_fiber, ACL_FIBER, acl_fiber_kill};
use open_coroutine::interop_sort;

// 对interop_sort函数的安全抽象
pub fn sort_from_cpp(arr: &mut [c_int; 10], n: c_int) {
    unsafe {
        interop_sort(arr as *mut i32, n);
    }
}

// unsafe extern "C" fn fiber_main(fiber: *mut ACL_FIBER, context: *mut c_void) {
//     println!("fiber_main {}", context as usize);
//     acl_fiber_kill(fiber as *mut acl_fiber);
// }

fn main() {
    let mut my_arr: [i32; 10] = [10, 42, -9, 12, 8, 25, 7, 13, 55, -1];
    println!("Before sorting...");
    println!("{:?}\n", my_arr);
    sort_from_cpp(&mut my_arr, 10);
    println!("After sorting...");
    println!("{:?}", my_arr);

    // unsafe {
    //     acl_fiber::hook_api(true);
    //     let context: *mut c_void = 123usize as *mut c_void;
    //     acl_fiber::fiber_create(Some(fiber_main), context, 128 * 1024 * 1024);
    //     acl_fiber::schedule();
    // }
}