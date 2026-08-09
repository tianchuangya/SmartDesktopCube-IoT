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


def bevel(obj, width=0.45, segments=2):
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
    boss = cyl(f"body_back_screw_socket_{idx}", (52, y, z), 5.6, 12, axis="X", mat=mat)
    diff(boss, cyl(f"body_back_screw_pilot_{idx}", (52, y, z), 1.8, 16, axis="X"))
    parts.append(boss)
    parts.append(box(f"body_back_screw_rib_{idx}", (52, y, 9 if z < 60 else 109), (12, 8, 18), mat))


def add_front_screw_receiver(parts, mat, y, z, idx):
    boss = cyl(f"front_bezel_receiver_{idx}", (-55, y, z), 4.6, 8, axis="X", mat=mat)
    diff(boss, cyl(f"front_bezel_receiver_pilot_{idx}", (-55, y, z), 1.7, 10, axis="X"))
    parts.append(boss)
    parts.append(box(f"front_receiver_rib_{idx}", (-50, y, z), (12, 7, 4), mat))


def make_body(mat):
    # X = front/back, Y = left/right, Z = height.
    outer = box("v15_body_outer", (0, 0, 59), (116, 138, 118), mat)
    diff(outer, box("rear_open_service_cavity", (10, 0, 61), (112, 124, 104)))

    # Plain front: a deep rectangular screen pocket and a simple rectangular viewing opening.
    diff(outer, box("front_screen_deep_pocket", (-54, 0, 59), (22, 106, 84)))
    diff(outer, box("front_screen_view_window", (-62, 0, 59), (20, 84, 62)))
    diff(outer, box("right_screen_insert_mouth", (-49, 58, 59), (20, 30, 84)))

    # Simple right-side wire slot, covered by a small sliding strip.
    diff(outer, box("right_wire_slot", (-4, 69, 52), (96, 18, 56)))
    # Two straight grooves for the side sliding strip. This follows the original v1 long-rail idea.
    diff(outer, box("side_slider_upper_groove", (-4, 69, 86), (96, 8, 6)))
    diff(outer, box("side_slider_lower_groove", (-4, 69, 24), (96, 8, 6)))
    for z in (94, 104):
        diff(outer, box(f"right_side_upper_vent_{z}", (12, 69, z), (54, 12, 4)))
    for z in (96, 106):
        diff(outer, box(f"left_real_top_side_vent_{z}", (12, -65.5, z), (58, 7, 5)))
        diff(outer, box(f"right_real_top_side_vent_{z}", (12, 65.5, z), (58, 7, 5)))

    # Real screw passages through the front/rear wall before adding receiver bosses.
    # The bosses are also hollow, so screws do not hit a solid wall.
    for y, z in [(-60, 16), (60, 16), (-60, 106), (60, 106)]:
        diff(outer, cyl(f"front_wall_screw_passage_{y}_{z}", (-55, y, z), 1.9, 14, axis="X"))
        diff(outer, cyl(f"rear_wall_screw_passage_{y}_{z}", (52, y, z), 1.9, 16, axis="X"))

    # Keep side airflow and top light window, but avoid making the rear cover visually complex.
    for side_y in (-69,):
        for z in (76, 86, 96):
            diff(outer, box(f"left_side_air_louver_{z}", (12, side_y, z), (54, 12, 4)))
    diff(outer, box("top_light_window", (-18, 44, 118), (50, 38, 12)))

    parts = [outer]

    # Screen pocket supports: screen sits deeper, front bezel holds it from the front.
    parts += [
        box("screen_pocket_bottom_shelf", (-49, 0, 22), (10, 104, 5), mat),
        box("screen_pocket_top_shelf", (-49, 0, 96), (10, 104, 5), mat),
        box("screen_pocket_left_stop", (-49, -52, 59), (10, 5, 78), mat),
        box("screen_pocket_right_wire_clearance_edge", (-49, 52, 59), (10, 5, 78), mat),
        box("screen_back_edge_support_lower", (-42, 0, 30), (5, 96, 7), mat),
        box("screen_back_edge_support_upper", (-42, 0, 88), (5, 96, 7), mat),
        # Small lips reduce the visible corner gap near the slider side without closing the screen entry.
        box("front_slider_side_gap_cover_upper", (-56, 53, 91), (6, 8, 18), mat),
        box("front_slider_side_gap_cover_lower", (-56, 53, 27), (6, 8, 18), mat),
    ]

    # Front bezel screw receivers.
    for idx, (y, z) in enumerate([(-60, 16), (60, 16), (-60, 106), (60, 106)]):
        add_front_screw_receiver(parts, mat, y, z, idx)

    # Internal module layout kept simple and close to previous versions.
    parts += [
        # DC tray.
        box("dc_tray_floor", (3, -36, 8), (92, 58, 4), mat),
        box("dc_tray_outer_rail", (3, -66, 20), (92, 4, 24), mat),
        box("dc_tray_inner_rail_front", (-28, -6, 20), (28, 4, 24), mat),
        box("dc_tray_inner_rail_rear", (30, -6, 20), (36, 4, 24), mat),
        box("dc_tray_outer_top_lip", (3, -59, 34), (90, 12, 5), mat),
        box("dc_tray_inner_top_lip_front", (-28, -13, 34), (28, 12, 5), mat),
        box("dc_tray_inner_top_lip_rear", (30, -13, 34), (36, 12, 5), mat),
        box("dc_front_stop", (-45, -36, 22), (4, 58, 28), mat),
        # ESP32-S3 tray.
        box("esp32_tray_floor", (3, 36, 8), (92, 58, 4), mat),
        box("esp32_tray_inner_rail_front", (-28, 6, 20), (28, 4, 24), mat),
        box("esp32_tray_inner_rail_rear", (30, 6, 20), (36, 4, 24), mat),
        box("esp32_tray_outer_rail", (3, 66, 20), (92, 4, 24), mat),
        box("esp32_tray_inner_top_lip_front", (-28, 13, 34), (28, 12, 5), mat),
        box("esp32_tray_inner_top_lip_rear", (30, 13, 34), (36, 12, 5), mat),
        box("esp32_tray_outer_top_lip", (3, 59, 34), (90, 12, 5), mat),
        box("esp32_front_stop", (-45, 36, 22), (4, 58, 28), mat),
        # Sensor deck.
        # Split the middle shelf/webs and leave a central wire window between lower boards and upper modules.
        box("upper_sensor_deck_left", (-8, -43, 63), (88, 34, 4), mat),
        box("upper_sensor_deck_right", (-8, 43, 63), (88, 34, 4), mat),
        box("upper_sensor_deck_front_bridge", (-38, 0, 63), (28, 44, 4), mat),
        box("upper_sensor_deck_rear_bridge", (24, 0, 63), (24, 44, 4), mat),
        box("upper_sensor_front_web_left", (-50, -43, 53), (4, 34, 22), mat),
        box("upper_sensor_front_web_right", (-50, 43, 53), (4, 34, 22), mat),
        box("upper_sensor_rear_web_left", (36, -43, 53), (4, 34, 22), mat),
        box("upper_sensor_rear_web_right", (36, 43, 53), (4, 34, 22), mat),
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

    obj = join("smart_cube_shell_v21_body_real_screw_holes", parts)
    bevel(obj, 0.55, 2)
    return obj


def make_front_bezel(mat):
    bezel = box("v21_front_bezel_full_face_outer", (-64, 0, 59), (5, 138, 118), mat)
    diff(bezel, box("v15_front_bezel_view_window", (-64, 0, 59), (8, 84, 62)))
    # Inner shallow relief: the bezel is a simple pressure frame, not a decorative shape.
    diff(bezel, box("v15_front_bezel_inner_relief", (-61, 0, 59), (4, 102, 78)))
    for idx, (y, z) in enumerate([(-60, 16), (60, 16), (-60, 106), (60, 106)]):
        diff(bezel, cyl(f"front_bezel_screw_clearance_{idx}", (-64, y, z), 2.4, 10, axis="X"))
    bevel(bezel, 0.35, 2)
    return bezel


def make_side_slider(mat):
    # One simple strip with two tongues. It slides through the straight grooves on the right side.
    parts = [
        box("v16_side_slider_flat_strip", (-4, 73, 55), (94, 5, 64), mat),
        box("v16_side_slider_upper_tongue", (-4, 68, 86), (90, 5, 4), mat),
        box("v16_side_slider_lower_tongue", (-4, 68, 24), (90, 5, 4), mat),
    ]
    slider = join("smart_cube_shell_v21_side_simple_slider_vents", parts)
    for z in (62, 74):
        diff(slider, box(f"side_slider_vent_slot_{z}", (-4, 73, z), (62, 12, 4)))
    bevel(slider, 0.3, 1)
    return slider


def make_back_cover(mat):
    cover = box("v21_back_cover_simple_two_ports_vents", (59, 0, 59), (8, 138, 120), mat)
    # Back is intentionally simple: one DC barrel hole and one Type-C rectangular hole.
    diff(cover, cyl("dc_barrel_hole", (59, -36, 24), 6.0, 14, axis="X"))
    diff(cover, box("typec_hole", (59, 36, 24), (14, 24, 14)))
    for z in (68, 78, 88, 98):
        diff(cover, box(f"rear_cover_vent_slot_{z}", (59, 0, z), (14, 96, 4)))
    for i, (y, z) in enumerate([(-60, 16), (60, 16), (-60, 106), (60, 106)]):
        diff(cover, cyl(f"back_cover_screw_clearance_{i}", (59, y, z), 2.4, 14, axis="X"))
    bevel(cover, 0.45, 2)
    return cover


def make_clear_lens(mat):
    lens = box("smart_cube_shell_v21_top_clear_lens", (-18, 44, 118), (48, 36, 1.8), mat)
    bevel(lens, 0.25, 1)
    return lens


def main():
    clear_scene()
    body_mat = material("v21 body blue gray", (0.10, 0.24, 0.34, 1))
    bezel_mat = material("v21 front bezel black", (0.03, 0.03, 0.035, 1))
    side_mat = material("v21 side slider black", (0.03, 0.03, 0.035, 1))
    cover_mat = material("v21 rear gray", (0.55, 0.55, 0.50, 1))
    lens_mat = material("v21 clear lens preview", (0.55, 0.85, 1.0, 0.35))

    body = make_body(body_mat)
    front = make_front_bezel(bezel_mat)
    side = make_side_slider(side_mat)
    back = make_back_cover(cover_mat)
    lens = make_clear_lens(lens_mat)

    export_stl(body, "smart_cube_shell_v21_body_real_screw_holes.stl")
    export_stl(front, "smart_cube_shell_v21_front_bezel_real_holes.stl")
    export_stl(side, "smart_cube_shell_v21_side_simple_slider_vents.stl")
    export_stl(back, "smart_cube_shell_v21_back_cover_real_holes.stl")
    export_stl(lens, "smart_cube_shell_v21_top_clear_lens.stl")

    # Preview: pull front, side slider, and back cover away from the body.
    front.location.x -= 24
    side.location.y += 22
    back.location.x += 82
    bpy.ops.object.light_add(type="AREA", location=(0, -185, 190))
    bpy.context.object.data.energy = 820
    bpy.context.object.data.size = 120
    bpy.ops.object.camera_add(location=(195, -265, 168), rotation=(1.05, 0, 0.67))
    bpy.context.scene.camera = bpy.context.object
    bpy.ops.wm.save_as_mainfile(filepath=str(BASE / "smart_cube_shell_v21_real_screw_holes_preview.blend"))


if __name__ == "__main__":
    main()
