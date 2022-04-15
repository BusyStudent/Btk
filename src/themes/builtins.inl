static void construct_theme_default(Btk::Theme & theme){
    theme.font = Btk::Font("<default>",13);
    theme.active.text = Btk::Color(0,0,0);
    theme.active.window = Btk::Color(239,240,241);
    theme.active.button = Btk::Color(233,234,235);
    theme.active.background = Btk::Color(255,255,255);
    theme.active.border = Btk::Color(208,208,208);
    theme.active.highlight = Btk::Color(61,174,233);
    theme.active.highlight_text = Btk::Color(255,255,255);
}