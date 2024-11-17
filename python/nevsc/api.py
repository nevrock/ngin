import os

from nevsc.blend import blend

from nevf import nevf

def execute():

    bnev = nevf.Nevf(os.getcwd() + "/.nevsc")

    if bnev.has('file'):
        filepath = os.path.join(os.getcwd(), bnev.get('file'))
        print(filepath)
        #debug_scene.process(filepath)
        blend.NevBlend(filepath=filepath)

    # os.remove(os.getcwd() + "/.nevsc")