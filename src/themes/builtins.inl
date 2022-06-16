static void construct_theme_default(Btk::Theme & theme){
    theme.font = Btk::Font("<default>",13);
    theme.active.text = Btk::Color(0,0,0);
    theme.active.window = Btk::Color(239,240,241);
    theme.active.button = Btk::Color(238,240,241);
    theme.active.background = Btk::Color(252,252,252);
    theme.active.border = Btk::Color(188,190,191);
    theme.active.highlight = Btk::Color(61,174,233);
    theme.active.highlight_text = Btk::Color(255,255,255);
    theme.active.placeholder_text = Btk::Color(128,128,128);
    theme.inactive.text = Btk::Color(0,0,0);
    theme.inactive.window = Btk::Color(239,240,241);
    theme.inactive.button = Btk::Color(150,210,237);
    theme.inactive.background = Btk::Color(252,252,252);
    theme.inactive.border = Btk::Color(188,190,191);
    theme.inactive.highlight = Btk::Color(61,174,233);
    theme.inactive.highlight_text = Btk::Color(255,255,255);
    theme.inactive.placeholder_text = Btk::Color(128,128,128);
    theme.disabled.text = Btk::Color(172,173,175);
    theme.disabled.window = Btk::Color(239,240,241);
    theme.disabled.button = Btk::Color(150,210,237);
    theme.disabled.background = Btk::Color(240,240,240);
    theme.disabled.border = Btk::Color(230,232,233);
    theme.disabled.highlight = Btk::Color(61,174,233);
    theme.disabled.highlight_text = Btk::Color(255,255,255);
    theme.disabled.placeholder_text = Btk::Color(128,128,128);
}