import sys
import os

project_root = os.getcwd()
build_dir = os.path.join(project_root, "build", "Release")

if not os.path.exists(build_dir):
    print(f"Error: Path not found: {build_dir}")
    print("Make sure you are running this script from the project root!")
    sys.exit(1)

sys.path.append(build_dir)
import sh_engine

OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
if OUTPUT_DIR not in sys.path:
    sys.path.insert(0, OUTPUT_DIR)

import scene_utils

if __name__ == "__main__":

    OBJ_FILENAME = "cornell_box.obj"

    scene_utils.clear_scene()
    scene_utils.create_cornell_box()
    scene_utils.setup_camera()

    obj_path = os.path.join(OUTPUT_DIR, OBJ_FILENAME)
    scene_utils.export_obj(obj_path)
    # obj_path = f"\"C:\\Users\\vivek\\CLionProjects\\raytracer2\\assets\\robin-green-sh-scene2.obj\""
    print(f"OBJ file: {obj_path}")

    camera_eye = [0.0, 1.0, 2.0]
    camera_target = [0.0, 0.0, 0.0]
    field_of_view = 40.0
    if not os.path.exists(obj_path):
        print(f"⚠️ Warning: OBJ file not found at {obj_path}. C++ might crash or throw an error.")

    sh_engine.render(
        obj_path=obj_path,
        eye=camera_eye,
        target=camera_target,
        fov=field_of_view,
        width=1280,
        height=720
    )
    # print(f"MTL file: {obj_path.replace('.obj','.mtl')}")
