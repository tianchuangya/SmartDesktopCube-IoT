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


def cyl(name, loc, radius, depth, axis="Z", vertices=64, mat=None):
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


def bevel(obj, width=0.5, segments=2):
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
    # X = front/back, Y = left/right, Z = height. Front is -X, screen inserts from +Y.
    outer = box("v14_body_outer", (0, 0, 59), (116, 138, 118), mat)
    diff(outer, box("rear_service_opening", (10, 0, 61), (112, 124, 104)))

    # Front is flat. Only a deeper rectangular recessed screen pocket plus a rectangular visible window.
    diff(outer, box("front_deep_screen_recess", (-59, 0, 59), (9, 104, 82)))
    diff(outer, box("front_rect_display_window", (-61, 0, 59), (18, 84, 62)))

    # Right-side screen service opening. It is only for sliding the screen and routing Dupont wires.
    diff(outer, box("right_screen_insert_opening", (-32, 69, 59), (66, 20, 92)))
    diff(outer, box("right_wire_raceway_opening", (-2, 69, 50), (96, 20, 66)))

    for side_y in (-69, 69):
        for z in (74, 82, 90, 98):
            diff(outer, box(f"side_air_louver_{side_y}_{z}", (12, side_y, z), (60, 14, 4)))
    diff(outer, box("top_light_window", (-18, 44, 118), (50, 38, 12)))

    parts = [outer]

    # Continuous internal screen insertion rails. The screen edge sits deeper behind the front surface.
    parts += [
        box("screen_lower_continuous_slide_rail", (-48, 0, 20), (14, 114, 5), mat),
        box("screen_upper_continuous_slide_rail", (-48, 0, 98), (14, 114, 5), mat),
        box("screen_rear_continuous_support_rail", (-41, 0, 59), (4, 114, 82), mat),
        box("screen_left_end_stop", (-48, -58, 59), (14, 5, 82), mat),
        box("front_recess_lower_pressure_lip", (-55, 0, 28), (5, 96, 5), mat),
        box("front_recess_upper_pressure_lip", (-55, 0, 90), (5, 96, 5), mat),
        box("front_recess_left_pressure_lip", (-55, -48, 59), (5, 5, 66), mat),
        box("front_recess_right_pressure_lip", (-55, 48, 59), (5, 5, 66), mat),
        # Side wire trough remains visible and large enough for Dupont wires.
        box("screen_wire_trough_floor", (-2, 55, 24), (98, 26, 4), mat),
        box("screen_wire_trough_inner_wall", (-2, 42, 52), (98, 4, 58), mat),
    ]

    # Rear-insert DC and ESP32 trays. The upper retaining lips are made continuous and wider.
    parts += [
        box("dc_tray_floor", (3, -36, 8), (92, 58, 4), mat),
        box("dc_tray_outer_rail", (3, -66, 20), (92, 4, 24), mat),
        box("dc_tray_inner_rail", (3, -6, 20), (92, 4, 24), mat),
        box("dc_tray_outer_wide_top_lip", (3, -59, 34), (90, 12, 5), mat),
        box("dc_tray_inner_wide_top_lip", (3, -13, 34), (90, 12, 5), mat),
        box("dc_front_stop", (-45, -36, 22), (4, 58, 28), mat),
        box("esp32_tray_floor", (3, 36, 8), (92, 58, 4), mat),
        box("esp32_tray_inner_rail", (3, 6, 20), (92, 4, 24), mat),
        box("esp32_tray_outer_rail", (3, 66, 20), (92, 4, 24), mat),
        box("esp32_tray_inner_wide_top_lip", (3, 13, 34), (90, 12, 5), mat),
        box("esp32_tray_outer_wide_top_lip", (3, 59, 34), (90, 12, 5), mat),
        box("esp32_front_stop", (-45, 36, 22), (4, 58, 28), mat),
    ]

    # Top sensor deck kept from v12 baseline.
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

    for idx, (x, z) in enumerate([(-54, 20), (-54, 98), (18, 20), (18, 98)]):
        boss = cyl(f"screen_side_cover_receiver_{idx}", (x, 60.5, z), 4.2, 7, axis="Y", mat=mat)
        diff(boss, cyl(f"screen_side_cover_pilot_{idx}", (x, 60.5, z), 1.6, 9, axis="Y"))
        parts.append(boss)
        parts.append(box(f"screen_side_cover_receiver_rib_{idx}", (x, 56, z), (10, 10, 4), mat))

    obj = join("smart_cube_shell_v14_body_refined_v12", parts)
    bevel(obj, 0.6, 2)
    return obj


def make_screen_side_cover(mat):
    # One complete side cover: full plate + connected outer bulge. No separated block appearance.
    parts = [
        box("v14_side_cover_full_plate", (-18, 72, 59), (94, 8, 98), mat),
        box("v14_side_cover_connected_wire_bulge", (-2, 79, 54), (78, 12, 66), mat),
        box("v14_side_cover_top_bridge", (-18, 75, 101), (94, 12, 8), mat),
        box("v14_side_cover_bottom_bridge", (-18, 75, 17), (94, 12, 8), mat),
    ]
    cover = join("smart_cube_shell_v14_screen_side_cover_one_piece", parts)
    # Inner relief keeps wire space without cutting the cover into independent cubes.
    diff(cover, box("v14_side_cover_inner_wire_relief", (-2, 70, 54), (66, 8, 50)))
    for idx, (x, z) in enumerate([(-54, 20), (-54, 98), (18, 20), (18, 98)]):
        diff(cover, cyl(f"side_cover_screw_clearance_{idx}", (x, 72, z), 2.1, 12, axis="Y"))
    bevel(cover, 0.45, 2)
    return cover


def make_back_cover(mat):
    cover = box("v14_back_cover_plate", (59, 0, 59), (8, 138, 120), mat)
    # DC is now a smaller cylindrical opening with a small flat relief, not a huge rectangle.
    diff(cover, cyl("dc_small_round_port", (59, -36, 24), 6.0, 14, axis="X"))
    diff(cover, box("dc_small_key_relief", (59, -36, 17), (14, 14, 7)))
    # Type-C remains rectangular but reduced compared with earlier broad service windows.
    diff(cover, box("typec_reduced_rect_port", (59, 36, 24), (14, 24, 14)))
    for z in (76, 84, 92, 100):
        diff(cover, box(f"rear_air_louver_{z}", (59, 0, z), (14, 104, 4)))

    parts = [cover]
    parts += [
        box("dc_round_port_reinforcement", (53, -36, 24), (10, 28, 28), mat),
        box("typec_port_reinforcement", (53, 36, 24), (10, 34, 26), mat),
        box("dc_board_edge_anti_push_left", (51, -64, 27), (12, 6, 18), mat),
        box("dc_board_edge_anti_push_right", (51, -8, 27), (12, 6, 18), mat),
        box("esp32_board_edge_anti_push_left", (51, 8, 27), (12, 6, 18), mat),
        box("esp32_board_edge_anti_push_right", (51, 64, 27), (12, 6, 18), mat),
    ]
    # Re-cut ports through reinforcement blocks as well.
    joined = join("smart_cube_shell_v14_back_cover_small_dc_port_precut", parts)
    diff(joined, cyl("dc_round_port_recutter", (59, -36, 24), 6.0, 18, axis="X"))
    diff(joined, box("dc_key_relief_recutter", (59, -36, 17), (18, 14, 7)))
    diff(joined, box("typec_port_recutter", (59, 36, 24), (18, 24, 14)))

    more = [joined]
    for i, (y, z) in enumerate([(-60, 16), (60, 16), (-60, 106), (60, 106)]):
        pad = box(f"cover_screw_pad_{i}", (53, y, z), (12, 18, 18), mat)
        diff(pad, cyl(f"cover_screw_clearance_{i}", (53, y, z), 2.2, 16, axis="X"))
        more.append(pad)
        more.append(box(f"cover_screw_rib_{i}", (53, y, 9 if z < 60 else 109), (10, 8, 20), mat))
    obj = join("smart_cube_shell_v14_back_cover_small_dc_port", more)
    bevel(obj, 0.55, 2)
    return obj


def make_clear_lens(mat):
    lens = box("smart_cube_shell_v14_top_clear_lens", (-18, 44, 118), (48, 36, 1.8), mat)
    bevel(lens, 0.25, 1)
    return lens


def main():
    clear_scene()
    body_mat = material("v14 body blue gray", (0.10, 0.24, 0.34, 1))
    side_mat = material("v14 side cover black", (0.03, 0.03, 0.035, 1))
    cover_mat = material("v14 rear gray", (0.55, 0.55, 0.50, 1))
    lens_mat = material("v14 clear lens preview", (0.55, 0.85, 1.0, 0.35))

    body = make_body(body_mat)
    side = make_screen_side_cover(side_mat)
    back = make_back_cover(cover_mat)
    lens = make_clear_lens(lens_mat)

    export_stl(body, "smart_cube_shell_v14_body_refined_v12.stl")
    export_stl(side, "smart_cube_shell_v14_screen_side_cover_one_piece.stl")
    export_stl(back, "smart_cube_shell_v14_back_cover_small_dc_port.stl")
    export_stl(lens, "smart_cube_shell_v14_top_clear_lens.stl")

    side.location.y += 24
    back.location.x += 82
    bpy.ops.object.light_add(type="AREA", location=(0, -185, 190))
    bpy.context.object.data.energy = 820
    bpy.context.object.data.size = 120
    bpy.ops.object.camera_add(location=(195, -265, 168), rotation=(1.05, 0, 0.67))
    bpy.context.scene.camera = bpy.context.object
    bpy.ops.wm.save_as_mainfile(filepath=str(BASE / "smart_cube_shell_v14_refined_v12_preview.blend"))


if __name__ == "__main__":
    main()
