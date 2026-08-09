import bpy
from pathlib import Path
base = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")
files = [
    base / "smart_cube_shell_v7_body_front_clamp_reinforced_ports.stl",
    base / "smart_cube_shell_v7_front_screen_clamp_bezel.stl",
    base / "smart_cube_shell_v7_back_cover_reinforced_open_ports.stl",
]
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete()
for index, path in enumerate(files):
    if hasattr(bpy.ops.wm, "stl_import"):
        bpy.ops.wm.stl_import(filepath=str(path))
    else:
        bpy.ops.import_mesh.stl(filepath=str(path))
    obj = bpy.context.object
    obj.name = path.stem
    obj.location.x += index * 125
    obj.color = [(0.2,0.55,1,1),(1,0.65,0.15,1),(0.2,0.8,0.45,1)][index]
bpy.ops.wm.save_as_mainfile(filepath=str(base / "smart_cube_shell_v7_preview.blend"))
