import bpy
from pathlib import Path
base = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")
files = [
    base / "smart_cube_shell_v4_body_typec_dc.stl",
    base / "smart_cube_shell_v4_back_cover_open_ports.stl",
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
    obj.location.x += index * 120
    obj.color = (0.2, 0.55, 1.0, 1.0) if index == 0 else (0.2, 0.8, 0.45, 1.0)
bpy.ops.wm.save_as_mainfile(filepath=str(base / "smart_cube_shell_v4_open_ports_preview.blend"))
