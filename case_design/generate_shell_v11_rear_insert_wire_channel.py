import math
from pathlib import Path

import bpy

BASE = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def material(name, color):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = color
    return mat


def box(name, loc, scale, mat=None):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if mat:
        obj.data.materials.append(mat)
    return obj


def cyl(name, loc, radius, depth, axis="Z", vertices=48, mat=None):
    rot = (0, 0, 0)
    if axis == "X":
        rot = (0, math.radians(90), 0)
    elif axis == "Y":
        rot = (math.radians(90), 0, 0)
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=loc, rotation=rot)
    obj = bpy.context.object
    obj.name = name
    if mat:
        obj.data.materials.append(mat)
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


def bevel(obj, width=0.55, segments=2):
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


def add_body_screw_socket(parts, mat, y, z, idx):
    boss = cyl(f"body_screw_socket_{idx}", (52, y, z), 6.0, 14, axis="X", mat=mat)
    diff(boss, cyl(f"body_screw_pilot_{idx}", (52, y, z), 1.8, 18, axis="X"))
    parts.append(boss)
    parts.append(box(f"body_screw_side_rib_{idx}", (52, y, z), (14, 10, 4), mat))
    parts.append(box(f"body_screw_wall_rib_{idx}", (52, y, 9 if z < 60 else 109), (14, 8, 18), mat))


def make_body(mat):
    # X = front/back, Y = left/right, Z = height. Front is -X, rear is +X.
    outer = box("v11_body_outer", (0, 0, 59), (116, 136, 118), mat)
    diff(outer, box("rear_service_opening", (10, 0, 61), (112, 122, 104)))
    diff(outer, box("front_screen_install_opening", (-60, 0, 59), (20, 120, 92)))

    # Make the screen cable channel obvious: a side window from the screen edge to the rear service opening.
    diff(outer, box("right_side_screen_wire_window", (-8, 68, 54), (98, 18, 60)))

    # Airflow for ENS160/AHT21 area.
    for side_y in (-68, 68):
        for z in (74, 82, 90, 98):
            diff(outer, box(f"side_air_louver_{side_y}_{z}", (10, side_y, z), (60, 14, 4)))

    # Top BH1750 clear window.
    diff(outer, box("top_light_window", (-18, 44, 118), (50, 38, 12)))

    parts = [outer]

    # Screen mechanical ledges. Right side is deliberately open; wire channel is not a hidden narrow slot.
    parts += [
        box("screen_bottom_ledge", (-52, 0, 12), (12, 120, 4), mat),
        box("screen_top_ledge", (-52, 0, 106), (12, 120, 4), mat),
        box("screen_left_lip", (-52, -60, 59), (12, 4, 94), mat),
        box("screen_right_short_lip_above_wire", (-52, 60, 91), (12, 4, 30), mat),
        box("screen_wire_channel_floor", (-5, 56, 24), (94, 24, 4), mat),
        box("screen_wire_channel_lower_guard", (-5, 44, 31), (94, 4, 18), mat),
        box("screen_wire_channel_upper_guard", (-5, 44, 77), (94, 4, 18), mat),
    ]

    # Rear-insert lower module trays. There is no rear stop/挡板, so modules slide in from the back.
    # DC tray on the left, ESP32-S3 tray on the right.
    parts += [
        box("dc_tray_floor", (3, -36, 8), (92, 58, 4), mat),
        box("dc_tray_outer_rail", (3, -66, 20), (92, 4, 24), mat),
        box("dc_tray_inner_rail", (3, -6, 20), (92, 4, 24), mat),
        box("dc_tray_outer_top_lip", (3, -60, 34), (88, 10, 4), mat),
        box("dc_tray_inner_top_lip", (3, -12, 34), (88, 10, 4), mat),
        box("dc_front_only_stop", (-45, -36, 22), (4, 58, 28), mat),
        box("esp32_tray_floor", (3, 36, 8), (92, 58, 4), mat),
        box("esp32_tray_inner_rail", (3, 6, 20), (92, 4, 24), mat),
        box("esp32_tray_outer_rail", (3, 66, 20), (92, 4, 24), mat),
        box("esp32_tray_inner_top_lip", (3, 12, 34), (88, 10, 4), mat),
        box("esp32_tray_outer_top_lip", (3, 60, 34), (88, 10, 4), mat),
        box("esp32_front_only_stop", (-45, 36, 22), (4, 58, 28), mat),
    ]

    # Sensor deck: separated slots with spacing.
    parts += [
        box("upper_sensor_deck", (-8, 0, 63), (88, 120, 4), mat),
        box("upper_sensor_front_web", (-50, 0, 53), (4, 120, 22), mat),
        box("upper_sensor_rear_web", (36, 0, 53), (4, 120, 22), mat),
        box("sensor_divider_ld_env", (-8, -24, 75), (88, 3, 22), mat),
        box("sensor_divider_env_bh", (-8, 24, 75), (88, 3, 22), mat),
        box("ld2410c_floor", (-18, -44, 68), (44, 32, 4), mat),
        box("ld2410c_left_rail", (-18, -62, 77), (44, 3, 18), mat),
        box("ld2410c_right_rail", (-18, -26, 77), (44, 3, 18), mat),
        box("ens160_aht21_floor", (-18, 0, 68), (48, 36, 4), mat),
        box("ens160_aht21_left_rail", (-18, -20, 77), (48, 3, 18), mat),
        box("ens160_aht21_right_rail", (-18, 20, 77), (48, 3, 18), mat),
        box("bh1750_floor_under_window", (-18, 44, 68), (52, 28, 4), mat),
        box("bh1750_left_rail", (-18, 28, 77), (52, 3, 18), mat),
        box("bh1750_right_rail", (-18, 60, 77), (52, 3, 18), mat),
        box("light_window_front_lip", (-18, 24, 113), (54, 3, 5), mat),
        box("light_window_back_lip", (-18, 64, 113), (54, 3, 5), mat),
        box("light_window_left_lip", (-46, 44, 113), (3, 38, 5), mat),
        box("light_window_right_lip", (10, 44, 113), (3, 38, 5), mat),
    ]

    for idx, (y, z) in enumerate([(-60, 16), (60, 16), (-60, 106), (60, 106)]):
        add_body_screw_socket(parts, mat, y, z, idx)

    body = join("smart_cube_shell_v11_body_rear_insert_wire_channel", parts)
    bevel(body, 0.6, 2)
    return body


def make_front_bezel(mat):
    bezel = box("v11_front_bezel_outer", (-63, 0, 59), (8, 130, 102), mat)
    diff(bezel, box("front_visible_window", (-63, 0, 59), (12, 84, 62)))
    diff(bezel, box("screen_pcb_back_relief", (-59, 0, 59), (6, 112, 86)))
    # Large side exit, visible from the right side, for already-plugged Dupont wires.
    diff(bezel, box("front_bezel_screen_wire_side_exit", (-59, 58, 53), (10, 30, 72)))

    parts = [bezel]
    for y, z in [(-56, 20), (56, 20), (-56, 98), (56, 98)]:
        boss = cyl("front_bezel_screw_boss", (-58, y, z), 4.7, 6, axis="X", mat=mat)
        diff(boss, cyl("front_bezel_screw_hole", (-58, y, z), 1.8, 10, axis="X"))
        parts.append(boss)
    obj = join("smart_cube_shell_v11_front_screen_clamp_bezel", parts)
    bevel(obj, 0.5, 2)
    return obj


def make_back_cover(mat):
    cover = box("v11_back_cover_plate", (59, 0, 59), (8, 136, 118), mat)

    # Both ports are now rectangular service windows for maximum tolerance.
    diff(cover, box("dc_rect_service_window", (59, -36, 24), (14, 36, 28)))
    diff(cover, box("typec_rect_service_window", (59, 36, 24), (14, 36, 24)))

    for z in (76, 84, 92, 100):
        diff(cover, box(f"rear_air_louver_{z}", (59, 0, z), (14, 104, 4)))

    parts = [cover]
    # Reinforcement frames around big rectangular service windows.
    parts += [
        box("dc_window_bottom_frame", (53, -36, 7), (10, 44, 8), mat),
        box("dc_window_top_frame", (53, -36, 43), (10, 44, 8), mat),
        box("dc_window_left_frame", (53, -60, 25), (10, 8, 34), mat),
        box("dc_window_right_frame", (53, -12, 25), (10, 8, 34), mat),
        box("typec_window_bottom_frame", (53, 36, 8), (10, 44, 8), mat),
        box("typec_window_top_frame", (53, 36, 42), (10, 44, 8), mat),
        box("typec_window_left_frame", (53, 12, 25), (10, 8, 32), mat),
        box("typec_window_right_frame", (53, 60, 25), (10, 8, 32), mat),
    ]

    # Edge anti-push pads: they do not block the connector windows and only work after the rear cover is installed.
    parts += [
        box("dc_board_edge_anti_push_left", (51, -64, 27), (12, 6, 18), mat),
        box("dc_board_edge_anti_push_right", (51, -8, 27), (12, 6, 18), mat),
        box("esp32_board_edge_anti_push_left", (51, 8, 27), (12, 6, 18), mat),
        box("esp32_board_edge_anti_push_right", (51, 64, 27), (12, 6, 18), mat),
    ]

    for i, (y, z) in enumerate([(-60, 16), (60, 16), (-60, 106), (60, 106)]):
        pad = box(f"cover_screw_pad_{i}", (53, y, z), (12, 18, 18), mat)
        diff(pad, cyl(f"cover_screw_clearance_{i}", (53, y, z), 2.2, 16, axis="X"))
        parts.append(pad)
        parts.append(box(f"cover_screw_rib_{i}", (53, y, 9 if z < 60 else 109), (10, 8, 20), mat))

    obj = join("smart_cube_shell_v11_back_cover_rect_ports_vents", parts)
    bevel(obj, 0.55, 2)
    return obj


def make_clear_lens(mat):
    lens = box("smart_cube_shell_v11_top_clear_lens", (-18, 44, 118), (48, 36, 1.8), mat)
    bevel(lens, 0.25, 1)
    return lens


def main():
    clear_scene()
    body_mat = material("v11 body blue gray", (0.10, 0.24, 0.34, 1))
    bezel_mat = material("v11 front black", (0.025, 0.025, 0.03, 1))
    cover_mat = material("v11 rear gray", (0.55, 0.55, 0.50, 1))
    lens_mat = material("v11 clear lens preview", (0.55, 0.85, 1.0, 0.35))

    body = make_body(body_mat)
    bezel = make_front_bezel(bezel_mat)
    cover = make_back_cover(cover_mat)
    lens = make_clear_lens(lens_mat)

    export_stl(body, "smart_cube_shell_v11_body_rear_insert_wire_channel.stl")
    export_stl(bezel, "smart_cube_shell_v11_front_screen_clamp_bezel.stl")
    export_stl(cover, "smart_cube_shell_v11_back_cover_rect_ports_vents.stl")
    export_stl(lens, "smart_cube_shell_v11_top_clear_lens.stl")

    bezel.location.x -= 82
    cover.location.x += 82
    bpy.ops.object.light_add(type="AREA", location=(0, -180, 185))
    bpy.context.object.data.energy = 800
    bpy.context.object.data.size = 115
    bpy.ops.object.camera_add(location=(190, -255, 165), rotation=(1.05, 0, 0.67))
    bpy.context.scene.camera = bpy.context.object
    bpy.ops.wm.save_as_mainfile(filepath=str(BASE / "smart_cube_shell_v11_rear_insert_wire_channel_preview.blend"))


if __name__ == "__main__":
    main()
