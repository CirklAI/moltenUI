use molten_ui::{ui, App, Color, MoltenStr, ThemeColors};
use std::time::Instant;

struct DemoState {
    fps: f32,
    frame_count: u32,
    last_time: Instant,
    email: MoltenStr,
    password: MoltenStr,
    remember_me: bool,
    toggle1: bool,
    toggle2: bool,
    toggle3: bool,
    volume: f32,
    brightness: f32,
    red: f32,
    green: f32,
    blue: f32,
    count: i32,
    switch: bool,
}

impl DemoState {
    fn new() -> Self {
        Self {
            fps: 60.0,
            frame_count: 0,
            last_time: Instant::now(),
            email: MoltenStr::new(""),
            password: MoltenStr::new(""),
            remember_me: true,
            toggle1: true,
            toggle2: false,
            toggle3: true,
            volume: 75.0,
            brightness: 50.0,
            red: 200.0,
            green: 50.0,
            blue: 255.0,
            count: 5,
            switch: true,
        }
    }

    fn update_fps(&mut self) {
        self.frame_count += 1;
        let now = Instant::now();
        let elapsed = now.duration_since(self.last_time).as_secs_f32();

        if elapsed >= 1.0 {
            self.fps = self.frame_count as f32 / elapsed;
            self.frame_count = 0;
            self.last_time = now;
        }
    }
}

fn main() {
    ThemeColors::set_purple_theme();

    let mut app = App::new("MoltenUI Demo", 1280, 720).expect("Failed to create app");

    if let Err(code) = app.init() {
        eprintln!("Failed to initialize app: {}", code);
        return;
    }

    let mut state = DemoState::new();

    let exit_code = app.run(move || {
        ui::clear_focus();
        let theme = ThemeColors::get_current();

        ui::draw_rect(0.0, 0.0, 1280.0, 720.0, theme.background);

        state.update_fps();

        const COL_W: f32 = 220.0;
        const GAP: f32 = 20.0;
        const COL1_X: f32 = 20.0;
        const COL2_X: f32 = COL1_X + COL_W + GAP;
        const COL3_X: f32 = COL2_X + COL_W + GAP;
        const COL4_X: f32 = COL3_X + COL_W + GAP;
        const START_Y: f32 = 20.0;

        let mut y = START_Y;
        ui::panel((COL1_X, y), (COL_W, 230.0), "Sign In");
        y += 45.0;
        ui::input_field(1, &state.email, (COL1_X + 15.0, y), (COL_W - 30.0, 36.0));
        y += 45.0;
        ui::input_field(2, &state.password, (COL1_X + 15.0, y), (COL_W - 30.0, 36.0));
        y += 45.0;
        ui::checkbox(3, &mut state.remember_me, (COL1_X + 15.0, y), "Remember me");
        y += 50.0;
        ui::button_primary(4, (COL1_X + 15.0, y), (COL_W - 30.0, 36.0), "Sign In");

        y = 250.0 + 25.0;
        ui::panel((COL1_X, y), (COL_W, 140.0), "Toggles");
        y += 45.0;
        ui::checkbox(10, &mut state.toggle1, (COL1_X + 15.0, y), "Enabled");
        y += 30.0;
        ui::checkbox(11, &mut state.toggle2, (COL1_X + 15.0, y), "Notifications");
        y += 30.0;
        ui::checkbox(12, &mut state.toggle3, (COL1_X + 15.0, y), "Auto-save");

        y = START_Y;
        ui::panel((COL2_X, y), (COL_W, 235.0), "Buttons");
        y += 45.0;
        ui::button_primary(20, (COL2_X + 15.0, y), (COL_W - 30.0, 32.0), "Submit");
        y += 42.0;
        ui::button(21, (COL2_X + 15.0, y), (COL_W - 30.0, 32.0), "Cancel");
        y += 42.0;
        ui::button(22, (COL2_X + 15.0, y), (COL_W - 30.0, 32.0), "Delete");
        y += 42.0;
        ui::button(23, (COL2_X + 15.0, y), (90.0, 28.0), "Edit");
        ui::button(24, (COL2_X + 115.0, y), (90.0, 28.0), "Copy");

        y = 255.0 + 25.0;
        ui::panel((COL2_X, y), (COL_W, 145.0), "Sliders");
        y += 45.0;
        ui::label(
            &format!("{}%", state.volume as i32),
            (COL2_X + 15.0, y + 12.0),
            theme.text_muted,
        );
        ui::slider_float(
            14,
            &mut state.volume,
            0.0,
            100.0,
            (COL2_X + 55.0, y),
            (COL_W - 75.0, 24.0),
        );
        y += 45.0;
        ui::slider_float(
            15,
            &mut state.brightness,
            0.0,
            100.0,
            (COL2_X + 15.0, y),
            (COL_W - 30.0, 16.0),
        );
        y += 28.0;
        ui::progress_bar(
            state.brightness / 100.0,
            (COL2_X + 15.0, y),
            (COL_W - 30.0, 10.0),
            theme.accent,
        );

        y = START_Y;
        ui::panel((COL3_X, y), (COL_W, 260.0), "Info");
        y += 45.0;
        ui::label("moltenUI", (COL3_X + 15.0, y), theme.accent);
        y += 24.0;
        ui::label_small(
            "v1.0.0 (Vulkan Backend)",
            (COL3_X + 15.0, y),
            theme.text_muted,
        );
        y += 24.0;
        ui::separator((COL3_X + 15.0, y), COL_W - 30.0);
        y += 20.0;
        ui::label_small("- Rounded corners", (COL3_X + 15.0, y), theme.text_muted);
        y += 18.0;
        ui::label_small("- Theme support", (COL3_X + 15.0, y), theme.text_muted);
        y += 24.0;
        ui::separator((COL3_X + 15.0, y), COL_W - 30.0);
        y += 20.0;
        ui::label(
            &format!("FPS: {}", state.fps as i32),
            (COL3_X + 15.0, y),
            theme.text,
        );

        y = 280.0 + 25.0;
        ui::panel((COL3_X, y), (COL_W, 165.0), "RGB Color");
        y += 45.0;
        ui::slider_float(
            40,
            &mut state.red,
            0.0,
            255.0,
            (COL3_X + 15.0, y),
            (COL_W - 30.0, 14.0),
        );
        y += 22.0;
        ui::slider_float(
            41,
            &mut state.green,
            0.0,
            255.0,
            (COL3_X + 15.0, y),
            (COL_W - 30.0, 14.0),
        );
        y += 22.0;
        ui::slider_float(
            42,
            &mut state.blue,
            0.0,
            255.0,
            (COL3_X + 15.0, y),
            (COL_W - 30.0, 14.0),
        );
        y += 26.0;
        ui::draw_rounded_rect(
            COL3_X + 15.0,
            y,
            COL_W - 30.0,
            30.0,
            4.0,
            Color::rgb(state.red / 255.0, state.green / 255.0, state.blue / 255.0),
        );

        y = START_Y;
        ui::panel((COL4_X, y), (COL_W, 400.0), "All Widgets");
        y += 45.0;
        ui::label("Labels:", (COL4_X + 15.0, y), theme.text);
        y += 24.0;
        ui::label_small("Small label", (COL4_X + 15.0, y), theme.text_muted);
        y += 30.0;
        ui::separator((COL4_X + 15.0, y), COL_W - 30.0);
        y += 20.0;
        ui::checkbox(60, &mut state.switch, (COL4_X + 15.0, y), "Switch");
        y += 35.0;
        ui::separator((COL4_X + 15.0, y), COL_W - 30.0);
        y += 20.0;
        ui::slider_int(
            61,
            &mut state.count,
            0,
            10,
            (COL4_X + 15.0, y),
            (COL_W - 30.0, 16.0),
        );
        y += 45.0;
        ui::label(
            &format!("Count: {}", state.count),
            (COL4_X + 15.0, y),
            theme.text_muted,
        );
        y += 35.0;
        ui::separator((COL4_X + 15.0, y), COL_W - 30.0);
        y += 20.0;
        ui::draw_rounded_rect(COL4_X + 15.0, y, 50.0, 30.0, 6.0, theme.error);
        ui::draw_rounded_rect(COL4_X + 75.0, y, 50.0, 30.0, 6.0, theme.success);
        ui::draw_rounded_rect(COL4_X + 135.0, y, 50.0, 30.0, 6.0, theme.warning);
        y += 45.0;
        ui::draw_rect(COL4_X + 15.0, y, COL_W - 30.0, 15.0, theme.border);

        ui::update();
    });

    app.destroy();
    std::process::exit(exit_code);
}
