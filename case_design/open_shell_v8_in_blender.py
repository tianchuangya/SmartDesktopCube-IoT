import bpy
from pathlib import Path

base = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")
parts = [
    ("smart_cube_shell_v8_web_dims_body.stl", (0, 0, 0), "Body"),
    ("smart_cube_shell_v8_web_dims_front_bezel.stl", (-58, 0, 0), "Front screen clamp bezel"),
    ("smart_cube_shell_v8_web_dims_back_cover.stl", (58, 0, 0), "Back cover"),
]

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete()

for filename, location, label in parts:
    bpy.ops.wm.stl_import(filepath=str(base / filename))
    obj = bpy.context.object
    obj.name = label
    obj.location = location
    mat = bpy.data.materials.new(label + " material")
    mat.diffuse_color = {
        "Body": (0.16, 0.34, 0.55, 1.0),
        "Front screen clamp bezel": (0.05, 0.05, 0.05, 1.0),
        "Back cover": (0.55, 0.55, 0.50, 1.0),
    }[label]
    obj.data.materials.append(mat)

bpy.ops.object.light_add(type="AREA", location=(0, -120, 140))
bpy.context.object.data.energy = 500
bpy.context.object.data.size = 80

bpy.ops.object.camera_add(location=(135, -185, 125), rotation=(1.1, 0, 0.65))
bpy.context.scene.camera = bpy.context.object

bpy.ops.wm.save_as_mainfile(filepath=str(base / "smart_cube_shell_v8_preview.blend"))
