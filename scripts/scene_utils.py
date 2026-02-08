import bpy
import bmesh
import math
import os


ROOM_SIZE = 2.0

def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    for block in bpy.data.meshes:
        if block.users == 0:
            bpy.data.meshes.remove(block)
    for block in bpy.data.materials:
        if block.users == 0:
            bpy.data.materials.remove(block)

def create_material(name, color, emission=0.0):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True

    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = (*color, 1.0)
        bsdf.inputs["Roughness"].default_value = 1.0
        bsdf.inputs["Metallic"].default_value = 0.0
        if emission > 0:
            bsdf.inputs["Emission Color"].default_value = (*color, 1.0)
            bsdf.inputs["Emission Strength"].default_value = emission

    mat.diffuse_color = (*color, 1.0)
    return mat

def subdivide_object(obj, cuts=10):
    bpy.context.view_layer.objects.active = obj

    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.subdivide(number_cuts=cuts)
    bpy.ops.object.mode_set(mode='OBJECT')

    bpy.ops.object.modifier_add(type='TRIANGULATE')
    bpy.ops.object.modifier_apply(modifier="Triangulate")

def create_quad(name, vertices, material, flip_normal=False):
    mesh = bpy.data.meshes.new(name + "_mesh")
    obj = bpy.data.objects.new(name, mesh)

    bpy.context.collection.objects.link(obj)
    bm = bmesh.new()

    verts = [bm.verts.new(v) for v in vertices]
    bm.verts.ensure_lookup_table()
    if flip_normal:
        verts = verts[::-1]
    bm.faces.new(verts)

    bm.to_mesh(mesh)
    bm.free()

    obj.data.materials.append(material)
    subdivide_object(obj, cuts=20)

    return obj

def create_box(name, center, size, material, rotation_z=0):
    bpy.ops.mesh.primitive_cube_add(size=1, location=center)
    obj = bpy.context.active_object
    obj.name = name

    obj.scale = (size[0], size[1], size[2])

    obj.rotation_euler = (0, 0, rotation_z)

    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)

    obj.data.materials.clear()
    obj.data.materials.append(material)
    subdivide_object(obj, cuts=20)

    return obj

def create_cornell_box():
    half = ROOM_SIZE / 2

    # mats
    mat_white = create_material("white", (0.6, 0.6, 0.6))
    mat_red = create_material("red", (0.85, 0.05, 0.05))
    mat_green = create_material("green", (0.12, 0.85, 0.15))
    mat_light = create_material("light", (1.0, 1.0, 1.0)) #, emission=15.0

    # Floor: Z = -half (normal points up +Z)
    create_quad(
        "floor",
        [(-half, -half, -half), (half, -half, -half), (half, half, -half), (-half, half, -half)],
        mat_white,
        flip_normal=False # Standard CCW winding for Up normal
    )

    # Ceiling: Z = +half (Normal points Down -Z)
    create_quad(
        "ceiling",
        [(-half, -half, half), (-half, half, half), (half, half, half), (half, -half, half)],
        mat_white,
        flip_normal=False
    )

    # Back Wall: Y = +half (Normal points -Y towards origin)
    # Note: In Blender, +Y is "Back", -Y is "Front".
    # We want the wall at the "back" of the room relative to a front view.
    create_quad(
        "back_wall",
        [(-half, half, -half), (half, half, -half), (half, half, half), (-half, half, half)],
        mat_white,
        flip_normal=False
    )

    # Left Wall (Red): X = -half (Normal points Right +X)
    create_quad(
        "left_wall",
        [(-half, -half, -half), (-half, half, -half), (-half, half, half), (-half, -half, half)],
        mat_red,
        flip_normal=False
    )

    # Right Wall (Green): X = +half (Normal points Left -X)
    create_quad(
        "right_wall",
        [(half, -half, -half), (half, -half, half), (half, half, half), (half, half, -half)],
        mat_green,
        flip_normal=False
    )

    # Light: Small patch on Ceiling
    light_size = 0.3
    light_z = half - 0.001
    create_quad(
        "light",
        [(-light_size, -light_size, light_z), (light_size, -light_size, light_z),
         (light_size, light_size, light_z), (-light_size, light_size, light_z)],
        mat_light,
        flip_normal=True # Flip to point down
    )

    # Tall Box
    # Swapped Y and Z coordinates for Blender
    tall_box_height = 0.6
    tall_box = create_box(
        "tall_box",
        center=(0.35, 0.35, -half + tall_box_height/2),
        size=(0.3, 0.3, tall_box_height), # Width, Depth, Height
        material=mat_white,
        rotation_z=math.radians(-18)
    )

    # Short Box
    short_box_height = 0.3
    short_box = create_box(
        "short_box",
        center=(-0.35, -0.3, -half + short_box_height/2),
        size=(0.3, 0.3, short_box_height),
        material=mat_white,
        rotation_z=math.radians(18)
    )
def setup_camera():
    bpy.ops.object.camera_add(location=(0, -ROOM_SIZE * 2, 0), rotation=(math.radians(90), 0, 0))
    camera = bpy.context.active_object
    camera.data.lens = 35
    bpy.context.scene.camera = camera

def export_obj(filepath):
    bpy.ops.wm.obj_export(
        filepath=filepath,
        export_selected_objects=False,
        export_uv=True,
        export_normals=True,
        export_colors=False,
        export_materials=True,
        export_triangulated_mesh=True,
        forward_axis='NEGATIVE_Z',
        up_axis='Y'
    )

















