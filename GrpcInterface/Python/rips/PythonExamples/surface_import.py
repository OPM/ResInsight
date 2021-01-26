# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()
print("ResInsight version: " + resinsight.version_string())

# Example code

# get the project
project = resinsight.project

# get the topmost surface folder from the project
surfacefolder = project.surface_folder()

# list of surface files to load
filenames = ["surface1.ts", "surface2.ts", "surface3.ts"]

# Load the files into the top level
for surffile in filenames:
    surface = surfacefolder.import_surface(surffile)
    if surface is None:
        print("Could not import the surface " + surffile)

# add a subfolder
subfolder = surfacefolder.add_folder("ExampleFolder")

# load the same surface multiple times using increasing depth offsets
# store them in the new subfolder we just created
for offset in range(0, 200, 20):
    surface = subfolder.import_surface("mysurface.ts")
    if surface:
        surface.depth_offset = offset
        surface.update()
    else:
        print("Could not import surface.")

# get an existing subfolder
existingfolder = project.surface_folder("ExistingFolder")
if existingfolder is None:
    print("Could not find the specified folder.")
