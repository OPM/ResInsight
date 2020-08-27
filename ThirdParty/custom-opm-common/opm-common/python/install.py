import sys
import os
import shutil
import compileall
import filecmp

src_root = sys.argv[1]
target_prefix = sys.argv[2]
install = int(sys.argv[3])
target_destdir = os.environ.get("DESTDIR", "")
if target_destdir != "":
    target_prefix = target_destdir + target_prefix

if not os.path.isdir(src_root):
    sys.exit("No such directory: {}".format(src_root))



path_offset = len(os.path.dirname(src_root))
for path,_ ,fnames in os.walk(src_root):
    target_path = os.path.join(target_prefix, path[path_offset+1:])
    if not os.path.isdir(target_path):
        os.makedirs(target_path)

    for f in fnames:
        _, ext = os.path.splitext(f)
        if ext == ".pyc":
            continue

        if ext == ".in":
            continue

        src_file = os.path.join(path, f)
        target_file = os.path.join(target_path, f)
        if not os.path.isfile(target_file):
            shutil.copy(src_file, target_file)
        elif not filecmp.cmp(src_file, target_file):
            shutil.copy(src_file, target_file)


        if install:
            print("-- Installing: {}".format(target_file))

if install:
    target_root = os.path.join(target_prefix, os.path.basename(src_root))
    compileall.compile_dir(target_root)
