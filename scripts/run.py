import sys
import os

# 1. Point Python to the build folder
#    We need to tell Python where to find 'sh_engine.pyd'
#    It is located in build/Release
project_root = os.getcwd()
build_dir = os.path.join(project_root, "build")

if not os.path.exists(build_dir):
    print(f"Error: Path not found: {build_dir}")
    print("Make sure you are running this script from the project root!")
    sys.exit(1)

sys.path.append(build_dir)
try:
    import sh_engine
    print(f"Loaded C++ Renderer from: {sh_engine.__file__}")
except ImportError as e:
    print("Failed to import sh_engine.")
    print(f"   Python Version: {sys.version.split()[0]}")
    print(f"   Looking in: {build_dir}")
    print(f"   Error details: {e}")
    print("\nTIP: Your .pyd is for Python 3.11 (cp311). Make sure you are running this script with Python 3.11.")
    sys.exit(1)

obj_file = "assets/cornell_embree_tri.obj" # CHANGE THIS to a real path on your disk!

scenes = {
    "cbe_tri_divs" : {"path" : "assets/cornell_embree_tri_divs.obj", "cam" : {"center" : [278, 274, 280]} },
    "cbe_small" : {"path" : "assets/cornell_embree_small_divs.obj", "cam" : {"center" : [278, 274, 280]} },
    "cbe_tri" : {"path" : "assets/cornell_embree_tri.obj", "cam" : {"center" : [278, 274, 280]} },
    "bmw" : {"path" : "assets/bmw_tri.obj"},
    "breakfast" : {"path" : "assets/breakfast_room.obj"},
    "conference" : {"path" : "assets/conference.obj"},
}

camera_target = scenes["cbe_tri"]["cam"]["center"]
camera_eye = [camera_target[0], camera_target[1] , -175]
field_of_view = 40.0

print("Starting Renderer...")
try:
    # sh_engine.render(
    #     obj_path=scenes["cbe_small"]["path"],
    #     eye=camera_eye,
    #     target=camera_target,
    #     fov=field_of_view,
    #     li=1.0,
    #     numBounces=30,
    #     width=840,
    #     height=620
    # )

    sh_engine.orbit_render(obj_path="assets/CornellBox/CornellBox-Empty-CO.obj", bounces=20, width=3840, height=2160)
except Exception as e:
    print(f"C++ Error: {e}")