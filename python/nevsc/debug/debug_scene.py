import bpy
import math


def print_material_node_info(obj):
  """
  Prints detailed information about each material's node tree, including 
  input and output types, connections, and the specific connected output socket.

  Args:
    obj: The object to extract materials from.
  """
  if not obj.type == 'MESH':  # Only process mesh objects
    return
  
  for material in obj.data.materials:
    if material is not None:
      print(f"Material: {material.name}")
      for node in material.node_tree.nodes:
        print(f"  Node Name: {node.name}")
        print(f"  Node Type: {node.type}")

        # Print input information
        for input in node.inputs:
          print(f"    Input: {input.name} ({input.type})")
          for link in input.links:
            print(f"      Connected to: {link.from_node.name} - {link.from_socket.name} ({link.from_socket.type})")

        # Print output information
        for output in node.outputs:
          print(f"    Output: {output.name} ({output.type})")


def print_mesh_data(obj):
  """
  Prints mesh data including vertices, triangles, normals, and UVs.

  Args:
    obj: The object to extract mesh data from.
  """
  if obj.type == 'MESH':
    mesh = obj.data

    print(f"Mesh: {mesh.name}")

    # Vertices
    print("Vertices:")
    for vertex in mesh.vertices:
      print(f"  {vertex.co}")  # vertex.co gives you the vertex coordinates

    # Triangles (Faces)
    print("Triangles:")
    for face in mesh.polygons:
      print(f"  Vertices: {face.vertices}")  # Indices of vertices forming the face
      print(f"    Normal: {face.normal}")  # Face normal

    # Normals (per vertex)
    # Note: You need to have custom normals enabled for this to work
    if mesh.has_custom_normals:
      print("Normals:")
      mesh.calc_tangents()  # Calculate tangents to get split normals
      for loop in mesh.loops:
        print(f"  Vertex {loop.vertex_index}: {loop.normal}") 

    # UVs
    if mesh.uv_layers:
      print("UVs:")
      uv_layer = mesh.uv_layers.active.data  # Get the active UV layer
      for loop in mesh.loops:
        uv = uv_layer[loop.index].uv 
        print(f"  Vertex {loop.vertex_index}: {uv}")

    if mesh.vertex_colors:
      print("Vertex Colors:")
      color_layer = mesh.vertex_colors.active.data  # Get the active color layer
      for loop in mesh.loops:
        color = color_layer[loop.index].color
        print(f"  Vertex {loop.vertex_index}: {color}")

    # Vertex Weights
    print("Vertex Weights:")
    for vertex in mesh.vertices:
      for group in vertex.groups:
        group_name = obj.vertex_groups[group.group].name
        weight = group.weight
        print(f"  Vertex {vertex.index}: Group '{group_name}', Weight: {weight}") 


def print_transform_data(obj):
  """
  Prints transform data (location, rotation, scale) and parent/child information.

  Args:
    obj: The object to extract transform data from.
  """
  print(f"Object: {obj.name}")

  # Location
  print(f"  Location: {obj.location}")

  # Rotation (as Euler angles in degrees)
  rotation_euler_degrees = [math.degrees(a) for a in obj.rotation_euler] # Convert to degrees
  print(f"  Rotation: {rotation_euler_degrees}")  

  # Scale
  print(f"  Scale: {obj.scale}")

  # Parent
  if obj.parent:
    print(f"  Parent: {obj.parent.name}")

  # Children
  if obj.children:
    print("  Children:")
    for child in obj.children:
      print(f"    {child.name}")


def print_armature_data(obj):
  """
  Prints armature data including bone names, their base pose transforms,
  and parent-child relationships.

  Args:
    obj: The object to extract armature data from.
  """
  if obj.type == 'ARMATURE':
    armature = obj.data
    print(f"Armature: {armature.name}")

    for bone in armature.bones:
      print(f"  Bone: {bone.name}")

      # Base (rest) position
      print(f"    Base Position: {bone.head_local}")

      # You can also get the tail position:
      # print(f"    Tail Position: {bone.tail_local}")

      # Base (rest) rotation (as quaternion)
      # print(f"    Base Rotation: {bone.matrix_local.to_quaternion()}")

      # To get rotation as Euler angles in degrees:
      rotation_euler_degrees = [math.degrees(a) for a in bone.matrix_local.to_euler()]
      print(f"    Base Rotation (degrees): {rotation_euler_degrees}")

      # Base (rest) scale - this is usually (1, 1, 1)
      print(f"    Base Scale: {bone.matrix_local.to_scale()}")

      # Parent bone
      if bone.parent:
        print(f"    Parent: {bone.parent.name}")
      else:
        print("    Parent: None (root bone)")

      # Children bones
      if bone.children:
        print("    Children:")
        for child in bone.children:
          print(f"      {child.name}")


def print_bone_animation(obj):
  if obj.type == 'ARMATURE':
    
    actions = [action.name for action in bpy.data.actions]
    
    action = obj.animation_data.action
    if action:
      print(f"Action: {action.name}")
      for fcurve in action.fcurves:
        if fcurve.data_path.startswith("pose.bones"):
          bone_name = fcurve.data_path.split('"')[1]
          print(f"  Bone: {bone_name}")

          # Determine transform type and axis
          transform_type = fcurve.data_path.split(".")[-1]  # Get 'location', 'rotation_quaternion', or 'scale'
          array_index = fcurve.array_index

          if transform_type == "location":
            type_str = "Position"
            axis_str = "x" if array_index == 0 else ("y" if array_index == 1 else "z")
          elif transform_type == "rotation_quaternion":
            type_str = "Rotation"
            axis_str = "w" if array_index == 0 else ("x" if array_index == 1 else ("y" if array_index == 2 else "z"))
          elif transform_type == "scale":
            type_str = "Scale"
            axis_str = "x" if array_index == 0 else ("y" if array_index == 1 else "z")
          else:
            type_str = "Unknown"
            axis_str = "Unknown"

          print(f"    Type: {type_str}, Axis: {axis_str}") 

          for keyframe in fcurve.keyframe_points:
            print(f"      Frame: {keyframe.co[0]}, Value: {keyframe.co[1]}")
    
      print(f"Actions: {actions}")


def process(file_path):
  try:
    for obj in bpy.context.scene.objects:

      if not obj.type == 'MESH':
        continue

      print(f"Object: {obj.name}")
      
      #print_material_node_info(obj)

      print_mesh_data(obj)

      #print_transform_data(obj)

     # print_armature_data(obj)
      #print_bone_animation(obj)

      break

  except Exception as e:
    print(f"Error: Could not open or process the file. {e}")