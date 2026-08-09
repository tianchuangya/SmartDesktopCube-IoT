import math
from pathlib import Path

import bpy

BASE = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete()


def cube_obj(name, loc, scale, mat=None):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if mat:
        obj.data.materials.append(mat)
    return obj


def cyl_obj(name, loc, radius, depth, axis="Z", vertices=48, mat=None):
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


def bool_diff(target, cutter):
    bpy.context.view_layer.objects.active = target
    target.select_set(True)
    mod = target.modifiers.new("cut_" + cutter.name, "BOOLEAN")
    mod.operation = "DIFFERENCE"
    mod.object = cutter
    mod.solver = "EXACT"
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.data.objects.remove(cutter, do_unlink=True)


def join_as(name, objects, mat=None):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = name
    if mat and not obj.data.materials:
        obj.data.materials.append(mat)
    return obj


def add_bevel(obj, amount=0.8, segments=2):
    bpy.context.view_layer.objects.active = obj
    mod = obj.modifiers.new("small_edge_bevel", "BEVEL")
    mod.width = amount
    mod.segments = segments
    mod.affect = "EDGES"
    bpy.ops.object.modifier_apply(modifier=mod.name)
    mod = obj.modifiers.new("weighted_normals", "WEIGHTED_NORMAL")
    bpy.ops.object.modifier_apply(modifier=mod.name)


def export_obj(obj, filename):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.wm.stl_export(filepath=str(BASE / filename), export_selected_objects=True)


def material(name, color):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = color
    return mat


def make_body(mat):
    # Axes: X depth/front-back, Y width, Z height. Front is X negative, rear is X positive.
    body = cube_obj("v9_body_outer", (0, 0, 56), (104, 126, 112), mat)
    bool_diff(body, cube_obj("rear_open_cavity", (8, 0, 58), (96, 112, 98)))
    bool_diff(body, cube_obj("front_screen_install_opening", (-54, 0, 56), (14, 108, 80)))

    parts = [body]

    # Front screen support ledges: all tied into the front frame, leaving a large side relief for pin headers.
    parts += [
        cube_obj("screen_bottom_ledge", (-48, 0, 15), (12, 112, 4), mat),
        cube_obj("screen_top_ledge", (-48, 0, 97), (12, 112, 4), mat),
        cube_obj("screen_left_side_lip", (-48, -56, 56), (12, 4, 82), mat),
        cube_obj("screen_right_cable_relief_wall", (-48, 56, 56), (12, 4, 82), mat),
        cube_obj("screen_pin_wire_tunnel_floor", (-38, 45, 18), (28, 24, 4), mat),
        cube_obj("screen_pin_wire_tunnel_guard", (-38, 57, 44), (28, 4, 56), mat),
    ]

    # ESP32-S3 envelope tray: 78 x 46 x 24 mm, for board + pin headers + Dupont plugs.
    parts += [
        cube_obj("esp32_floor", (4, -34, 8), (86, 50, 4), mat),
        cube_obj("esp32_left_rail", (4, -60, 18), (86, 4, 20), mat),
        cube_obj("esp32_right_rail", (4, -8, 18), (86, 4, 20), mat),
        cube_obj("esp32_left_top_lip", (4, -55, 29), (82, 8, 4), mat),
        cube_obj("esp32_right_top_lip", (4, -13, 29), (82, 8, 4), mat),
        cube_obj("esp32_front_stop", (-42, -34, 20), (4, 50, 24), mat),
        cube_obj("esp32_port_side_abutment", (45, -34, 17), (5, 44, 18), mat),
    ]

    # DC power module tray: document says PCB 59.5 x 26.5 mm; tray allows pins, plug housing, and wires.
    parts += [
        cube_obj("dc_floor", (8, 32, 8), (80, 52, 4), mat),
        cube_obj("dc_left_rail", (8, 5, 18), (80, 4, 20), mat),
        cube_obj("dc_right_rail", (8, 59, 18), (80, 4, 20), mat),
        cube_obj("dc_left_top_lip", (8, 10, 30), (76, 8, 4), mat),
        cube_obj("dc_right_top_lip", (8, 54, 30), (76, 8, 4), mat),
        cube_obj("dc_front_stop", (-35, 32, 20), (4, 52, 24), mat),
        cube_obj("dc_port_side_abutment", (45, 32, 18), (5, 44, 20), mat),
    ]

    # Upper sensor area, tied to the side wall and rear-accessible for wiring.
    parts += [
        cube_obj("upper_sensor_shelf", (-3, 0, 55), (74, 112, 4), mat),
        cube_obj("upper_shelf_front_web", (-40, 0, 45), (4, 112, 20), mat),
        cube_obj("upper_shelf_right_web", (-3, 56, 45), (74, 4, 20), mat),
        cube_obj("ld2410c_card_slot_floor", (-20, -36, 61), (36, 30, 4), mat),
        cube_obj("ld2410c_slot_left", (-20, -53, 70), (36, 3, 18), mat),
        cube_obj("ld2410c_slot_right", (-20, -19, 70), (36, 3, 18), mat),
        cube_obj("env_combo_slot_floor", (-20, 16, 61), (42, 36, 4), mat),
        cube_obj("env_combo_slot_left", (-20, -4, 70), (42, 3, 18), mat),
        cube_obj("env_combo_slot_right", (-20, 36, 70), (42, 3, 18), mat),
        cube_obj("bh1750_slot_floor", (23, 18, 61), (46, 26, 4), mat),
        cube_obj("bh1750_slot_left", (23, 3, 70), (46, 3, 18), mat),
        cube_obj("bh1750_slot_right", (23, 33, 70), (46, 3, 18), mat),
    ]

    # Rear screw bosses on the body: horizontal sockets connected to side/top/bottom walls by ribs.
    screw_points = [(-54, 16), (54, 16), (-54, 96), (54, 96)]
    for i, (y, z) in enumerate(screw_points):
        boss = cyl_obj(f"body_screw_socket_{i}", (47, y, z), 5.8, 14, axis="X", mat=mat)
        bool_diff(boss, cyl_obj(f"body_screw_pilot_{i}", (47, y, z), 1.8, 18, axis="X"))
        parts.append(boss)
        parts.append(cube_obj(f"body_screw_side_rib_{i}", (47, y * 0.98, z), (14, 8, 4), mat))
        if z < 56:
            parts.append(cube_obj(f"body_screw_bottom_rib_{i}", (47, y, 9), (14, 8, 14), mat))
        else:
            parts.append(cube_obj(f"body_screw_top_rib_{i}", (47, y, 103), (14, 8, 14), mat))

    joined = join_as("smart_cube_shell_v9_practical_body", parts, mat)
    add_bevel(joined, 0.7, 2)
    return joined


def make_front_bezel(mat):
    bezel = cube_obj("v9_front_bezel_outer", (-58, 0, 56), (5, 120, 92), mat)
    bool_diff(bezel, cube_obj("display_visible_window", (-58, 0, 56), (8, 76, 56)))
    # Big rear relief so the screen PCB and one-sided Dupont header can sit behind the bezel.
    bool_diff(bezel, cube_obj("bezel_rear_module_relief", (-55, 0, 56), (4, 102, 76)))
    parts = [bezel]
    for y, z in [(-51, 22), (51, 22), (-51, 90), (51, 90)]:
        boss = cyl_obj("front_bezel_screw_boss", (-55, y, z), 4.5, 5, axis="X", mat=mat)
        bool_diff(boss, cyl_obj("front_bezel_screw_hole", (-55, y, z), 1.8, 8, axis="X"))
        parts.append(boss)
    joined = join_as("smart_cube_shell_v9_front_screen_clamp_bezel", parts, mat)
    add_bevel(joined, 0.5, 2)
    return joined


def make_back_cover(mat):
    cover = cube_obj("v9_back_cover_plate", (55, 0, 56), (6, 126, 112), mat)
    # Rear ports deliberately oversized: exact connector offset varies between modules.
    bool_diff(cover, cube_obj("typec_port_window", (55, -34, 22), (10, 22, 12)))
    bool_diff(cover, cube_obj("dc_port_square_relief", (55, 32, 22), (10, 18, 18)))
    bool_diff(cover, cyl_obj("dc_barrel_round_hole", (55, 32, 22), 5.0, 12, axis="X"))

    parts = [cover]
    # Anti-push pads align with the two rear-facing module trays.
    parts += [
        cube_obj("esp32_rear_compression_pad", (50, -34, 25), (8, 38, 16), mat),
        cube_obj("dc_rear_compression_pad", (50, 32, 26), (8, 38, 18), mat),
        cube_obj("typec_port_reinforcement_bridge", (51, -34, 10), (8, 32, 8), mat),
        cube_obj("dc_port_reinforcement_bridge", (51, 32, 10), (8, 32, 8), mat),
    ]

    for i, (y, z) in enumerate([(-54, 16), (54, 16), (-54, 96), (54, 96)]):
        pad = cube_obj(f"cover_screw_pad_{i}", (51, y, z), (8, 16, 16), mat)
        bool_diff(pad, cyl_obj(f"cover_screw_clearance_{i}", (51, y, z), 2.2, 12, axis="X"))
        parts.append(pad)
        parts.append(cube_obj(f"cover_screw_rib_to_edge_{i}", (51, y, z), (8, 6, 28 if z < 56 else 24), mat))

    joined = join_as("smart_cube_shell_v9_back_cover_reinforced_ports", parts, mat)
    add_bevel(joined, 0.55, 2)
    return joined


def main():
    clear_scene()
    body_mat = material("v9 body blue gray", (0.12, 0.28, 0.42, 1))
    bezel_mat = material("v9 front black", (0.03, 0.03, 0.035, 1))
    cover_mat = material("v9 rear warm gray", (0.55, 0.55, 0.50, 1))

    body = make_body(body_mat)
    bezel = make_front_bezel(bezel_mat)
    cover = make_back_cover(cover_mat)

    export_obj(body, "smart_cube_shell_v9_practical_body.stl")
    export_obj(bezel, "smart_cube_shell_v9_front_screen_clamp_bezel.stl")
    export_obj(cover, "smart_cube_shell_v9_back_cover_reinforced_ports.stl")

    # Preview layout: spread parts so the user can inspect all faces in Blender.
    body.location.x = 0
    bezel.location.x -= 72
    cover.location.x += 72
    bpy.ops.object.light_add(type="AREA", location=(0, -150, 170))
    bpy.context.object.data.energy = 650
    bpy.context.object.data.size = 100
    bpy.ops.object.camera_add(location=(165, -220, 145), rotation=(1.05, 0, 0.68))
    bpy.context.scene.camera = bpy.context.object
    bpy.ops.wm.save_as_mainfile(filepath=str(BASE / "smart_cube_shell_v9_practical_preview.blend"))


if __name__ == "__main__":
    main()
