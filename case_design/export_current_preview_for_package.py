import bpy
from pathlib import Path

base = Path(r"C:\Users\ASUS\Desktop\SmartDesktopCube-IoT\SmartDesktopCube-IoT\case_design")
out = base / "SmartDesktopCube_case_v20_package" / "preview"

bpy.ops.object.select_all(action="SELECT")
bpy.ops.wm.stl_export(
    filepath=str(out / "SmartDesktopCube_case_v20_all_parts_preview.stl"),
    export_selected_objects=True,
)
bpy.ops.export_scene.gltf(
    filepath=str(out / "SmartDesktopCube_case_v20_all_parts_preview.glb"),
    export_format="GLB",
)
