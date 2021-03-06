use std::os::raw::c_void;
use std::ptr;
use crate::inner_context::{InnerContext, Transfer};
use crate::stack::{ProtectedFixedSizeStack, Stack};

pub type Function = fn(param: Option<*mut c_void>) -> Option<*mut c_void>;

#[derive(Debug)]
pub struct Context<'a> {
    stack: &'a Stack,
    sp: Transfer,
    //用户函数
    proc: Function,
    //调用用户函数的参数
    param: Option<*mut c_void>,
    //上下文切换栈，方便用户在N个协程中任意切换
    context_stack: Vec<&'a Context<'a>>,
}

extern "C" fn inner_context_function(mut t: Transfer) {
    unsafe {
        loop {
            let context = t.data as *mut Context;
            let param = (*context).param as Option<*mut c_void>;
            match param {
                Some(data) => { print!("inner_context_function {} => ", data as usize) }
                None => { print!("inner_context_function no param => ") }
            }
            // copy stack
            let mut context_stack: Vec<&Context> = Vec::new();
            unsafe {
                for data in (*context).context_stack.iter() {
                    context_stack.push(data);
                }
            }
            context_stack.push(&*context);
            let mut new_context = Context::init((*context).stack, (*context).proc, context_stack);
            //调用用户函数
            let func = (*context).proc;
            new_context.param = func(param);
            //调用完用户函数后，需要清理context_stack
            new_context.context_stack.pop();
            t = t.resume(&mut new_context as *mut Context as *mut c_void);
        }
    }
}

impl<'a> Context<'a> {
    pub fn new(stack: &'a Stack, proc: Function) -> Self {
        Context::init(stack, proc, Vec::new())
    }

    fn init(stack: &'a Stack, proc: Function, context_stack: Vec<&'a Context<'a>>) -> Self {
        let inner = InnerContext::new(stack, inner_context_function);
        // Allocate a Context on the stack.
        let mut sp = Transfer::new(inner, 0 as *mut c_void);
        let mut context = Context {
            stack,
            sp,
            proc,
            param: None,
            context_stack,
        };
        context.sp.data = &mut context as *mut Context as *mut c_void;
        context
    }

    pub fn set_param(&mut self, param: Option<*mut c_void>) {
        unsafe {
            let context = self.sp.data as *mut Context;
            (*context).param = param;
        }
    }

    pub fn yields(&mut self) -> Self {
        //没有参数
        self.resume(None)
    }

    pub fn resume(&mut self, param: Option<*mut c_void>) -> Self {
        self.set_param(param);
        let mut sp = self.sp.resume(self.sp.data);
        let context = sp.data as *mut Context;
        let mut context_stack: Vec<&Context> = Vec::new();
        unsafe {
            for data in (*context).context_stack.iter() {
                context_stack.push(data);
            }
        }
        Context {
            stack: self.stack,
            sp,
            proc: self.proc,
            param: None,
            context_stack,
        }
    }

    pub fn switch(&self, to: &Context) -> Self {
        let mut sp = Transfer::switch(&to.sp);
        let context = sp.data as *mut Context;
        let mut context_stack: Vec<&Context> = Vec::new();
        unsafe {
            for data in (*context).context_stack.iter() {
                context_stack.push(data);
            }
        }
        Context {
            stack: self.stack,
            sp,
            proc: self.proc,
            param: None,
            context_stack,
        }
    }

    pub fn get_result(&self) -> Option<*mut c_void> {
        let context = self.sp.data as *mut Context;
        unsafe { (*context).param }
    }
}

#[cfg(test)]
mod tests {
    use std::os::raw::c_void;
    use crate::context::Context;
    use crate::stack::ProtectedFixedSizeStack;

    fn user_function(param: Option<*mut c_void>) -> Option<*mut c_void> {
        match param {
            Some(param) => {
                print!("user_function {} => ", param as usize);
            }
            None => {
                print!("user_function no param => ");
            }
        }
        param
    }

    #[test]
    fn test() {
        println!("context test started !");
        let stack = ProtectedFixedSizeStack::new(2048)
            .expect("allocate stack failed !");
        let mut c = Context::new(&stack, user_function);
        for i in 0..10 {
            print!("Resuming {} => ", i);
            c = c.resume(Some(i as *mut c_void));
            match c.get_result() {
                Some(result) => { println!("Got {}", result as usize) }
                None => { println!("No result") }
            }
        }
        println!("context test finished!");
    }
}