import bpy
from pathlib import Path

model_path = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design\smart_cube_modular_insert_v2_compact.stl")

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete()

if hasattr(bpy.ops.wm, "stl_import"):
    bpy.ops.wm.stl_import(filepath=str(model_path))
else:
    bpy.ops.import_mesh.stl(filepath=str(model_path))

for obj in bpy.context.scene.objects:
    obj.select_set(True)

bpy.ops.view3d.view_axis(type="FRONT", align_active=False)

blend_path = model_path.with_suffix(".blend")
bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
