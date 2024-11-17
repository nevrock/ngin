bl_info = {
    "name": "Nevf Data Panel",
    "author": "Bard",
    "version": (1, 0),
    "blender": (3, 0, 0),
    "location": "View3D > Sidebar > String Panel",
    "description": "Displays a panel with a file browser for selecting a filepath",
    "warning": "",
    "doc_url": "",
    "category": "Development",
}

import bpy

# Define the property on the object level, not the scene
def create_object_filepath_property():
    if not hasattr(bpy.types.Object, "nevf_data"):
        bpy.types.Object.nevf_data = bpy.props.StringProperty(
            name="Nevf Data",
            description="Filepath to a nevf file",
            default="",
            update=None  # Optional: Define an update function if needed
        )

class NevfDataPanel(bpy.types.Panel):
    """Creates a Panel in the Object properties window"""
    bl_label = "Nevf Data"
    bl_idname = "OBJECT_PT_string_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Nevf"

    def draw(self, context):
        layout = self.layout
        obj = context.object  # Get the currently selected object

        if obj:
            # Display the file path property for the active object
            layout.prop(obj, "nevf_data")

            # Check if the file path is valid
            if obj.nevf_data and not obj.nevf_data.lower().endswith(".nevf"):
                layout.label(text="Invalid file type! Please select a .nevf file.", icon='ERROR')

            # Add the file browser button to select a file
            layout.operator("object.file_browser", text="Browse File")

# Operator to open the file browser and set the selected path to the property
class OBJECT_OT_file_browser(bpy.types.Operator):
    bl_idname = "object.file_browser"
    bl_label = "Select File"
    bl_description = "Select a file and set its path to the custom property"

    # Define the file path property that stores the selected path
    filepath: bpy.props.StringProperty(subtype='FILE_PATH')

    def execute(self, context):
        # Validate the file extension
        if not self.filepath.lower().endswith(".nevf"):
            self.report({'ERROR'}, "Invalid file type! Please select a .nevf file.")
            return {'CANCELLED'}  # Cancel the operation if the file type is invalid
        
        # Set the custom property to the selected file path
        context.object.nevf_data = self.filepath
        return {'FINISHED'}

    def invoke(self, context, event):
        # Open the file dialog to pick a file
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

# Panel that will display the custom string property
class OBJECT_PT_custom_string_property(bpy.types.Panel):
    bl_label = "Nevf Data"
    bl_idname = "OBJECT_PT_custom_string_property"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        # Add the custom string property field
        layout.prop(obj, "nevf_data")
        
        # Add a button that will invoke the file browser operator
        layout.operator("object.file_browser", text="Browse File")


# Function to handle saving logic
def write_nevf_data(dummy):  # 'dummy' is required for handler functions
    for obj in bpy.data.objects:
        if hasattr(obj, "nevf_data") and obj.nevf_data:
            filepath = obj.nevf_data
            print(filepath)


def register():
    create_object_filepath_property()  # Add the custom property to the object type
    bpy.utils.register_class(NevfDataPanel)
    bpy.utils.register_class(OBJECT_OT_file_browser)
    bpy.utils.register_class(OBJECT_PT_custom_string_property)

    # Add the save_post handler
    bpy.app.handlers.save_post.append(write_nevf_data)

def unregister():
    bpy.utils.unregister_class(NevfDataPanel)
    del bpy.types.Object.nevf_data  # Clean up by removing the property
    bpy.utils.unregister_class(OBJECT_OT_file_browser)
    bpy.utils.unregister_class(OBJECT_PT_custom_string_property)

    # Remove the save_post handler
    bpy.app.handlers.save_post.remove(write_nevf_data)

if __name__ == "__main__":
    register()