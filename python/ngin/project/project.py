import os

class NginProject:

    def __init__(self, config):
        self._config = config
        self.name = self._config.get('name')

        print("--- New NGIN Project ---")

        self.set_default_folder_structure()

    def set_default_folder_structure(self):

        print("setting defaults...")

        self._base_dir = self._config.get('base_dir')
        self._blend_dir = os.path.join(self._base_dir, 'blend')
        self._assets_dir = os.path.join(self._base_dir, 'assets')
        self._scipts_dir = os.path.join(self._base_dir, 'scripts')
        self._nevf_dir = os.path.join(self._base_dir, 'nevf')

        os.makedirs(self._blend_dir, exist_ok=True)
        os.makedirs(self._assets_dir, exist_ok=True)
        os.makedirs(self._scipts_dir, exist_ok=True)
        os.makedirs(self._nevf_dir, exist_ok=True)

        print(f"base folder: {self._base_dir}")


