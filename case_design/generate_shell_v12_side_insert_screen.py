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


def screw_socket(parts, mat, y, z, idx):
    boss = cyl(f"body_screw_socket_{idx}", (52, y, z), 6.0, 14, axis="X", mat=mat)
    diff(boss, cyl(f"body_screw_pilot_{idx}", (52, y, z), 1.8, 18, axis="X"))
    parts.append(boss)
    parts.append(box(f"body_screw_side_rib_{idx}", (52, y, z), (14, 10, 4), mat))
    parts.append(box(f"body_screw_wall_rib_{idx}", (52, y, 9 if z < 60 else 109), (14, 8, 18), mat))


def make_body(mat):
    # X = front/back, Y = left/right, Z = height. Front is -X. Screen slides from +Y side.
    outer = box("v12_body_outer", (0, 0, 59), (116, 138, 118), mat)
    diff(outer, box("rear_service_opening", (10, 0, 61), (112, 124, 104)))
    diff(outer, box("front_integrated_display_window", (-60, 0, 59), (20, 86, 64)))

    # Large removable-side opening for side-inserting the screen and already-attached Dupont wires.
    diff(outer, box("screen_side_insert_service_opening", (-30, 69, 59), (64, 20, 92)))

    # Continuous screen-wire raceway from the side opening toward the rear/ESP32 area.
    diff(outer, box("screen_wire_side_raceway_open_cut", (-1, 69, 49), (96, 20, 68)))

    for side_y in (-69, 69):
        for z in (74, 82, 90, 98):
            diff(outer, box(f"side_air_louver_{side_y}_{z}", (12, side_y, z), (60, 14, 4)))

    diff(outer, box("top_light_window", (-18, 44, 118), (50, 38, 12)))

    parts = [outer]

    # Integrated front display seat. No separate front bezel is required.
    # Screen module envelope accounts for PCB + rear SD-card socket: about 92W x 74H x 16D.
    parts += [
        box("screen_lower_slide_rail_front_lip", (-52, 0, 18), (12, 112, 4), mat),
        box("screen_upper_slide_rail_front_lip", (-52, 0, 100), (12, 112, 4), mat),
        box("screen_lower_rear_support_rail", (-43, 0, 23), (8, 112, 4), mat),
        box("screen_upper_rear_support_rail", (-43, 0, 95), (8, 112, 4), mat),
        box("screen_left_end_stop", (-48, -58, 59), (12, 4, 78), mat),
        # Corner supports stop the screen from being pressed inward while leaving the SD socket area clear.
        box("screen_back_support_lower_left", (-42, -44, 29), (8, 16, 10), mat),
        box("screen_back_support_upper_left", (-42, -44, 89), (8, 16, 10), mat),
        box("screen_back_support_lower_right", (-42, 36, 29), (8, 16, 10), mat),
        box("screen_back_support_upper_right", (-42, 36, 89), (8, 16, 10), mat),
        # A visible wiring shelf that starts at the screen pin side and continues backward.
        box("screen_wire_channel_floor", (-2, 55, 24), (98, 26, 4), mat),
        box("screen_wire_channel_inner_guard", (-2, 42, 42), (98, 4, 38), mat),
        box("screen_wire_channel_upper_guard", (-2, 42, 86), (98, 4, 14), mat),
    ]

    # Rear-insert lower module trays: no rear stop, still compatible with v11 assembly.
    parts += [
        box("dc_tray_floor", (3, -36, 8), (92, 58, 4), mat),
        box("dc_tray_outer_rail", (3, -66, 20), (92, 4, 24), mat),
        box("dc_tray_inner_rail", (3, -6, 20), (92, 4, 24), mat),
        box("dc_tray_outer_top_lip", (3, -60, 34), (88, 10, 4), mat),
        box("dc_tray_inner_top_lip", (3, -12, 34), (88, 10, 4), mat),
        box("dc_front_stop", (-45, -36, 22), (4, 58, 28), mat),
        box("esp32_tray_floor", (3, 36, 8), (92, 58, 4), mat),
        box("esp32_tray_inner_rail", (3, 6, 20), (92, 4, 24), mat),
        box("esp32_tray_outer_rail", (3, 66, 20), (92, 4, 24), mat),
        box("esp32_tray_inner_top_lip", (3, 12, 34), (88, 10, 4), mat),
        box("esp32_tray_outer_top_lip", (3, 60, 34), (88, 10, 4), mat),
        box("esp32_front_stop", (-45, 36, 22), (4, 58, 28), mat),
    ]

    # Sensor deck.
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
        screw_socket(parts, mat, y, z, idx)

    # Screw receivers for the removable screen-side cover.
    for idx, (x, z) in enumerate([(-54, 20), (-54, 98), (18, 20), (18, 98)]):
        boss = cyl(f"screen_side_cover_receiver_{idx}", (x, 60.5, z), 4.2, 7, axis="Y", mat=mat)
        diff(boss, cyl(f"screen_side_cover_pilot_{idx}", (x, 60.5, z), 1.6, 9, axis="Y"))
        parts.append(boss)
        parts.append(box(f"screen_side_cover_receiver_rib_{idx}", (x, 56, z), (10, 10, 4), mat))

    body = join("smart_cube_shell_v12_body_side_insert_screen", parts)
    bevel(body, 0.6, 2)
    return body


def make_screen_side_cover(mat):
    # Removable side cover: locks the sliding screen in Y and protects the continuous Dupont wire channel.
    parts = [
        box("screen_side_cover_outer_plate", (-18, 72, 59), (80, 6, 94), mat),
        box("screen_side_cover_wire_bulge", (-2, 77, 53), (72, 12, 62), mat),
        box("screen_side_cover_screen_edge_pad_lower", (-52, 64, 29), (8, 8, 14), mat),
        box("screen_side_cover_screen_edge_pad_upper", (-52, 64, 89), (8, 8, 14), mat),
        box("screen_side_cover_rear_wire_guard", (32, 64, 54), (10, 8, 58), mat),
    ]
    cover = join("smart_cube_shell_v12_screen_side_lock_wire_cover", parts)
    # Open the inner part of the bulge so it acts like a wire tunnel, not a solid block.
    diff(cover, box("screen_side_cover_wire_tunnel_hollow", (-2, 73, 53), (62, 12, 48)))
    for idx, (x, z) in enumerate([(-54, 20), (-54, 98), (18, 20), (18, 98)]):
        diff(cover, cyl(f"screen_side_cover_screw_clearance_{idx}", (x, 72, z), 2.1, 12, axis="Y"))
    bevel(cover, 0.45, 2)
    return cover


def make_back_cover(mat):
    cover = box("v12_back_cover_plate", (59, 0, 59), (8, 138, 120), mat)
    diff(cover, box("dc_rect_service_window", (59, -36, 24), (14, 36, 28)))
    diff(cover, box("typec_rect_service_window", (59, 36, 24), (14, 36, 24)))
    for z in (76, 84, 92, 100):
        diff(cover, box(f"rear_air_louver_{z}", (59, 0, z), (14, 104, 4)))

    parts = [cover]
    parts += [
        box("dc_window_bottom_frame", (53, -36, 7), (10, 44, 8), mat),
        box("dc_window_top_frame", (53, -36, 43), (10, 44, 8), mat),
        box("dc_window_left_frame", (53, -60, 25), (10, 8, 34), mat),
        box("dc_window_right_frame", (53, -12, 25), (10, 8, 34), mat),
        box("typec_window_bottom_frame", (53, 36, 8), (10, 44, 8), mat),
        box("typec_window_top_frame", (53, 36, 42), (10, 44, 8), mat),
        box("typec_window_left_frame", (53, 12, 25), (10, 8, 32), mat),
        box("typec_window_right_frame", (53, 60, 25), (10, 8, 32), mat),
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
    obj = join("smart_cube_shell_v12_back_cover_rect_ports_vents", parts)
    bevel(obj, 0.55, 2)
    return obj


def make_clear_lens(mat):
    lens = box("smart_cube_shell_v12_top_clear_lens", (-18, 44, 118), (48, 36, 1.8), mat)
    bevel(lens, 0.25, 1)
    return lens


def main():
    clear_scene()
    body_mat = material("v12 body blue gray", (0.10, 0.24, 0.34, 1))
    side_mat = material("v12 removable side black", (0.03, 0.03, 0.035, 1))
    cover_mat = material("v12 rear gray", (0.55, 0.55, 0.50, 1))
    lens_mat = material("v12 clear lens preview", (0.55, 0.85, 1.0, 0.35))

    body = make_body(body_mat)
    side_cover = make_screen_side_cover(side_mat)
    back_cover = make_back_cover(cover_mat)
    lens = make_clear_lens(lens_mat)

    export_stl(body, "smart_cube_shell_v12_body_side_insert_screen.stl")
    export_stl(side_cover, "smart_cube_shell_v12_screen_side_lock_wire_cover.stl")
    export_stl(back_cover, "smart_cube_shell_v12_back_cover_rect_ports_vents.stl")
    export_stl(lens, "smart_cube_shell_v12_top_clear_lens.stl")

    side_cover.location.y += 24
    back_cover.location.x += 82
    bpy.ops.object.light_add(type="AREA", location=(0, -185, 190))
    bpy.context.object.data.energy = 820
    bpy.context.object.data.size = 120
    bpy.ops.object.camera_add(location=(195, -265, 168), rotation=(1.05, 0, 0.67))
    bpy.context.scene.camera = bpy.context.object
    bpy.ops.wm.save_as_mainfile(filepath=str(BASE / "smart_cube_shell_v12_side_insert_screen_preview.blend"))


if __name__ == "__main__":
    main()
