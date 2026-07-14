"""
rebuild.py - Regenerate all C files from smart_cube.guiguider JSON.
Run this whenever GUI-Guider deletes the generated files.
Usage: python3 rebuild.py
"""
import json, os, re

BASE = os.path.dirname(os.path.abspath(__file__))
GEN = os.path.join(BASE, "generated")
GUIDER = os.path.join(BASE, "smart_cube.guiguider")

# ---- Fix .guiguider Application.screen section ----
with open(GUIDER, "r", encoding="utf-8") as f:
    data = json.load(f)

fj = data["FrontJson"]
old_app = {s["name"]: s for s in data["Application"]["screen"]}

NO_STYLE_TYPES = {"checkbox"}
NO_SIZE_TYPES = {"checkbox"}
SCROLL_TYPES = {"cont"}
CONTAINER_TYPES = {"cont"}

def convert_widget(w, screen_name):
    wt = w["type"]
    app_w = {
        "name": f"{screen_name}_{w['name']}",
        "id": w["id"], "type": wt,
        "visible": w.get("visible", True),
        "flag": w.get("flag", []),
        "pos": [w.get("left", 0), w.get("top", 0)],
    }
    app_w["size"] = [None, None] if wt in NO_SIZE_TYPES else [w.get("width", 0), w.get("height", 0)]
    if wt in SCROLL_TYPES:
        app_w["scrollbar_mode"] = w.get("scrollbar_mode", "OFF")
    if wt not in NO_STYLE_TYPES:
        app_w["default_style"] = w.get("default_style", False)
        app_w["style"] = w.get("style", [])
    if wt in CONTAINER_TYPES and "child" in w:
        app_w["child"] = [convert_widget(cw, screen_name) for cw in w["child"]]
    return app_w

new_screens = []
for fjs in fj:
    sn = fjs["name"]
    old = old_app.get(sn, {})
    new_screens.append({
        "name": sn, "id": old.get("id", fjs.get("id", "")), "type": "scr",
        "version": old.get("version", 190), "scrollbar_mode": "OFF",
        "customer_code": old.get("customer_code", {"code_c":"","inc_c":"","code_py":"","inc_py":"","type":"screen"}),
        "width": fjs.get("width", 320), "height": fjs.get("height", 240),
        "visible": fjs.get("visible", True),
        "flag": old.get("flag", []), "event": old.get("event", {}),
        "style": old.get("style", fjs.get("style", [])),
        "size": [fjs.get("width", 320), fjs.get("height", 240)],
        "layerName": sn,
        "widgets": [convert_widget(w, sn) for w in fjs["list"]],
    })

data["Application"]["screen"] = new_screens
with open(GUIDER, "w", encoding="utf-8") as f:
    json.dump(data, f, ensure_ascii=False, indent=2)
print("[1/4] Fixed .guiguider Application.screen")

# ---- Generate setup_scr_*.c files ----
FONTS = {
    "yezi_hei": {10: "lv_font_yezi_hei_10", 15: "lv_font_yezi_hei_10"},
    "montserratMedium": {16: "lv_font_montserratMedium_12", 18: "lv_font_montserratMedium_12"},
}

def hex_color(c):
    return "0x" + c[1:] if c and c.startswith("#") else c

def img_var_name(path):
    return "_" + os.path.splitext(os.path.basename(path))[0]

def c_name(screen, widget):
    return f"{screen}_{widget}"

# Build image var name map from actual generated files
IMG_MAP = {}
img_dir = os.path.join(GEN, "images")
if os.path.isdir(img_dir):
    for fn in os.listdir(img_dir):
        if fn.endswith(".c"):
            with open(os.path.join(img_dir, fn), "r") as f:
                m = re.search(r'const lv_img_dsc_t (\w+)', f.read())
                if m:
                    IMG_MAP[fn.replace(".c", "")] = m.group(1)

def resolve_img_var(path):
    base = "_" + os.path.splitext(os.path.basename(path))[0]
    return IMG_MAP.get(base, base)

def gen_setup_scr(screen_data, screen_name):
    widgets = screen_data.get("list", [])
    L = []
    L.append(f'/* GUI-Guider setup for {screen_name} */')
    L.append('#include "lvgl.h"')
    L.append('#include "gui_guider.h"')
    L.append('')
    L.append(f'void setup_scr_{screen_name}(lv_ui *ui)')
    L.append('{')
    L.append(f'    ui->{screen_name} = lv_obj_create(NULL);')
    L.append(f'    lv_obj_set_size(ui->{screen_name}, 320, 240);')

    for s in screen_data.get("style", []):
        c = hex_color(s.get("bg_color", ""))
        if c != "0x":
            L.append(f'    lv_obj_set_style_bg_color(ui->{screen_name}, lv_color_hex({c}), LV_PART_MAIN|LV_STATE_DEFAULT);')
        L.append(f'    lv_obj_set_style_bg_opa(ui->{screen_name}, {s.get("bg_opa", 255)}, LV_PART_MAIN|LV_STATE_DEFAULT);')
        bi = s.get("bg_img_src", "")
        if bi:
            iv = resolve_img_var(bi)
            L.append(f'    LV_IMG_DECLARE({iv});')
            L.append(f'    lv_obj_set_style_bg_img_src(ui->{screen_name}, &{iv}, LV_PART_MAIN|LV_STATE_DEFAULT);')
            L.append(f'    lv_obj_set_style_bg_img_opa(ui->{screen_name}, {s.get("bg_img_opa", 255)}, LV_PART_MAIN|LV_STATE_DEFAULT);')

    L.append('')

    def gen_w(w, parent_ref):
        wt, wn = w["type"], w["name"]
        cid = c_name(screen_name, wn)
        ref = f"ui->{cid}"
        left, top = w.get("left", 0), w.get("top", 0)
        width, height = w.get("width", 0), w.get("height", 0)
        visible, text = w.get("visible", True), w.get("text", "")
        sm = w.get("scrollbar_mode", "OFF")

        if wt == "label":
            L.append(f'    {ref} = lv_label_create({parent_ref});')
            L.append(f'    lv_obj_set_pos({ref}, {left}, {top});')
            L.append(f'    lv_obj_set_size({ref}, {width}, {height});')
            if text:
                esc = text.replace("\\", "\\\\").replace('"', '\\"')
                L.append(f'    lv_label_set_text({ref}, "{esc}");')
            L.append(f'    lv_label_set_long_mode({ref}, {w.get("long_mode", "LV_LABEL_LONG_WRAP")});')

        elif wt == "btn":
            L.append(f'    {ref} = lv_btn_create({parent_ref});')
            L.append(f'    lv_obj_set_pos({ref}, {left}, {top});')
            L.append(f'    lv_obj_set_size({ref}, {width}, {height});')
            lr = f"ui->{cid}_label"
            L.append(f'    {lr} = lv_label_create({ref});')
            if text:
                esc = text.replace("\\", "\\\\").replace('"', '\\"')
                L.append(f'    lv_label_set_text({lr}, "{esc}");')
            L.append(f'    lv_label_set_long_mode({lr}, {w.get("long_mode", "LV_LABEL_LONG_WRAP")});')
            L.append(f'    lv_obj_set_style_text_align({lr}, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);')
            L.append(f'    lv_obj_center({lr});')

        elif wt == "cont":
            L.append(f'    {ref} = lv_obj_create({parent_ref});')
            L.append(f'    lv_obj_set_pos({ref}, {left}, {top});')
            L.append(f'    lv_obj_set_size({ref}, {width}, {height});')
            if sm != "OFF":
                L.append(f'    lv_obj_set_scrollbar_mode({ref}, LV_SCROLLBAR_MODE_{sm});')
            for cw in w.get("child", []):
                gen_w(cw, ref)

        elif wt == "img":
            L.append(f'    {ref} = lv_img_create({parent_ref});')
            L.append(f'    lv_obj_set_pos({ref}, {left}, {top});')
            L.append(f'    lv_obj_set_size({ref}, {width}, {height});')
            ip = w.get("img_path", "")
            if ip:
                iv = resolve_img_var(ip)
                L.append(f'    LV_IMG_DECLARE({iv});')
                L.append(f'    lv_img_set_src({ref}, &{iv});')
            if w.get("x") or w.get("y"):
                L.append(f'    lv_img_set_pivot({ref}, {w.get("x", 0)}, {w.get("y", 0)});')
            if w.get("rotate"):
                L.append(f'    lv_img_set_angle({ref}, {w.get("rotate", 0)});')

        elif wt == "slider":
            L.append(f'    {ref} = lv_slider_create({parent_ref});')
            L.append(f'    lv_obj_set_pos({ref}, {left}, {top});')
            L.append(f'    lv_obj_set_size({ref}, {width}, {height});')
            sr = w.get("slider_range", [10, 100])
            L.append(f'    lv_slider_set_range({ref}, {sr[0]}, {sr[1]});')
            L.append(f'    lv_slider_set_value({ref}, {w.get("slider_value", 80)}, LV_ANIM_OFF);')
            L.append(f'    lv_slider_set_mode({ref}, {w.get("slider_mode", "LV_SLIDER_MODE_NORMAL")});')

        elif wt == "checkbox":
            L.append(f'    {ref} = lv_checkbox_create({parent_ref});')
            L.append(f'    lv_obj_set_pos({ref}, {left}, {top});')
            if text:
                esc = text.replace("\\", "\\\\").replace('"', '\\"')
                L.append(f'    lv_checkbox_set_text({ref}, "{esc}");')

        if not visible:
            L.append(f'    lv_obj_add_flag({ref}, LV_OBJ_FLAG_HIDDEN);')

        # Styles
        for s in w.get("style", []):
            if s.get("disable"): continue
            sel = f'{s.get("part", "LV_PART_MAIN")}|{s.get("state", "LV_STATE_DEFAULT")}'

            c = s.get("bg_color", "")
            if c and c.startswith("#"):
                L.append(f'    lv_obj_set_style_bg_color({ref}, lv_color_hex({hex_color(c)}), {sel});')
            if "bg_color" in s:
                L.append(f'    lv_obj_set_style_bg_opa({ref}, {s.get("bg_opa", 255)}, {sel});')

            tc = s.get("text_color", "")
            if tc and tc.startswith("#"):
                L.append(f'    lv_obj_set_style_text_color({ref}, lv_color_hex({hex_color(tc)}), {sel});')

            ta = s.get("text_align", "")
            if ta:
                L.append(f'    lv_obj_set_style_text_align({ref}, {ta}, {sel});')

            ff, fs = s.get("font_family", ""), s.get("font", 0)
            if ff in FONTS and fs in FONTS[ff]:
                L.append(f'    lv_obj_set_style_text_font({ref}, &{FONTS[ff][fs]}, {sel});')

            if s.get("border_width", 0) > 0:
                L.append(f'    lv_obj_set_style_border_width({ref}, {s["border_width"]}, {sel});')
            bc = s.get("border_color", "")
            if bc and bc.startswith("#"):
                L.append(f'    lv_obj_set_style_border_color({ref}, lv_color_hex({hex_color(bc)}), {sel});')

            r = s.get("radius", 0)
            if r > 0:
                L.append(f'    lv_obj_set_style_radius({ref}, {r}, {sel});')

            pt, pr, pb, pl = s.get("padding_top",0), s.get("padding_right",0), s.get("padding_bottom",0), s.get("padding_left",0)
            if pt or pr or pb or pl:
                if pt == pb == pl == pr and pt:
                    L.append(f'    lv_obj_set_style_pad_all({ref}, {pt}, {sel});')
                else:
                    for pn, pv in [("top", pt), ("bottom", pb), ("left", pl), ("right", pr)]:
                        if pv: L.append(f'    lv_obj_set_style_pad_{pn}({ref}, {pv}, {sel});')

            gd = s.get("bg_grad_dir", "LV_GRAD_DIR_NONE")
            if gd != "LV_GRAD_DIR_NONE":
                L.append(f'    lv_obj_set_style_bg_grad_dir({ref}, {gd}, {sel});')
            gc = s.get("bg_grad_color", "")
            if gc and gc.startswith("#"):
                L.append(f'    lv_obj_set_style_bg_grad_color({ref}, lv_color_hex({hex_color(gc)}), {sel});')

            if wt == "img":
                ir = s.get("image_recolor", "")
                if ir and ir.startswith("#"):
                    L.append(f'    lv_obj_set_style_img_recolor({ref}, lv_color_hex({hex_color(ir)}), {sel});')
                if s.get("image_recolor_opa", 0):
                    L.append(f'    lv_obj_set_style_img_recolor_opa({ref}, {s["image_recolor_opa"]}, {sel});')
                if s.get("image_opa", 255) < 255:
                    L.append(f'    lv_obj_set_style_img_opa({ref}, {s["image_opa"]}, {sel});')

            if s.get("letter_space", 0):
                L.append(f'    lv_obj_set_style_text_letter_space({ref}, {s["letter_space"]}, {sel});')
            if s.get("line_space", 0):
                L.append(f'    lv_obj_set_style_text_line_space({ref}, {s["line_space"]}, {sel});')

    for w in widgets:
        gen_w(w, f"ui->{screen_name}")

    L.append('}')
    return '\n'.join(L)

for screen in fj:
    sn = screen["name"]
    content = gen_setup_scr(screen, sn)
    path = os.path.join(GEN, f"setup_scr_{sn}.c")
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"[2/4] Generated setup_scr_{sn}.c ({len(content)} bytes)")

# ---- Fix font references (use only available fonts) ----
print("[3/4] Fixing font references...")
for fn in os.listdir(GEN):
    if fn.startswith("setup_scr_") and fn.endswith(".c"):
        fp = os.path.join(GEN, fn)
        with open(fp, "r", encoding="utf-8") as f:
            c = f.read()
        c = c.replace("lv_font_yezi_hei_15", "lv_font_yezi_hei_10")
        c = c.replace("lv_font_montserratMedium_18", "lv_font_montserratMedium_12")
        with open(fp, "w", encoding="utf-8") as f:
            f.write(c)

# ---- Fix image variable names ----
print("[4/4] Fixing image references...")
for fn in os.listdir(GEN):
    if fn.startswith("setup_scr_") and fn.endswith(".c"):
        fp = os.path.join(GEN, fn)
        with open(fp, "r", encoding="utf-8") as f:
            c = f.read()
        for base, var in sorted(IMG_MAP.items(), key=lambda x: -len(x[0])):
            # Extract short name (e.g. _img_bg_main from _img_bg_main_320x240)
            short = base
            for suffix in ["_320x240", "_alpha_80x60", "_alpha_50x42", "_alpha_40x40", "_alpha_50x50"]:
                if base.endswith(suffix):
                    short = base[:-len(suffix)]
                    break
            if short != base and short != "":
                # Replace short name with full variable name in C code
                c = c.replace(f'"{short}"', f'"{var}"')  # never needed
                # Replace &_short_name with &_full_name in LV_IMG_DECLARE and lv_img_set_src
                c = c.replace(f'({short})', f'({var})')
                c = c.replace(f'&{short},', f'&{var},')
                c = c.replace(f'&{short})', f'&{var})')
                c = c.replace(f' {short};', f' {var};')
                c = c.replace(f'{short},', f'{var},')
                c = c.replace(f'{short})', f'{var})')
        with open(fp, "w", encoding="utf-8") as f:
            f.write(c)

# ---- Verify ----
print()
print("Verification:")
for screen in fj:
    sn = screen["name"]
    fjc = len(screen["list"])
    app_scr = next((s for s in new_screens if s["name"] == sn), None)
    appc = len(app_scr["widgets"]) if app_scr else 0
    sf = os.path.join(GEN, f"setup_scr_{sn}.c")
    sf_ok = "OK" if os.path.exists(sf) else "MISSING"
    m = "OK" if fjc == appc else "MISMATCH"
    print(f"  {sn}: FJ={fjc} App={appc} {m}  setup_scr={sf_ok}")

print("\nDone! Now run 'mingw32-make' in the lvgl-simulator directory.")
