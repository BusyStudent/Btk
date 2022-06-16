#!/usr/bin/env python3
import configparser
import os
import re
from sys import platform

internal_varname = "theme"

default_font_ptsize = 12

palette_options_map = {
    "map" : None
}

def palette_map_color(s : str) -> str:
    return s
#Generate String to
def parse_color(c : str) -> str:
    c = c.strip()
    if c.startswith("RGBA(") and c.endswith(")"):
        l = c[5:-1].split(',')
        if len(l) == 3:
            #Only 3 values
            return "Btk::Color(%s,%s,%s)" % (l[0],l[1],l[2])
        else:
            return "Btk::Color(%s,%s,%s,%s)" % (l[0],l[1],l[2],l[3])
        pass
    elif c.startswith("RGB(") and c.endswith(")"):
        l = c[4:-1].split(',')
        return "Btk::Color(%s,%s,%s)" % (l[0],l[1],l[2])
    elif c.startswith("#"):
        if len(c) == 9:
            return "Btk::Color(0x%s,0x%s,0x%s,0x%s)" % (c[1:3],c[3:5],c[5:7],c[7:9])
        else:
            return "Btk::Color(0x%s,0x%s,0x%s)" % (c[1:3],c[3:5],c[5:7])
    raise RuntimeError("Invalid param %s" % c)
# Make a brush construct string from c
def parse_brush(c : str) -> str:
    if c.startswith("LinearGradient(") and c.endswith(")"):
        pat = re.compile("""Stop\(.+?\)""")

        ret = "[]() -> Btk::Brush {\n"
        ret += "        Btk::LinearGradient grad;\n"
        for stop in pat.findall(c):
            stop = stop.strip()[5:]
            #Find first ,
            pos = stop.find(",")
            if pos == -1:
                raise RuntimeError("Invalid stop %s" % stop)
            #Split on ,

            pos_str = stop[:pos].lower()
            color_str = stop[pos+1:]

            if pos_str.endswith('f'):
                pos_str = pos_str[:-1]
            
            pos = float(pos_str)
            color_str = parse_color(color_str)

            ret += "        grad.add_color(%s,%s);\n" % (pos,color_str)
        ret += "        return Btk::Brush(std::move(grad));\n"
        ret += "    }()"
        return ret
    else:
        return parse_color(c)

def handle_ui(conf : configparser.ConfigParser,name : str) -> str:
    return ""
def handle_font(conf : configparser.ConfigParser(),name : str) -> str:
    fname = "<default>"
    ptsize = str(default_font_ptsize)
    if "name" in conf[name]:
        fname = conf[name]["name"]
    if "ptsize" in conf[name]:
        ptsize = conf[name]["ptsize"]
    return "    %s.font = Btk::Font(\"%s\",%s);\n" % (internal_varname,fname,ptsize)

def handle_palette(conf : configparser.ConfigParser,name : str) -> str:
    values = conf.items(name)
    #Get active or disabled etc...
    palette_area = name[name.find("::") + len("::"):].lower()
    result = ""
    for v in values:
        op = v[1]
        #Generate theme.p.v = Color(...)
        result += "    %s.%s.%s = %s;\n" % (internal_varname,palette_area,v[0],parse_brush(op))
    return result
        

section_map = {
    "ui" : handle_ui,
    "font" : handle_font,
    "palette::active" : handle_palette,
    "palette::disabled" : handle_palette,
    "palette::inactive" : handle_palette,
}
# Just process and generate calls
def compile_raw(conf : configparser.ConfigParser) -> str:
    result = ""
    for sect in conf.sections():
        t = sect.lower()
        result += section_map[t](conf,sect)
    return result
# Add function body
def compile(name : str,conf : configparser.ConfigParser) -> str:
    result = ""
    result += "static void construct_theme_%s(Btk::Theme & %s){\n" % (name,internal_varname)
    result += compile_raw(conf)
    result += "}"
    return result
def compile_file(filename : str,out = None):
    # Make conf parser
    conf = configparser.ConfigParser()

    if filename not in conf.read(filename):
        # ReadError
        raise FileNotFoundError(filename)

    name = os.path.basename(filename)
    name = name[:name.find(".")]

    result = compile(name,conf)

    # Write to stream
    if out == None:
        output_dir = filename[:filename.rfind('.')] + ".inl"
        with open(output_dir,"w") as f:
            f.write(result)
    else:
        out.write(result);

if __name__ == "__main__":
    # print(parse_color("RGBA(1,2,3)"))
    # print(parse_color("RGBA(1,2,3,5)"))
    # print(parse_color("#FFCCAA"))
    # print(parse_color("#FFCCAABB"))

    # conf = configparser.ConfigParser()
    # conf.read("../themes/default.conf")
    # print(compile("a",conf));
    with open("../src/themes/builtins.inl","w") as f:
        compile_file("../themes/default.conf",f)