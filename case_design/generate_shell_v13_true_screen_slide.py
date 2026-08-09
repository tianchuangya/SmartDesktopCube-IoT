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


def body_screw_socket(parts, mat, y, z, idx):
    boss = cyl(f"body_screw_socket_{idx}", (52, y, z), 6.0, 14, axis="X", mat=mat)
    diff(boss, cyl(f"body_screw_pilot_{idx}", (52, y, z), 1.8, 18, axis="X"))
    parts.append(boss)
    parts.append(box(f"body_screw_side_rib_{idx}", (52, y, z), (14, 10, 4), mat))
    parts.append(box(f"body_screw_wall_rib_{idx}", (52, y, 9 if z < 60 else 109), (14, 8, 18), mat))


def make_body(mat):
    # X = front/back, Y = left/right, Z = height. Screen tray slides along Y from +Y.
    outer = box("v13_body_flat_front_outer", (0, 0, 59), (116, 140, 118), mat)
    diff(outer, box("rear_service_opening", (10, 0, 61), (112, 126, 104)))
    # The front is now intentionally plain: one clean rectangular display window only.
    diff(outer, box("front_clean_rect_display_window", (-60, 0, 59), (20, 88, 66)))
    # Large side slot for a whole screen slide tray, not just wire clearance.
    diff(outer, box("right_screen_tray_insert_slot", (-48, 70, 59), (34, 24, 92)))
    # Extra passage for Dupont wires after the tray is inserted.
    diff(outer, box("right_wire_escape_after_tray", (-5, 70, 49), (90, 24, 68)))

    for side_y in (-70, 70):
        for z in (74, 82, 90, 98):
            diff(outer, box(f"side_air_louver_{side_y}_{z}", (12, side_y, z), (60, 14, 4)))
    diff(outer, box("top_light_window", (-18, 44, 118), (50, 38, 12)))

    parts = [outer]

    # Continuous, visible screen slide rails. These are not small support blocks.
    # Tray envelope: X 12 mm thick, Y 108 mm sliding length, Z 84 mm height.
    parts += [
        box("screen_slide_lower_channel_floor", (-48, 4, 19), (18, 116, 4), mat),
        box("screen_slide_lower_channel_outer_wall", (-48, 4, 25), (18, 116, 4), mat),
        box("screen_slide_upper_channel_roof", (-48, 4, 99), (18, 116, 4), mat),
        box("screen_slide_upper_channel_outer_wall", (-48, 4, 93), (18, 116, 4), mat),
        box("screen_slide_rear_backing_rail", (-39, 4, 59), (4, 116, 84), mat),
        box("screen_slide_left_end_stop", (-48, -57, 59), (18, 5, 84), mat),
        # Narrow front retaining lips around the display window to stop inward movement when pressing.
        box("front_window_lower_retaining_lip", (-56, 0, 24), (6, 96, 5), mat),
        box("front_window_upper_retaining_lip", (-56, 0, 94), (6, 96, 5), mat),
        box("front_window_left_retaining_lip", (-56, -48, 59), (6, 5, 70), mat),
        box("front_window_right_retaining_lip", (-56, 48, 59), (6, 5, 70), mat),
        # Wire raceway is a clear open trough leading to the ESP32 area.
        box("screen_wire_trough_floor", (-2, 56, 24), (98, 24, 4), mat),
        box("screen_wire_trough_inner_wall", (-2, 44, 52), (98, 4, 60), mat),
    ]

    # Rear-insert lower module trays.
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

    # Top sensor deck.
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
        body_screw_socket(parts, mat, y, z, idx)

    # Side tray lock screw receivers.
    for idx, (x, z) in enumerate([(-58, 19), (-58, 99), (12, 19), (12, 99)]):
        boss = cyl(f"screen_tray_side_lock_receiver_{idx}", (x, 61.5, z), 4.0, 7, axis="Y", mat=mat)
        diff(boss, cyl(f"screen_tray_side_lock_pilot_{idx}", (x, 61.5, z), 1.6, 9, axis="Y"))
        parts.append(boss)
        parts.append(box(f"screen_tray_side_lock_rib_{idx}", (x, 57, z), (10, 10, 4), mat))

    obj = join("smart_cube_shell_v13_body_true_screen_slide", parts)
    bevel(obj, 0.55, 2)
    return obj


def make_screen_slide_tray(mat):
    # A real one-piece sliding tray. The display module mounts to this tray, then the tray slides in.
    parts = [
        box("screen_slide_tray_left_frame", (-47, -45, 59), (8, 6, 82), mat),
        box("screen_slide_tray_right_handle_frame", (-47, 57, 59), (8, 10, 82), mat),
        box("screen_slide_tray_top_frame", (-47, 6, 99), (8, 108, 6), mat),
        box("screen_slide_tray_bottom_frame", (-47, 6, 19), (8, 108, 6), mat),
        # Rear carrier plate around the screen edge. Middle is cut out for display and SD-card bulge.
        box("screen_slide_tray_rear_edge_lower", (-42, 6, 29), (4, 108, 10), mat),
        box("screen_slide_tray_rear_edge_upper", (-42, 6, 89), (4, 108, 10), mat),
        box("screen_slide_tray_rear_edge_left", (-42, -45, 59), (4, 8, 70), mat),
        box("screen_slide_tray_rear_edge_right", (-42, 51, 59), (4, 8, 70), mat),
        # Slide tongues that engage the body channels.
        box("screen_slide_tray_lower_tongue", (-48, 6, 24), (14, 108, 4), mat),
        box("screen_slide_tray_upper_tongue", (-48, 6, 94), (14, 108, 4), mat),
        # Right-side handle/lock face; also covers the side insertion slot.
        box("screen_slide_tray_side_cover_face", (-24, 71, 59), (62, 7, 90), mat),
        box("screen_slide_tray_wire_bulge", (-2, 77, 52), (72, 12, 60), mat),
    ]
    tray = join("smart_cube_shell_v13_screen_slide_tray_with_wire_cover", parts)
    # Clear window area and SD-card/socket relief, so the tray is not a solid plate behind the screen.
    diff(tray, box("tray_front_display_open_area", (-47, 6, 59), (14, 82, 58)))
    diff(tray, box("tray_sd_card_back_relief", (-40, 2, 59), (10, 46, 36)))
    diff(tray, box("tray_wire_bulge_hollow", (-2, 75, 52), (62, 12, 46)))
    for idx, (x, z) in enumerate([(-58, 19), (-58, 99), (12, 19), (12, 99)]):
        diff(tray, cyl(f"tray_lock_clearance_{idx}", (x, 71, z), 2.1, 12, axis="Y"))
    bevel(tray, 0.45, 2)
    return tray


def make_back_cover(mat):
    cover = box("v13_back_cover_plate", (59, 0, 59), (8, 138, 120), mat)
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
    obj = join("smart_cube_shell_v13_back_cover_rect_ports_vents", parts)
    bevel(obj, 0.55, 2)
    return obj


def make_clear_lens(mat):
    lens = box("smart_cube_shell_v13_top_clear_lens", (-18, 44, 118), (48, 36, 1.8), mat)
    bevel(lens, 0.25, 1)
    return lens


def main():
    clear_scene()
    body_mat = material("v13 body blue gray", (0.10, 0.24, 0.34, 1))
    tray_mat = material("v13 screen slide tray black", (0.025, 0.025, 0.03, 1))
    cover_mat = material("v13 rear gray", (0.55, 0.55, 0.50, 1))
    lens_mat = material("v13 clear lens preview", (0.55, 0.85, 1.0, 0.35))

    body = make_body(body_mat)
    tray = make_screen_slide_tray(tray_mat)
    back = make_back_cover(cover_mat)
    lens = make_clear_lens(lens_mat)

    export_stl(body, "smart_cube_shell_v13_body_true_screen_slide.stl")
    export_stl(tray, "smart_cube_shell_v13_screen_slide_tray_with_wire_cover.stl")
    export_stl(back, "smart_cube_shell_v13_back_cover_rect_ports_vents.stl")
    export_stl(lens, "smart_cube_shell_v13_top_clear_lens.stl")

    # Preview: pull tray partly outward so the slide-block relationship is visible.
    tray.location.y += 32
    back.location.x += 84
    bpy.ops.object.light_add(type="AREA", location=(0, -190, 190))
    bpy.context.object.data.energy = 850
    bpy.context.object.data.size = 120
    bpy.ops.object.camera_add(location=(200, -270, 170), rotation=(1.05, 0, 0.67))
    bpy.context.scene.camera = bpy.context.object
    bpy.ops.wm.save_as_mainfile(filepath=str(BASE / "smart_cube_shell_v13_true_screen_slide_preview.blend"))


if __name__ == "__main__":
    main()
