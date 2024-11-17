import os

from nevf import nevf
from ngin.project import project

def execute():

    bnev = nevf.Nevf(os.getcwd() + "/.ngin")

    if bnev.has('name'):
        project.NginProject(config=bnev)
