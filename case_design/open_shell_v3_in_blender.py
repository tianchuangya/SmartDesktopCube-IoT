import bpy
from pathlib import Path

base = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")
files = [
    base / "smart_cube_shell_v3_body_adjustable.stl",
    base / "smart_cube_shell_v3_back_cover.stl",
    base / "smart_cube_fit_gauge_v3.stl",
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

    if index == 0:
        obj.color = (0.2, 0.55, 1.0, 1.0)
    elif index == 1:
        obj.color = (0.2, 0.8, 0.45, 1.0)
    else:
        obj.color = (1.0, 0.72, 0.2, 1.0)

for obj in bpy.context.scene.objects:
    obj.select_set(True)

bpy.ops.wm.save_as_mainfile(filepath=str(base / "smart_cube_shell_v3_preview.blend"))
