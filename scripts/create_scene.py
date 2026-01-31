import sys

import os


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
    print(f"OBJ file: {obj_path}")
    # print(f"MTL file: {obj_path.replace('.obj','.mtl')}")
