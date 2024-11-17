import bpy
import math
import numpy as np
from nevf import nevf
import os

def process_material_node_info(obj, data_dict):
    """
    Adds material node information to the data dictionary using only dictionaries.
    """
    if not obj.type == 'MESH':
        return

    materials = {}
    for material_index, material in enumerate(obj.data.materials):
        if material is not None:
            nodes = {}
            for node_index, node in enumerate(material.node_tree.nodes):
                inputs = {}
                for input_index, input in enumerate(node.inputs):
                    connections = {}
                    for connection_index, link in enumerate(input.links):
                        connections[str(connection_index)] = {
                            "from_node": link.from_node.name,
                            "from_socket": link.from_socket.name,
                            "from_socket_type": link.from_socket.type
                        }
                    inputs[str(input_index)] = {
                        "name": input.name,
                        "type": input.type,
                        "connections": connections
                    }

                outputs = {}
                for output_index, output in enumerate(node.outputs):
                    outputs[str(output_index)] = {
                        "name": output.name,
                        "type": output.type
                    }

                nodes[str(node_index)] = {
                    "name": node.name,
                    "type": node.type,
                    "inputs": inputs,
                    "outputs": outputs
                }

            materials[str(material_index)] = {
                "name": material.name,
                "nodes": nodes
            }

    data_dict["materials"] = materials


def process_mesh_data(obj, data_dict):
    """
    Adds mesh data to the data dictionary.
    """
    if obj.type == 'MESH':
        mesh = obj.data
        data_dict["mesh"] = {
            "name": mesh.name,
            "vertices": [np.array(vertex.co).tolist() for vertex in mesh.vertices],
            "triangles": {
                str(idx): {
                    "vertices": list(face.vertices),
                    "normal": np.array(face.normal).tolist()
                }
                for idx, face in enumerate(mesh.polygons)
            }
        }
    
        if mesh.has_custom_normals:
            mesh.calc_tangents()
            data_dict["mesh"]["normals"] = [np.array(loop.normal).tolist() for loop in mesh.loops]

        if mesh.uv_layers:
            uv_layer = mesh.uv_layers.active.data
            data_dict["mesh"]["uvs"] = [np.array(uv_layer[loop.index].uv).tolist() for loop in mesh.loops]

        if mesh.vertex_colors:
            color_layer = mesh.vertex_colors.active.data
            data_dict["mesh"]["vertex_colors"] = [np.array(color_layer[loop.index].color).tolist() for loop in mesh.loops]

        vertex_weights = {}  # Initialize as a dictionary
        for vertex in mesh.vertices:
            vertex_weights[str(vertex.index)] = {}  # Create a nested dictionary for each vertex
            for group in vertex.groups:
                group_name = obj.vertex_groups[group.group].name
                weight = group.weight
                vertex_weights[str(vertex.index)][group_name] = weight  # Store weight with group name as key

        data_dict["mesh"]["vertex_weights"] = vertex_weights # Assign the list to the dictionary


def process_transform_data(obj, data_dict):
    """
    Adds transform data to the data dictionary.
    """
    data_dict["transform"] = {
        "location": np.array(obj.location).tolist(),
        "rotation": [math.degrees(a) for a in obj.rotation_euler],
        "scale": np.array(obj.scale).tolist()
    }
    if obj.parent:
        data_dict["transform"]["parent"] = obj.parent.name
    if obj.children:
        data_dict["transform"]["children"] = [child.name for child in obj.children]


def process_armature_data(obj, data_dict):
    """
    Adds armature data to the data dictionary.
    """
    if obj.type == 'ARMATURE':
        armature = obj.data
        bones = {}  # Initialize as a dictionary

        for i, bone in enumerate(armature.bones):  # Use enumerate to get index
            bone_data = {
                "name": bone.name,
                "base_position": np.array(bone.head_local).tolist(),
                "base_rotation": [math.degrees(a) for a in bone.matrix_local.to_euler()],
                "base_scale": np.array(bone.matrix_local.to_scale()).tolist()
            }
            if bone.parent:
                bone_data["parent"] = bone.parent.name
            if bone.children:
                bone_data["children"] = [child.name for child in bone.children]
            bones[bone.name] = bone_data  # Use index as key

        data_dict["armature"] = {
            "name": armature.name,
            "bones": bones  # Assign the dictionary to the dictionary
        }


def process_bone_animation(obj, data_dict):
    """
    Adds bone animation data to the data dictionary.
    """
    if obj.type == 'ARMATURE':
        animations = {}  # Initialize as a dictionary
        actions = bpy.data.actions

        for action in actions:
            action_data = {
                "name": action.name,
                "fcurves": {}  # Fcurves will also be a dictionary
            }
            for i, fcurve in enumerate(action.fcurves):  # Use enumerate for fcurves
                if fcurve.data_path.startswith("pose.bones"):
                    bone_name = fcurve.data_path.split('"')[1]
                    transform_type = fcurve.data_path.split(".")[-1]
                    array_index = fcurve.array_index

                    if transform_type == "location":
                        type_str = "position"
                        axis_str = "x" if array_index == 0 else ("y" if array_index == 1 else "z")
                    elif transform_type == "rotation_quaternion":
                        type_str = "rotation"
                        axis_str = "w" if array_index == 0 else ("x" if array_index == 1 else ("y" if array_index == 2 else "z"))
                    elif transform_type == "scale":
                        type_str = "scale"
                        axis_str = "x" if array_index == 0 else ("y" if array_index == 1 else "z")
                    else:
                        type_str = "Unknown"
                        axis_str = "Unknown"

                    fcurve_data = {
                        "bone": bone_name,
                        "type": type_str,
                        "axis": axis_str,
                        "keyframes": {
                            str(keyframe.co[0]): keyframe.co[1]  # Use frame as key and value as value
                            for keyframe in fcurve.keyframe_points
                        }
                    }
                    action_data["fcurves"][str(i)] = fcurve_data  # Use index as key for fcurves

            animations[action.name] = action_data  # Use action name as key

        data_dict["animations"] = animations  # Assign the dictionary to the dictionary


def process_nevf_data(obj, data_dict):
    filepath = obj.nevf_data
    if filepath:
        data_dict['nevf_data'] = filepath


def process(dir_out):
    try:
        # Create the "nevsc" subdirectory if it doesn't exist
        nevsc_dir = os.path.join(dir_out, "nevsc")
        os.makedirs(nevsc_dir, exist_ok=True)

        for obj in bpy.context.scene.objects:
            data_dict = nevf.Nevf()

            filename = f"{obj.name}.nevf"
            # Save the file in the "nevsc" subdirectory
            filepath_out = os.path.join(nevsc_dir, filename)

            process_material_node_info(obj, data_dict)
            process_mesh_data(obj, data_dict)
            process_transform_data(obj, data_dict)
            process_armature_data(obj, data_dict)
            process_bone_animation(obj, data_dict)
            process_nevf_data(obj, data_dict)

            data_dict.write(filepath_out)

    except Exception as e:
        print(f"Error: Could not open or process the file. {e}")