import bpy

from nevsc.blend import blend_settings
from nevsc.blend import blend_processor

import os

class NevBlend:
    def __init__(self, filepath):
        """
        Opens a Blender file.

        Args:
            filepath (str): The path to the .blend file.
        """
        self.filepath = filepath
        bpy.ops.wm.open_mainfile(filepath=filepath)

        blend_settings.set_defaults()

        self.from_blender()

    def from_blender(self):

        dict = blend_processor.process(os.path.dirname(self.filepath))

