import math
from pathlib import Path

import bpy

BASE = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def mat(name, color):
    m = bpy.data.materials.new(name)
    m.diffuse_color = color
    return m


def box(name, loc, scale, material=None):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if material:
        obj.data.materials.append(material)
    return obj


def cyl(name, loc, radius, depth, axis="Z", vertices=48, material=None):
    rot = (0, 0, 0)
    if axis == "X":
        rot = (0, math.radians(90), 0)
    elif axis == "Y":
        rot = (math.radians(90), 0, 0)
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=loc, rotation=rot)
    obj = bpy.context.object
    obj.name = name
    if material:
        obj.data.materials.append(material)
    return obj


def diff(target, cutter):
    bpy.context.view_layer.objects.active = target
    target.select_set(True)
    mod = target.modifiers.new("cut_" + cutter.name, "BOOLEAN")
    mod.operation = "DIFFERENCE"
    mod.object = cutter
    mod.solver = "EXACT"
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.data.objects.remove(cutter, do_unlink=True)


def join(name, objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    bpy.context.object.name = name
    return bpy.context.object


def bevel(obj, width=0.6, segments=2):
    bpy.context.view_layer.objects.active = obj
    mod = obj.modifiers.new("edge_bevel", "BEVEL")
    mod.width = width
    mod.segments = segments
    mod.affect = "EDGES"
    bpy.ops.object.modifier_apply(modifier=mod.name)
    mod = obj.modifiers.new("weighted_normals", "WEIGHTED_NORMAL")
    bpy.ops.object.modifier_apply(modifier=mod.name)


def export_stl(obj, filename):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.wm.stl_export(filepath=str(BASE / filename), export_selected_objects=True)


def add_screw_socket(parts, material, y, z, idx):
    boss = cyl(f"body_rear_screw_socket_{idx}", (50, y, z), 6.0, 14, axis="X", material=material)
    diff(boss, cyl(f"body_rear_screw_pilot_{idx}", (50, y, z), 1.8, 18, axis="X"))
    parts.append(boss)
    parts.append(box(f"body_screw_rib_side_{idx}", (50, y, z), (14, 10, 4), material))
    parts.append(box(f"body_screw_rib_wall_{idx}", (50, y, 9 if z < 58 else 107), (14, 8, 18), material))


def make_body(material):
    # X = front/back, Y = left/right, Z = height. Front is -X, rear is +X.
    outer = box("v10_body_outer_shell", (0, 0, 58), (112, 132, 116), material)
    diff(outer, box("rear_open_cavity", (8, 0, 60), (106, 118, 102)))
    diff(outer, box("front_screen_service_opening", (-58, 0, 58), (18, 116, 88)))

    # Side ventilation for air sensor area.
    for side_y in (-66, 66):
        for z in (72, 80, 88, 96):
            diff(outer, box(f"side_air_louver_{side_y}_{z}", (10, side_y, z), (58, 14, 4)))

    # Top transparent light window opening over the BH1750 bay.
    diff(outer, box("top_light_window_cutout", (-18, 42, 116), (48, 36, 12)))

    parts = [outer]

    # Front screen support and continuous cable runway. The right side stays open to route Dupont wires backward.
    parts += [
        box("screen_bottom_ledge", (-50, 0, 13), (12, 116, 4), material),
        box("screen_top_ledge", (-50, 0, 103), (12, 116, 4), material),
        box("screen_left_lip", (-50, -58, 58), (12, 4, 90), material),
        box("screen_right_lip_with_wire_gap", (-50, 58, 58), (12, 4, 90), material),
        box("screen_wire_runway_floor", (-8, 52, 34), (82, 24, 4), material),
        box("screen_wire_runway_outer_guard", (-8, 64, 54), (82, 4, 44), material),
        box("screen_wire_runway_inner_low_guard", (-8, 40, 22), (82, 4, 18), material),
    ]

    # Lower trays. DC is left, ESP32-S3 is right so screen wires can run directly to ESP32.
    # Each tray uses a rectangular envelope: PCB + pins + Dupont plug height + wire bend space.
    parts += [
        # DC power board tray, envelope 84 x 52 x 28.
        box("dc_tray_floor", (5, -35, 8), (88, 56, 4), material),
        box("dc_tray_left_rail", (5, -64, 20), (88, 4, 24), material),
        box("dc_tray_right_rail", (5, -6, 20), (88, 4, 24), material),
        box("dc_tray_left_top_lip", (5, -58, 33), (82, 10, 4), material),
        box("dc_tray_right_top_lip", (5, -12, 33), (82, 10, 4), material),
        box("dc_tray_front_stop", (-42, -35, 22), (4, 56, 28), material),
        box("dc_rear_port_alignment_abutment", (47, -35, 20), (5, 42, 22), material),
        # ESP32-S3 tray, envelope 82 x 52 x 28.
        box("esp32_tray_floor", (5, 35, 8), (88, 56, 4), material),
        box("esp32_tray_left_rail", (5, 6, 20), (88, 4, 24), material),
        box("esp32_tray_right_rail", (5, 64, 20), (88, 4, 24), material),
        box("esp32_tray_left_top_lip", (5, 12, 33), (82, 10, 4), material),
        box("esp32_tray_right_top_lip", (5, 58, 33), (82, 10, 4), material),
        box("esp32_tray_front_stop", (-42, 35, 22), (4, 56, 28), material),
        box("esp32_rear_port_alignment_abutment", (47, 35, 20), (5, 42, 22), material),
    ]

    # Upper sensor deck: three separate bays with clear spacing, no mutual squeeze.
    parts += [
        box("upper_sensor_deck", (-8, 0, 62), (84, 116, 4), material),
        box("upper_sensor_front_web", (-49, 0, 52), (4, 116, 22), material),
        box("upper_sensor_rear_web", (35, 0, 52), (4, 116, 22), material),
        box("upper_sensor_left_wall", (-8, -58, 52), (84, 4, 22), material),
        box("upper_sensor_right_wall", (-8, 58, 52), (84, 4, 22), material),
        box("sensor_bay_divider_ld_env", (-8, -24, 73), (84, 3, 22), material),
        box("sensor_bay_divider_env_bh", (-8, 24, 73), (84, 3, 22), material),
        # LD2410C bay: 16 x 22 mm board, slot envelope 42 x 34 x 20.
        box("ld2410c_floor", (-18, -42, 67), (42, 32, 4), material),
        box("ld2410c_left_rail", (-18, -60, 76), (42, 3, 18), material),
        box("ld2410c_right_rail", (-18, -24, 76), (42, 3, 18), material),
        # ENS160 + AHT21 bay: measured 19.6 x 24.3, slot envelope 46 x 38 x 20.
        box("ens160_aht21_floor", (-18, 0, 67), (46, 36, 4), material),
        box("ens160_aht21_left_rail", (-18, -20, 76), (46, 3, 18), material),
        box("ens160_aht21_right_rail", (-18, 20, 76), (46, 3, 18), material),
        # BH1750 bay: measured 32.6 x 15.1, slot envelope 50 x 28 x 20.
        box("bh1750_floor_under_light_window", (-18, 43, 67), (50, 28, 4), material),
        box("bh1750_left_rail", (-18, 27, 76), (50, 3, 18), material),
        box("bh1750_right_rail", (-18, 59, 76), (50, 3, 18), material),
        # Recess/lip for a clear acrylic or clear PETG lens above BH1750.
        box("light_window_front_lip", (-18, 24, 111), (52, 3, 5), material),
        box("light_window_back_lip", (-18, 60, 111), (52, 3, 5), material),
        box("light_window_left_lip", (-44, 42, 111), (3, 36, 5), material),
        box("light_window_right_lip", (8, 42, 111), (3, 36, 5), material),
    ]

    for idx, (y, z) in enumerate([(-58, 16), (58, 16), (-58, 104), (58, 104)]):
        add_screw_socket(parts, material, y, z, idx)

    body = join("smart_cube_shell_v10_body_air_light_ports", parts)
    bevel(body, 0.65, 2)
    return body


def make_front_bezel(material):
    bezel = box("v10_front_bezel_outer", (-61, 0, 58), (8, 126, 98), material)
    diff(bezel, box("v10_front_visible_window", (-61, 0, 58), (12, 82, 60)))
    diff(bezel, box("v10_screen_pcb_back_relief", (-57, 0, 58), (6, 108, 82)))
    # Extra side outlet for the screen pin row and already-attached Dupont wires.
    diff(bezel, box("v10_screen_wire_side_exit", (-57, 55, 49), (8, 24, 58)))

    parts = [bezel]
    for y, z in [(-54, 20), (54, 20), (-54, 96), (54, 96)]:
        boss = cyl("front_bezel_boss", (-56, y, z), 4.7, 6, axis="X", material=material)
        diff(boss, cyl("front_bezel_hole", (-56, y, z), 1.8, 10, axis="X"))
        parts.append(boss)
    obj = join("smart_cube_shell_v10_front_screen_clamp_bezel", parts)
    bevel(obj, 0.5, 2)
    return obj


def make_back_cover(material):
    cover = box("v10_back_cover_plate", (57, 0, 58), (8, 132, 116), material)

    # Real through ports for external insertion.
    diff(cover, box("v10_dc_port_rect_clearance", (57, -35, 24), (14, 24, 22)))
    diff(cover, cyl("v10_dc_port_round_clearance", (57, -35, 24), 6.2, 16, axis="X"))
    diff(cover, box("v10_typec_port_clearance", (57, 35, 24), (14, 26, 14)))

    # Rear louver ventilation, aligned with the upper air sensor deck.
    for z in (76, 84, 92, 100):
        diff(cover, box(f"v10_rear_air_louver_{z}", (57, 0, z), (14, 100, 4)))

    parts = [cover]

    # Port reinforcement frames and anti-push pads.
    parts += [
        box("dc_port_lower_reinforcement", (51, -35, 9), (10, 34, 8), material),
        box("dc_port_upper_reinforcement", (51, -35, 39), (10, 34, 8), material),
        box("typec_port_lower_reinforcement", (51, 35, 11), (10, 36, 8), material),
        box("typec_port_upper_reinforcement", (51, 35, 37), (10, 36, 8), material),
        # Anti-push pads press the board edges, not the connector centerline.
        box("dc_module_anti_push_pad_left", (50, -58, 27), (12, 8, 18), material),
        box("dc_module_anti_push_pad_right", (50, -12, 27), (12, 8, 18), material),
        box("esp32_module_anti_push_pad_left", (50, 12, 27), (12, 8, 18), material),
        box("esp32_module_anti_push_pad_right", (50, 58, 27), (12, 8, 18), material),
    ]

    for i, (y, z) in enumerate([(-58, 16), (58, 16), (-58, 104), (58, 104)]):
        pad = box(f"cover_screw_pad_{i}", (51, y, z), (12, 18, 18), material)
        diff(pad, cyl(f"cover_screw_clearance_{i}", (51, y, z), 2.2, 16, axis="X"))
        parts.append(pad)
        parts.append(box(f"cover_screw_rib_{i}", (51, y, 9 if z < 58 else 107), (10, 8, 20), material))

    obj = join("smart_cube_shell_v10_back_cover_ports_vents", parts)
    bevel(obj, 0.55, 2)
    return obj


def make_clear_lens(material):
    lens = box("smart_cube_shell_v10_top_clear_lens", (-18, 42, 116), (46, 34, 1.8), material)
    bevel(lens, 0.25, 1)
    return lens


def main():
    clear_scene()
    body_mat = mat("body dark blue gray", (0.11, 0.25, 0.36, 1))
    bezel_mat = mat("front matte black", (0.025, 0.025, 0.03, 1))
    cover_mat = mat("rear warm gray", (0.55, 0.55, 0.5, 1))
    lens_mat = mat("clear lens cyan preview", (0.55, 0.85, 1.0, 0.35))
    lens_mat.use_nodes = False

    body = make_body(body_mat)
    bezel = make_front_bezel(bezel_mat)
    cover = make_back_cover(cover_mat)
    lens = make_clear_lens(lens_mat)

    export_stl(body, "smart_cube_shell_v10_body_air_light_ports.stl")
    export_stl(bezel, "smart_cube_shell_v10_front_screen_clamp_bezel.stl")
    export_stl(cover, "smart_cube_shell_v10_back_cover_ports_vents.stl")
    export_stl(lens, "smart_cube_shell_v10_top_clear_lens.stl")

    body.location.x = 0
    bezel.location.x -= 78
    cover.location.x += 78
    lens.location.x = 0
    bpy.ops.object.light_add(type="AREA", location=(0, -170, 180))
    bpy.context.object.data.energy = 750
    bpy.context.object.data.size = 110
    bpy.ops.object.camera_add(location=(180, -245, 160), rotation=(1.05, 0, 0.67))
    bpy.context.scene.camera = bpy.context.object
    bpy.ops.wm.save_as_mainfile(filepath=str(BASE / "smart_cube_shell_v10_air_light_ports_preview.blend"))


if __name__ == "__main__":
    main()
