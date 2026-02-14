#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
mod ffi {
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

use ffi::*;
use std::ffi::{CStr, CString};
use std::ptr;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Color {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}

impl Color {
    pub const fn new(r: f32, g: f32, b: f32, a: f32) -> Self {
        Self { r, g, b, a }
    }

    pub const fn rgb(r: f32, g: f32, b: f32) -> Self {
        Self::new(r, g, b, 1.0)
    }

    fn to_molten_color(&self) -> MoltenColor {
        MoltenColor {
            r: self.r,
            g: self.g,
            b: self.b,
            a: self.a,
        }
    }

    fn from_molten_color(c: MoltenColor) -> Self {
        Self {
            r: c.r,
            g: c.g,
            b: c.b,
            a: c.a,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Vec2 {
    pub x: f32,
    pub y: f32,
}

impl Vec2 {
    pub const fn new(x: f32, y: f32) -> Self {
        Self { x, y }
    }

    fn to_molten_vec2(&self) -> MoltenVec2 {
        MoltenVec2 {
            x: self.x,
            y: self.y,
        }
    }
}

impl From<(f32, f32)> for Vec2 {
    fn from((x, y): (f32, f32)) -> Self {
        Self::new(x, y)
    }
}

#[derive(Debug, Clone)]
pub struct ThemeColors {
    pub background: Color,
    pub panel: Color,
    pub border: Color,
    pub text: Color,
    pub text_muted: Color,
    pub accent: Color,
    pub success: Color,
    pub error: Color,
    pub warning: Color,
}

impl ThemeColors {
    pub fn get_current() -> Self {
        unsafe {
            let mut colors = MoltenThemeColors {
                background: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                panel: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                border: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                text: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                text_muted: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                accent: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                success: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                error: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
                warning: MoltenColor {
                    r: 0.0,
                    g: 0.0,
                    b: 0.0,
                    a: 0.0,
                },
            };
            molten_theme_get_colors(&mut colors);

            Self {
                background: Color::from_molten_color(colors.background),
                panel: Color::from_molten_color(colors.panel),
                border: Color::from_molten_color(colors.border),
                text: Color::from_molten_color(colors.text),
                text_muted: Color::from_molten_color(colors.text_muted),
                accent: Color::from_molten_color(colors.accent),
                success: Color::from_molten_color(colors.success),
                error: Color::from_molten_color(colors.error),
                warning: Color::from_molten_color(colors.warning),
            }
        }
    }

    pub fn set_purple_theme() {
        unsafe {
            molten_theme_set_purple();
        }
    }
}

pub struct MoltenStr {
    inner: MoltenString,
}

impl MoltenStr {
    pub fn new(s: &str) -> Self {
        let c_str = CString::new(s).unwrap();
        unsafe {
            Self {
                inner: molten_string_create(c_str.as_ptr()),
            }
        }
    }

    pub fn as_ptr(&self) -> MoltenString {
        self.inner
    }

    pub fn get(&self) -> &str {
        unsafe {
            let c_str = molten_string_get(self.inner);
            CStr::from_ptr(c_str).to_str().unwrap_or("")
        }
    }
}

impl Drop for MoltenStr {
    fn drop(&mut self) {
        unsafe {
            molten_string_destroy(self.inner);
        }
    }
}

pub struct App {
    inner: MoltenApp,
    owned: bool,
}

impl App {
    pub fn new(title: &str, width: i32, height: i32) -> Result<Self, String> {
        let c_title = CString::new(title).map_err(|e| e.to_string())?;

        unsafe {
            let app = molten_app_create(c_title.as_ptr(), width, height);
            if app.is_null() {
                Err("Failed to create Molten app".to_string())
            } else {
                Ok(Self {
                    inner: app,
                    owned: true,
                })
            }
        }
    }

    pub fn init(&self) -> Result<(), i32> {
        unsafe {
            let result = molten_app_init(self.inner);
            if result == 0 {
                Ok(())
            } else {
                Err(result)
            }
        }
    }

    pub fn run<F>(&mut self, callback: F) -> i32
    where
        F: FnMut() + 'static,
    {
        let callback_ptr = Box::into_raw(Box::new(callback));

        unsafe {
            static mut CALLBACK_PTR: *mut std::ffi::c_void = ptr::null_mut();
            CALLBACK_PTR = callback_ptr as *mut std::ffi::c_void;

            unsafe extern "C" fn callback_wrapper<F>()
            where
                F: FnMut(),
            {
                unsafe {
                    if !CALLBACK_PTR.is_null() {
                        let callback = &mut *(CALLBACK_PTR as *mut F);
                        callback();
                    }
                }
            }

            let result = molten_app_run(self.inner, Some(callback_wrapper::<F>));

            CALLBACK_PTR = ptr::null_mut();
            let _ = Box::from_raw(callback_ptr);

            result
        }
    }

    pub fn destroy(&mut self) {
        if self.owned && !self.inner.is_null() {
            unsafe {
                molten_app_destroy(self.inner);
            }
            self.inner = ptr::null_mut();
            self.owned = false;
        }
    }
}

impl Drop for App {
    fn drop(&mut self) {
        self.destroy();
    }
}

pub mod ui {
    use super::*;

    pub fn clear_focus() {
        unsafe {
            molten_ui_clear_focus();
        }
    }

    pub fn update() {
        unsafe {
            molten_ui_update();
        }
    }

    pub fn draw_rect(x: f32, y: f32, w: f32, h: f32, color: Color) {
        unsafe {
            molten_ui_draw_rect(x, y, w, h, color.to_molten_color());
        }
    }

    pub fn draw_rounded_rect(x: f32, y: f32, w: f32, h: f32, radius: f32, color: Color) {
        unsafe {
            molten_ui_draw_rounded_rect(x, y, w, h, radius, color.to_molten_color());
        }
    }

    pub fn panel(pos: impl Into<Vec2>, size: impl Into<Vec2>, title: &str) {
        let pos = pos.into();
        let size = size.into();
        let c_title = CString::new(title).unwrap();

        unsafe {
            molten_ui_panel(
                pos.to_molten_vec2(),
                size.to_molten_vec2(),
                c_title.as_ptr(),
            );
        }
    }

    pub fn label(text: &str, pos: impl Into<Vec2>, color: Color) {
        let pos = pos.into();
        let c_text = CString::new(text).unwrap();

        unsafe {
            molten_ui_label(
                c_text.as_ptr(),
                pos.to_molten_vec2(),
                color.to_molten_color(),
            );
        }
    }

    pub fn label_small(text: &str, pos: impl Into<Vec2>, color: Color) {
        let pos = pos.into();
        let c_text = CString::new(text).unwrap();

        unsafe {
            molten_ui_label_small(
                c_text.as_ptr(),
                pos.to_molten_vec2(),
                color.to_molten_color(),
            );
        }
    }

    pub fn separator(pos: impl Into<Vec2>, width: f32) {
        let pos = pos.into();

        unsafe {
            molten_ui_separator(pos.to_molten_vec2(), width);
        }
    }

    pub fn button(id: i32, pos: impl Into<Vec2>, size: impl Into<Vec2>, label: &str) -> bool {
        let pos = pos.into();
        let size = size.into();
        let c_label = CString::new(label).unwrap();

        unsafe {
            molten_ui_button(
                id,
                pos.to_molten_vec2(),
                size.to_molten_vec2(),
                c_label.as_ptr(),
            )
        }
    }

    pub fn button_primary(
        id: i32,
        pos: impl Into<Vec2>,
        size: impl Into<Vec2>,
        label: &str,
    ) -> bool {
        let pos = pos.into();
        let size = size.into();
        let c_label = CString::new(label).unwrap();

        unsafe {
            molten_ui_button_primary(
                id,
                pos.to_molten_vec2(),
                size.to_molten_vec2(),
                c_label.as_ptr(),
            )
        }
    }

    pub fn checkbox(id: i32, value: &mut bool, pos: impl Into<Vec2>, label: &str) -> bool {
        let pos = pos.into();
        let c_label = CString::new(label).unwrap();

        unsafe {
            molten_ui_checkbox(
                id,
                value as *mut bool,
                pos.to_molten_vec2(),
                c_label.as_ptr(),
            )
        }
    }

    pub fn slider_float(
        id: i32,
        value: &mut f32,
        min: f32,
        max: f32,
        pos: impl Into<Vec2>,
        size: impl Into<Vec2>,
    ) {
        let pos = pos.into();
        let size = size.into();

        unsafe {
            molten_ui_slider_float(
                id,
                value as *mut f32,
                min,
                max,
                pos.to_molten_vec2(),
                size.to_molten_vec2(),
            );
        }
    }

    pub fn slider_int(
        id: i32,
        value: &mut i32,
        min: i32,
        max: i32,
        pos: impl Into<Vec2>,
        size: impl Into<Vec2>,
    ) {
        let pos = pos.into();
        let size = size.into();

        unsafe {
            molten_ui_slider_int(
                id,
                value as *mut i32,
                min,
                max,
                pos.to_molten_vec2(),
                size.to_molten_vec2(),
            );
        }
    }

    pub fn progress_bar(progress: f32, pos: impl Into<Vec2>, size: impl Into<Vec2>, color: Color) {
        let pos = pos.into();
        let size = size.into();

        unsafe {
            molten_ui_progress_bar(
                progress,
                pos.to_molten_vec2(),
                size.to_molten_vec2(),
                color.to_molten_color(),
            );
        }
    }

    pub fn input_field(id: i32, string: &MoltenStr, pos: impl Into<Vec2>, size: impl Into<Vec2>) {
        let pos = pos.into();
        let size = size.into();

        unsafe {
            molten_ui_input_field(
                id,
                string.as_ptr(),
                pos.to_molten_vec2(),
                size.to_molten_vec2(),
            );
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_color() {
        let color = Color::rgb(1.0, 0.5, 0.25);
        assert_eq!(color.r, 1.0);
        assert_eq!(color.g, 0.5);
        assert_eq!(color.b, 0.25);
        assert_eq!(color.a, 1.0);
    }

    #[test]
    fn test_vec2() {
        let v = Vec2::new(10.0, 20.0);
        assert_eq!(v.x, 10.0);
        assert_eq!(v.y, 20.0);

        let v2: Vec2 = (30.0, 40.0).into();
        assert_eq!(v2.x, 30.0);
        assert_eq!(v2.y, 40.0);
    }
}
